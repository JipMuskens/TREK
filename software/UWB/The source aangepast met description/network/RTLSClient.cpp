#include "RTLSClient.h"

#include "RTLSDisplayApplication.h"
#include "SerialConnection.h"
#include "trilateration.h"

#include <QTextStream>
#include <QDateTime>
#include <QThread>
#include <QFile>
#include <QDebug>
#include <math.h>
#include <QMessageBox>
#include "Sender.h"

#include <QDomDocument>
#include <QFile>

#define TOF_REPORT_LEN  (58)
#define TOF_REPORT_ARGS (11)

/**
* @brief RTLSDisplayApplication
*        Constructor, it consumes the data received over the COM port connection and sends the
*        processed data to the graphical display
* */
RTLSClient::RTLSClient(QObject *parent) :
    QObject(parent),
    _first(true),
    _file(NULL)

{
    _graphicsWidgetReady = false ;
    _tagList.clear();

    //memset(&_ancArray, 0, MAX_NUM_ANCS*sizeof(anc_struct_t));
    _serial = NULL;

    RTLSDisplayApplication::connectReady(this, "onReady()");
}

void RTLSClient::onReady()
{
    QObject::connect(RTLSDisplayApplication::serialConnection(), SIGNAL(serialOpened(QString, QString)),
                         this, SLOT(onConnected(QString, QString)));
}

void RTLSClient::onConnected(QString ver, QString conf)
{
    QDateTime now = QDateTime::currentDateTime();

    QString filename("./Logs/"+now.toString("yyyyMMdd_hhmmss")+"RTLS_log.txt");
    _file = new QFile(filename);

    if (!_file->open(QFile::ReadWrite | QFile::Text))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(_file->errorString())));
        QMessageBox::critical(NULL, tr("Logfile Error"), QString("Cannot create file %1 %2\nPlease make sure ./Logs/ folder exists.").arg(filename).arg(_file->errorString()));
    }
    else
    {
        QString config;
        bool ok = true;
        int configuration = conf.toLocal8Bit().at(0);
        int anchor = 0, chan = 2, mode = 0, addr = 0, addr1 = 0, m = 0;
                //tag or anchor, chan 2 or 5,
        if(ok)
        {
            m = (configuration & 6) >> 1;

            if(configuration & 0x8)
            {
                anchor = 1;
            }

            mode = (configuration & 0x2) >> 1;

            if(configuration & 0x4)
            {
                chan = 5 ;
            }

            addr1 = (configuration & 0x70) >> 4;

            addr = ((addr1 >> 2) & 0x1) + (((addr1 >> 1) & 0x1) << 1) + ((addr1 & 0x1) << 2);
        }

        //write the header
        if(anchor)
        {
            config = QString(":Conf:Anchor%1:%2:Chan%3\n").arg(addr).arg(mode).arg(chan);
            emit statusBarMessage(QString("DecaRangeRTLS Anchor ID%1 Mode %2").arg(addr).arg(m));
#if 0
            if(addr & 0x3) //anchors 1, 2, and 3 don't support RTLS (only geo fencing)
            {
                QMessageBox::about(NULL, tr("COM"), QString("Connected to Anchor %1.\nRTLS is not supported, only Geo-fencing mode.").arg(addr));
            }
            else if (addr == 0x4)
            {
                QMessageBox::about(NULL, tr("COM"), QString("Connected to Listener.\nBoth RTLS and Geo-fencing modes are supported."));
            }
            else
            {
                QMessageBox::about(NULL, tr("COM"), QString("Connected to Anchor %1.\nBoth RTLS and Geo-fencing modes are supported.").arg(addr));
            }
#endif
        }
        else
        {
            config = QString(":Conf:Tag%1:%2:Chan%3\n").arg(addr).arg(mode).arg(chan);
            emit statusBarMessage(QString("DecaRangeRTLS Tag ID%1 Mode %2").arg(addr).arg(m));
#if 0
            QMessageBox::about(NULL, tr("COM"), QString("Connected to Tag %1.").arg(addr));
#endif
        }

        QString nowstr = now.toString("T:hhmmsszzz:");
        QString s = nowstr + QString("DecaRangeRTLS:LogFile:") + ver + config;
        QTextStream ts( _file );
        ts << s;
    }

    //get pointer to Serial connection serial port pointer
    _serial = RTLSDisplayApplication::serialConnection()->serialPort();
    connect(_serial, SIGNAL(readyRead()), this, SLOT(newData()));
}

