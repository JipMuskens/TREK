#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include "SafeLogging.h"
#include "RTLSDisplayApplication.h"
#include "Sender.h"
#include "WebsiteClient.h"
#include "mainwindow.h"
#include "ViewSettingsWidget.h"
#include "GraphicsWidget.h"


SafeLogging::SafeLogging()
{
    SearchItemIndex = 0;
    logmode = 2;
    filename="DebugLog.txt";
    ChangeTollerance = 1;
}

void SafeLogging::SendData()
{
    qDebug() << "SendData: Tag Count = " << _tagLogList.count();
    int count = _tagLogList.count();
    int i = 0;
    while(i<count)
    {
        tag_reports_t temp = _tagLogList.at(i);
        qDebug() << "tag: "<< temp.id;
        i++;
    }
    if(_tagLogList.count() > 0)
    {
        int Y = SafeLogging::SearchTagindex(0);
        if(Y != 100)
        {
            tag_reports_t GlassesData = _tagLogList.at(Y);
            tag_reports_t TagData;
            anc_struct_t Anc0;
            data_struct_t DataToSend;
            if(_tagLogList.count() >= 2)
            {
               // qDebug() << "SendData: added index " << SearchItemIndex <<" to second report of datapackage";
                TagData = _tagLogList.at(SearchItemIndex);
            }
            else
            {
               // qDebug() << "SendData: [temp] added index 0 to second report of datapackage!!!! not good!!";
                TagData = _tagLogList.at(0);
            }
            DataToSend = RTLSDisplayApplication::sender()->CreateData(&GlassesData,&TagData,&Anc0);
            RTLSDisplayApplication::sender()->SendData(DataToSend);
        }
        else
        {
            qDebug() << "SafeLogging: No Glasses detected!";
        }
        QList <tag_reports_t> Tags = _tagLogList;
        RTLSDisplayApplication::websiteClient()->SendJSONData(&Tags);
    }
    else
    {
        qDebug() << "SafeLogging: No Data!";
    }
}

void SafeLogging::TagReport(int tid,double x,double y,double z)
{
    int i = 0;
    int count = _tagLogList.count();
    if(count > 0)
    {
        if(!SearchTagReport(tid))
        {
            qDebug() << "TagReport: Adding tag " << tid <<" to list";
            tag_reports_t Tag;
            memset(&Tag, 0, sizeof(tag_reports_t));
            Tag.id = tid;
            Tag.av_x = x;
            Tag.av_y = y;
            Tag.av_z = z;
            _tagLogList.insert(count,Tag);
            _tagLoggedData.insert(count,Tag);
            RTLSDisplayApplication::mainWindow()->viewSettingsWidget()->AddItemToComboBox(tid);
        }
        while(i<count)
        {
            if(_tagLogList.at(i).id == tid)
            {
                tag_reports_t edit = _tagLogList.at(i);
                edit.av_x = x;
                edit.av_y = y;
                if(z <= 5)
                {
                    edit.av_z = z;
                }
                _tagLogList.replace(i, edit);
                 //qDebug() << "TagReport: Data refreshed";
            }
            i++;
        }
    }
    else
    {
        qDebug() << "TagReport: Adding tag " << tid <<" to list";
        tag_reports_t Tag;
        memset(&Tag, 0, sizeof(tag_reports_t));
        Tag.id = tid;
        Tag.av_x = x;
        Tag.av_y = y;
        Tag.av_z = z;
        _tagLogList.append(Tag);
        _tagLoggedData.append(Tag);
        RTLSDisplayApplication::mainWindow()->viewSettingsWidget()->AddItemToComboBox(tid);
    }
}

void SafeLogging::ChangeSearchItem(int index)
{
    qDebug() << "SearchItemChanged to: " << index;
    SearchItemIndex = index;
}

