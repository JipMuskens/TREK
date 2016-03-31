/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>
#include <string.h>

#include "Sender.h"
#include "RTLSDisplayApplication.h"
#include "mainwindow.h"
#include "GraphicsWidget.h"




Sender::Sender()
{
    udpSocket = new QUdpSocket();
    alarm = false;
}

data_struct_t Sender::CreateData(tag_reports_t *Glasses , tag_reports_t *Tag ,anc_struct_t *anc0)
{
    data_struct_t data;
    data.xGlass = Glasses->av_x;
    data.yGlass = Glasses->av_y;
    data.zGlass = Glasses->av_z;
    data.IdGlass = Glasses->id;

    data.xTag = Tag->av_x;
    data.yTag = Tag->av_y;
    data.zTag = Tag->av_z;
    data.IdTag = Tag->id;

    data.xAnc0 = anc0->x;
    data.yAnc0 = anc0->y;
    data.zAnc0 = anc0->z;
    //qDebug() << "CreateData: Data created";
    return data;
}

QByteArray Sender::serialize(data_struct_t datagram)
{
    QByteArray byteArray;

    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_5);

    stream << datagram.xGlass
           << datagram.yGlass
           << datagram.zGlass
           << datagram.IdGlass
           << datagram.xTag
           << datagram.yTag
           << datagram.zTag
           << datagram.IdTag
           << datagram.xAnc0
           << datagram.yAnc0
           << datagram.zAnc0;

    return byteArray;
}

void Sender::ChangeAlarmState()
{
        if(alarm)
        {
            alarm = false;
        }
        else
        {
            alarm = true;
        }
        qDebug() << "Alarmstate changed";
}

void Sender::SendData(data_struct_t datagram)
{
    //Methode 1 is de enige die werkt naar c# toe, omdat er iets fout gaat met het de-serializen van de data.
    //we krijgen alleen willekeurige data binnen bij het casten naar de struct.
    // de "broadcastreceiver" die gemaakt is kan alles wel goed ontvangen.
    bool debug = 0;
    if(debug)
    {
    qDebug() << "-------------------------------------------";
    qDebug() << "SendData: Data Send with the following data...";
    qDebug() << "xGlass:" << datagram.xGlass;
    qDebug() << "yGlass:" << datagram.yGlass;
    qDebug() << "zGlass:" << datagram.zGlass;
    qDebug() << "IdGlass:" << datagram.IdGlass;
    qDebug() << "xTag:" << datagram.xTag;
    qDebug() << "yTag:" << datagram.yTag;
    qDebug() << "zTag:" << datagram.zTag;
    qDebug() << "IdTag:" << datagram.IdTag;
    qDebug() << "xAnc:" << datagram.xAnc0;
    qDebug() << "yAnc:" << datagram.yAnc0;
    qDebug() << "zAnc:" << datagram.zAnc0;
    qDebug() << "-------------------------------------------";
    qDebug() << "";
    }

    int method = 1;

    if(method == 1)
    {
        char jestring[150];
        QString IDGlass = RTLSDisplayApplication::mainWindow()->graphicsWidget()->SearchListIndex(datagram.IdGlass);
        QString IDTag = RTLSDisplayApplication::mainWindow()->graphicsWidget()->SearchListIndex(datagram.IdTag);
        //qDebug() << "IDGlass: " << datagram.IdGlass << " ,IDTag: " << datagram.IdTag;
        QString Complete = IDGlass;
        Complete += "," + IDTag;
        char* cstr;
        cstr = new char [Complete.size()+1];
        strcpy( cstr, Complete.toLatin1() );
        //qDebug() << cstr;

        qDebug() << "SendData << IDGlasses changed to: " << IDGlass << " ,IDTag changed to: " << IDTag;
        snprintf(jestring, 150, "%f,%f,%f,%f,%f,%f,%s,%d", datagram.xGlass, datagram.yGlass, datagram.zGlass, datagram.xTag, datagram.yTag, datagram.zTag,cstr ,alarm);
        //qDebug() << jestring;

        int s = strlen((char*)jestring);

        DataToSend = QByteArray(jestring,sizeof(jestring));
        udpSocket->writeDatagram(DataToSend.data(), s,
        QHostAddress::Broadcast, 45454);
        qDebug() << "SendData << DataSend";
    }
    else if(method == 2)
    {
        QByteArray bqba = serialize(datagram);
        udpSocket->writeDatagram(bqba.data(), bqba.size(),
        QHostAddress::Broadcast, 45454);
    }
}