//initialise the tag reports strusture and add to the list
void RTLSClient::initialiseTagList(int id)
{
    tag_reports_t r;
    memset(&r, 0, sizeof(tag_reports_t));
    r.id = id;
    r.ready = false;
    r.rangeSeq = -1;
    memset(&r.rangeValue[0][0], -1, sizeof(r.rangeValue));
    _tagList.append(r);
}

void RTLSClient::updateTagStatistics(int i, double x, double y, double z)
//update the history array and the average
{
    QDateTime now = QDateTime::currentDateTime();
    QString nowstr = now.toString("T:hhmmsszzz:");
    int j = 0;
    int idx = _tagList.at(i).arr_idx;
    uint64_t id = _tagList.at(i).id;
    tag_reports_t rp = _tagList.at(i);
    double avDistanceXY = 0;
    double sum_std = 0;
    double DistanceXY[HIS_LENGTH];
    double DstCentreXY[HIS_LENGTH];
    double stdevXY = 0;

    rp.av_x = 0;
    rp.av_y = 0;
    rp.av_z = 0;


    for(j=0; j<HIS_LENGTH; j++)
    {
       rp.av_x += rp.x_arr[j];
       rp.av_y += rp.y_arr[j];
       rp.av_z += rp.z_arr[j];
    }

    rp.av_x /= HIS_LENGTH;
    rp.av_y /= HIS_LENGTH;
    rp.av_z /= HIS_LENGTH;

    for(j=0; j<HIS_LENGTH; j++)
    {
        DistanceXY[j] = sqrt((rp.x_arr[j] - rp.av_x)*(rp.x_arr[j] - rp.av_x) + (rp.y_arr[j] - rp.av_y)*(rp.y_arr[j] - rp.av_y));
    }

    for (j=0; j<HIS_LENGTH; j++)
    {
        avDistanceXY += DistanceXY[j]/HIS_LENGTH;
    }

    for(j=0; j<HIS_LENGTH; j++)
    {
        sum_std += (DistanceXY[j]-avDistanceXY)*(DistanceXY[j]-avDistanceXY);

    }

    stdevXY = sqrt(sum_std/HIS_LENGTH);

    vec2d sum_tempXY = {0, 0};
    vec2d CentrerXY = {0, 0};

    int counterXY = 0;

    for(j=0; j<HIS_LENGTH; j++)
    {
        if (DistanceXY[j] < stdevXY*2)
        {
            sum_tempXY.x += rp.x_arr[j];
            sum_tempXY.y += rp.y_arr[j];
            counterXY++;
        }

    }

    CentrerXY.x  = sum_tempXY.x/counterXY;
    CentrerXY.y  = sum_tempXY.y/counterXY;

    for(j=0; j<HIS_LENGTH; j++)
    {
        DstCentreXY[j] = sqrt((rp.x_arr[j] - CentrerXY.x)*(rp.x_arr[j] - CentrerXY.x) + (rp.y_arr[j] - CentrerXY.y)*(rp.y_arr[j] - CentrerXY.y));
    }

    r95Sort(DstCentreXY,0,HIS_LENGTH-1);

    rp.r95 = DstCentreXY[int(0.95*HIS_LENGTH)];

    //R95 = SQRT(meanErrx*meanErrx + meanErry*meanErry) + 2*SQRT(stdx*stdx+stdy*stdy)
    //rp.r95 = sqrt((rp.averr_x*rp.averr_x) + (rp.averr_y*rp.averr_y)) +
    //        2.0 * sqrt((rp.std_x*rp.std_x) + (rp.std_y*rp.std_y)) ;


    //update the value in the array
    rp.x_arr[idx] = x;
    rp.y_arr[idx] = y;
    rp.z_arr[idx] = z;

    rp.arr_idx++;
    //wrap the index
    if(rp.arr_idx >= HIS_LENGTH)
    {
        rp.arr_idx = 0;
        rp.ready = true;
    }

    rp.count++;

    //update the list entry
    _tagList.replace(i, rp);

    if(rp.ready)
    {
        if(_graphicsWidgetReady)
        {
            emit tagStats(id, CentrerXY.x, CentrerXY.y, rp.av_z, rp.r95);
        }

        //log data to file
        QString s = nowstr + QString("TS:%1 avx:%2 avy:%3 avz:%4 r95:%5\n").arg(id).arg(rp.av_x).arg(rp.av_y).arg(rp.av_z).arg(rp.r95);
        QTextStream ts( _file );
        ts << s;

        rp.ready = false;
    }
}