void SafeLogging::AddToProgramLog(QString Data)
{
    switch (logmode)
    {
        case 0://No Log file, No Debug.
        break;
        case 1://Only Log file.
        SafeLogging::WriteLog(Data, filename);
        break;
        case 2:
        //Only Debug.
        qDebug() << "Debug: " << Data;
        break;
        case 3:
        //Log File and Debug.
        qDebug() << "Debug: " << Data;
        SafeLogging::WriteLog(Data, filename);
        break;
        default:
        //If something else.
        break;
    }
}

bool SafeLogging::SearchTagReport(int tid)
{
 //qDebug() << "SearchTagReport: Start Search...";
 int count = _tagLogList.count();
 //qDebug() << "SearchTagReport: Count = " << count;
 tag_reports_t temp;
 int i = 0;
 while(i<count)
 {
     temp = _tagLogList.at(i);
     if(temp.id == tid)
     {
         //qDebug() << "SearchTagReport: Tag found! :)";
         return true;
     }
     i++;
 }
 //qDebug() << "SearchTagReport: Tag not found! :(";
 return false;
}

int SafeLogging::SearchTagindex(int tid)
{
 int count = _tagLogList.count();
 tag_reports_t temp;
 int i = 0;
 while(i<count)
 {
     temp = _tagLogList.at(i);
     if(temp.id == tid)
     {
         //qDebug() << "SearchTagIndex: Found tag " << tid <<" at index: " << i;
         return i;
     }
     i++;
 }
 return 100;
 qDebug() << "SearchTagIndex: Tag " << tid <<" not found!";
}

void SafeLogging::WriteLog(QString Data,QString Filename)
{
    QFile file( Filename );
    if ( file.open(QIODevice::ReadWrite) )
    {
        file.seek(file.size());
        QTextStream stream( &file );
        QDateTime now = QDateTime::currentDateTime();
        QString Log = now.toString("yyyy/MM/dd hh:mm:ss")+ " : " + Data + '\r' + '\n';
        stream << Log << endl;
        file.close();
    }
}

void SafeLogging::LogData()
{
    //qDebug() << "Logging data...";
    //qDebug() << "Tollerance: " << ChangeTollerance;
        for(int i = 0; i < _tagLoggedData.count(); i++)
        {
            bool changed = false;
            tag_reports_t OldPos = _tagLoggedData.at(i);
            tag_reports_t NewPos = _tagLogList.at(SafeLogging::SearchTagindex(OldPos.id));
            if(NewPos.av_x >= OldPos.av_x + ChangeTollerance || NewPos.av_x <= OldPos.av_x - ChangeTollerance)
            {
                changed = true;
            }
            else if(NewPos.av_y >= OldPos.av_y + ChangeTollerance || NewPos.av_y <= OldPos.av_y - ChangeTollerance)
            {
                changed = true;
            }
            else if(NewPos.av_z >= OldPos.av_z + ChangeTollerance || NewPos.av_z <= OldPos.av_z - ChangeTollerance)
            {
                changed = true;
            }
            if(changed)
            {
               qDebug() << "Writing data to log...";
               QString Data = "Tag: ";
               Data += RTLSDisplayApplication::mainWindow()->graphicsWidget()->SearchListIndex(NewPos.id);
               Data += " Changed position to: ";
               Data += QString::number(NewPos.av_x) + " , " + QString::number(NewPos.av_y) + " , " + QString::number(NewPos.av_z) + "\n";
               SafeLogging::WriteLog(Data,"TagLog.txt");
            }
            _tagLoggedData.replace(i, NewPos);
        }
}

void SafeLogging::ChangeLogMode(int Mode)
{
    logmode = Mode;
    QString log = "ChangeLogMode << Logmode changed to:" + QString::number(Mode);
    SafeLogging::AddToProgramLog(log);
}

void SafeLogging::Tollerance(int amount)
{
    //qDebug() << "Tollerance changed to: " << amount;
    if(amount >= 0)
    {
        //qDebug() << "Tollerance changed to: " << amount;
        QString log = "Tollerance << Tollerance changed to:" + QString::number(amount);
        SafeLogging::AddToProgramLog(log);
        ChangeTollerance = amount;
    }
}