void RTLSClient::newData()
{
    QByteArray data = _serial->readAll();
    QDateTime now = QDateTime::currentDateTime();
    QString nowstr = now.toString("T:hhmmsszzz:");
    int length = data.length();
    int idx = 0, lastSeq = 0, count = 0;
    bool trilaterate = false;
    vec3d report;
    bool newposition = false;
    int offset = 0;
    int nolocation = 0;
    QString statusMsg;

    while(length >= TOF_REPORT_LEN) //we have received a range report from an anchor or a tag
    {
        while(length >= TOF_REPORT_LEN)
        {
            QByteArray header = data.mid(offset, 2);

            if(header.contains("ma")) //loop here until we reach header ("ma")
            {
                break;
            }

            offset += 2;
            length -= 2;
        }

        if(length < TOF_REPORT_LEN)
        {
            return;
        }


        QByteArray tofReport = data.mid(offset, TOF_REPORT_LEN);

        //e.g. ma00 t00 000005d4 00000520 000008d5 46 00000000 4022 4022 a0
        int range_corrected = 0;
        int aid, tid, range, lnum, seq, i;
        int txant, rxant, raw_range, rangetime;
        char c;
        int n = sscanf(tofReport.constData(),"ma%x t%x %x %x %x %x %x %x %x %c%d", &aid, &tid, &range, &raw_range, &lnum, &seq, &rangetime, &txant, &rxant, &c, &i);

        aid &= 0x3;

        //qDebug() << "anc"<< aid << "tag" << tid << "range(mm)" << range ;
        //qDebug() << "number"<< lnum << "seq" << seq << c << i ;

        //qDebug() << n << QString::fromLocal8Bit(tofReport, TOF_REPORT_LEN);

        offset += TOF_REPORT_LEN;
        length -= TOF_REPORT_LEN;

        if(n != TOF_REPORT_ARGS)
        {
            QString string1 = QString::fromLocal8Bit(tofReport, TOF_REPORT_LEN);
            qDebug() << n << string1;
            continue;
        }

        //qDebug() << tofReport.constData();

        //notify the user if connected to a tag or an anchor
        if(_first)
        {
            if(c == 'a') //we are connected to an anchor id = i
            {
                statusMsg = "Connected to Anchor ID" + QString(" %1.").arg(i);

                emit statusBarMessage(statusMsg);
            }
            else
            if(c == 't') //we are connected to a tag id = i
            {
                statusMsg = "Connected to Tag ID" + QString(" %1.").arg(i);

                emit statusBarMessage(statusMsg);
            }
            else
            if(c == 'l') //we are connected to a listener id = i
            {
                statusMsg = "Connected to Listener ID" + QString(" %1.").arg(i);

                emit statusBarMessage(statusMsg);
            }
            //log the anchor co-ordinates to the file
            for(int j=0; j<MAX_NUM_ANCS; j++)
            {
                {
                    QString s =  nowstr + QString("AP:%1:%2:%3:%4\n").arg(j).arg(_ancArray[j].x).arg(_ancArray[j].y).arg(_ancArray[j].z);
                    QTextStream ts( _file );
                    ts << s;
                }
            }
            _first = false;
        }

        range_corrected = range + (_ancArray[aid].tagRangeCorection[tid] * 10); //range correction is in cm (range is in mm)

        //qDebug() << "a and t " << aid << tid << "correction = " << (_ancArray[aid].tagRangeCorection[tid] * 0.01);

        //log data to file
        {
            QString s =  nowstr + QString("RR:%1:%2:%3:%4:%5:%6\n").arg(tid).arg(aid).arg(range).arg(range_corrected).arg(seq).arg(lnum);
            QTextStream ts( _file );
            ts << s;
        }

        //find the tag in the list
        for(idx=0; idx<_tagList.size(); idx++)
        {
            //find this tag in the list
            if(_tagList.at(idx).id == tid)
                break;
        }

        //if we don't have this tag in the list add it
        if(idx == _tagList.size())
        {
            initialiseTagList(tid);
        }

        tag_reports_t rp = _tagList.at(idx);

        emit tagRange(tid, aid, (range_corrected * 0.001)); //convert to meters

        if(rp.rangeSeq == seq) // we have already received range from one or more anchors with this seq
        {
            rp.rangeCount[seq & 0xFF]++;
            rp.rangeSeq = seq & 0xFF;
            rp.rangeValue[seq][aid & 0x3] = range_corrected;
        }
        else //this is the 1st range from this range sequence
        {
            rp.rangeCount[seq & 0xFF] = 1;
            rp.rangeSeq = seq & 0xFF;
            rp.rangeValue[seq][aid & 0x3] = range_corrected;

            trilaterate = true;
        }

        if(trilaterate)
        {
            lastSeq = (seq-1) & 0xFF ;
            count = rp.rangeCount[lastSeq] ;

            //we got next range seq. lets try and trilaterate the previous
            if(count >= 3)
            {
                //qDebug() << "try to get location" ;

                if(calculateTagLocation(&report, count, &rp.rangeValue[lastSeq][0] ) == TRIL_3SPHERES)
                {
                    newposition = true;
                    rp.numberOfLEs++;
                    //log data to file
                    {
                        QString s = nowstr + QString("LE:%1:%2:%3:[%4,%5,%6]:").arg(tid).arg(rp.numberOfLEs).arg(lastSeq).arg(report.x).arg(report.y).arg(report.z) +
                                QString("%1:%2:%3:%4\n").arg(rp.rangeValue[lastSeq][0]).arg(rp.rangeValue[lastSeq][1]).arg(rp.rangeValue[lastSeq][2]).arg(rp.rangeValue[lastSeq][3]);
                        QTextStream ts( _file );
                        ts << s;
                    }

                    //qDebug() << "emit tagPos" << rp.numberOfLEs;
//                       data_struct_t dataToSend;
//                       tag_reports_t data1;
//                       data1.av_x =  report.x;
//                        data1.av_y =  report.y;
//                        data1.av_z =  report.z;
//                       tag_reports_t data2 =  _tagList.at(0);
//                      anc_struct_t data3 = _ancArray[0];

//                       dataToSend = RTLSDisplayApplication::sender()->CreateData(&data1,&data2,&data3);
//                       RTLSDisplayApplication::sender()->SendData(dataToSend);

                   RTLSDisplayApplication::safeLogging()->TagReport(tid, report.x, report.y, report.z);
                    emit tagPos(tid, report.x, report.y, report.z); //send the update to graphic

                    if(nolocation)
                    {
                        emit statusBarMessage("");
                    }

                    nolocation = 0;

                }
                else //no solution
                {
                    nolocation++;

                    //log data to file
                    {
                        QString s = nowstr + QString("NL:%1:%2:%3:[nan,nan,nan]:").arg(tid).arg(rp.numberOfLEs).arg(lastSeq) +
                                QString("%1:%2:%3:%4\n").arg(rp.rangeValue[lastSeq][0]).arg(rp.rangeValue[lastSeq][1]).arg(rp.rangeValue[lastSeq][2]).arg(rp.rangeValue[lastSeq][3]);
                        QTextStream ts( _file );
                        ts << s;
                    }

                    if( nolocation >= 5)
                    {
                        emit statusBarMessage("No location solution.");
                    }
                }

            }
            //clear the count
            rp.rangeCount[lastSeq] = 0;
        }

        //update the list entry
        _tagList.replace(idx, rp);

        //update statistics if new position has been calculated
        if(newposition)
        {
            updateTagStatistics(idx, report.x, report.y, report.z);
        }

        //qDebug() << "newposition" << newposition << idx << lastSeq << seq;
    }

}

int RTLSClient::calculateTagLocation(vec3d *report, int count, int *ranges)
{
    //change so it checks both arrays.
    int result = 0;
    vec3d anchorArray[4];

    anchorArray[0].x = _ancArray[0].x;
    anchorArray[0].y = _ancArray[0].y;
    anchorArray[0].z = _ancArray[0].z;

    anchorArray[1].x = _ancArray[1].x;
    anchorArray[1].y = _ancArray[1].y;
    anchorArray[1].z = _ancArray[1].z;

    anchorArray[2].x = _ancArray[2].x;
    anchorArray[2].y = _ancArray[2].y;
    anchorArray[2].z = _ancArray[2].z;

    anchorArray[3].x = _ancArray[3].x;
    anchorArray[3].y = _ancArray[3].y;
    anchorArray[3].z = _ancArray[3].z;

    result = GetLocation(report, (count==4) ? 1 : 0, &anchorArray[0], ranges);

    return result;
}

void RTLSClient::setGWReady(bool set)
{
    _graphicsWidgetReady = set;
}

void r95Sort (double s[], int l, int r)
{
    int i,j;
    double x;
    if(l<r)
    {
        i = l;
        j = r;
        x = s[i];
        while(i<j)
        {
            while (i<j&&s[j]>x) j--;
            if (i<j) s[i++] = s[j];
            while (i<j&&s[i]<x) i++;
            if (i < j) s[j--] = s[i];
        }
        s[i] = x;
        r95Sort(s, l, i-1);
        r95Sort(s, i+1, r);
    }

}

void RTLSClient::connectionStateChanged(SerialConnection::ConnectionState state)
{
    qDebug() << "RTLSClient::connectionStateChanged " << state;

    if(state == SerialConnection::Disconnected) //disconnect from Serial Port
    {
        if(_serial != NULL)
        {
            disconnect(_serial, SIGNAL(readyRead()), this, SLOT(newData()));
            _serial = NULL;
        }
        _file->close(); //close the Log file
    }

}

void RTLSClient::addMissingAnchors(void)
{
    int i;

    //check how many anchors were added and add placeholders for any missing ones
    for(i=0; i<MAX_NUM_ANCS; i++)
    {
        if(_ancArray[i].id == 0xff)
        {
            _ancArray[i].id = i;
            emit anchPos(i, _ancArray[i].x, _ancArray[i].y, _ancArray[i].z, false);
        }
    }

    emit centerOnAnchors();
}

void RTLSClient::loadConfigFile(QString filename)
{
    QFile file(filename);

    double x[4] = {0.0, 5.0, 0.0, 5.0};
    double y[4] = {0.0, 0.0, 5.0, 5.0};

    for(int i=0; i<MAX_NUM_ANCS; i++)
    {
        _ancArray[i].id = 0xff;
        _ancArray[i].label = "";
        _ancArray[i].x = x[i];  //default x
        _ancArray[i].y = y[i];  //default y
        _ancArray[i].z = 3.00;  //default z

        for(int j = 0; j<MAX_NUM_TAGS; j++)
        {
            _ancArray[i].tagRangeCorection[j] =  0;
        }
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(file.errorString())));
        addMissingAnchors();
        return;
    }

    QDomDocument doc;
    QString error;
    int errorLine;
    int errorColumn;

    if(doc.setContent(&file, false, &error, &errorLine, &errorColumn))
    {
        qDebug() << "file error !!!" << error << errorLine << errorColumn;
    }

    QDomElement config = doc.documentElement();

    if( config.tagName() == "config" )
    {
        QDomNode n = config.firstChild();
        while( !n.isNull() )
        {
            QDomElement e = n.toElement();
            if( !e.isNull() )
            {
                if( e.tagName() == "anc" )
                {
                    bool ok;
                    int id = (e.attribute( "ID", "" )).toInt(&ok);

                    id &= 0x3;

                    if(ok)
                    {
                        _ancArray[id].id = id & 0xf;
                        _ancArray[id].label = (e.attribute( "label", "" ));
                        _ancArray[id].x = (e.attribute("x", "0.0")).toDouble(&ok);
                        _ancArray[id].y = (e.attribute("y", "0.0")).toDouble(&ok);
                        _ancArray[id].z = (e.attribute("z", "0.0")).toDouble(&ok);

                        //tag distance correction (in cm)
                        _ancArray[id].tagRangeCorection[0] = (e.attribute("t0", "0")).toDouble(&ok);
                        _ancArray[id].tagRangeCorection[1] = (e.attribute("t1", "0")).toDouble(&ok);
                        _ancArray[id].tagRangeCorection[2] = (e.attribute("t2", "0")).toDouble(&ok);
                        _ancArray[id].tagRangeCorection[3] = (e.attribute("t3", "0")).toDouble(&ok);
                        _ancArray[id].tagRangeCorection[4] = (e.attribute("t4", "0")).toDouble(&ok);
                        _ancArray[id].tagRangeCorection[5] = (e.attribute("t5", "0")).toDouble(&ok);
                        _ancArray[id].tagRangeCorection[6] = (e.attribute("t6", "0")).toDouble(&ok);
                        _ancArray[id].tagRangeCorection[7] = (e.attribute("t7", "0")).toDouble(&ok);

                        if(id == 3) //hide anchor 4 by default
                        {
                            emit anchPos(id, _ancArray[id].x, _ancArray[id].y, _ancArray[id].z, false);
                        }
                        else
                        {
                            emit anchPos(id, _ancArray[id].x, _ancArray[id].y, _ancArray[id].z, true);
                        }
                    }
                }
            }

            n = n.nextSibling();
        }

    }

    file.close();

    addMissingAnchors();
}

QDomElement AnchorToNode( QDomDocument &d, anc_struct_t * anc )
{
    QDomElement cn = d.createElement( "anc" );
    cn.setAttribute("ID", QString::number(anc->id));
    cn.setAttribute("label", anc->label);
    cn.setAttribute("x", anc->x);
    cn.setAttribute("y", anc->y);
    cn.setAttribute("z", anc->z);
    cn.setAttribute("t0", anc->tagRangeCorection[0]);
    cn.setAttribute("t1", anc->tagRangeCorection[1]);
    cn.setAttribute("t2", anc->tagRangeCorection[2]);
    cn.setAttribute("t3", anc->tagRangeCorection[3]);
    cn.setAttribute("t4", anc->tagRangeCorection[4]);
    cn.setAttribute("t5", anc->tagRangeCorection[5]);
    cn.setAttribute("t6", anc->tagRangeCorection[6]);
    cn.setAttribute("t7", anc->tagRangeCorection[7]);

    return cn;
}

void RTLSClient::saveConfigFile(QString filename)
{
    QFile file( filename );
    int i = 0;

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug(qPrintable(QString("Error: Cannot read file %1 %2").arg(filename).arg(file.errorString())));
        return;
    }

    QDomDocument doc;

    // Adding tag config root
    QDomElement config = doc.createElement("config");
    doc.appendChild(config);

    while (i < MAX_NUM_ANCS)
    {
        config.appendChild( AnchorToNode(doc, &_ancArray[i]) );

        i++;
    }

    QTextStream ts( &file );
    ts << doc.toString();

    file.close();

    if(_file)
    {
        _file->flush();
    }

    qDebug() << doc.toString();
}


void RTLSClient::updateAnchorXYZ(int id, int x, double value)
{
    QDateTime now = QDateTime::currentDateTime();
    QString nowstr = now.toString("T:hhmmsszzz:");
    if(x == 1)
    {
        _ancArray[id].x = value;
    }
    else if (x == 2)
    {
        _ancArray[id].y = value;
    }
    else if (x == 3)
    {
        _ancArray[id].z = value;
    }

    if(_file)
    {
        QString s =  nowstr + QString("AP:%1:%2:%3:%4\n").arg(id).arg(_ancArray[id].x).arg(_ancArray[id].y).arg(_ancArray[id].z);
        QTextStream ts( _file );
        ts << s;
    }
}


void RTLSClient::updateTagCorrection(int aid, int tid, int value)
{
    tid &= 0x7;

    _ancArray[aid].tagRangeCorection[tid] = value;

}

int* RTLSClient::getTagCorrections(int anchID)
{
    return &_ancArray[anchID].tagRangeCorection[0];
}
