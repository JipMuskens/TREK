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

#include "sender.h"


Sender::Sender(QWidget *parent)
    : QWidget(parent)
{
    statusLabel = new QLabel(tr("Ready to broadcast datagrams on port 45454"));
    statusLabel->setWordWrap(true);

    startButton = new QPushButton(tr("&Start"));
    dataButton = new QPushButton(tr("&Send!!"));
    quitButton = new QPushButton(tr("&Quit"));


    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(dataButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    timer = new QTimer(this);
//! [0]
    udpSocket = new QUdpSocket(this);
//! [0]

    Testdata.xGlass = 107;
    Testdata.yGlass = 2;
    Testdata.zGlass = 3;
    Testdata.IdGlass = 00;
    Testdata.xTag = 4;
    Testdata.yTag = 5;
    Testdata.zTag = 6;
    Testdata.IdTag = 00;
    Testdata.xAnc0 = 666;
    Testdata.yAnc0 = 8;
    Testdata.zAnc0 = 9;
    temp = 0;

    dataswitch = false;

    connect(startButton, SIGNAL(clicked()), this, SLOT(startBroadcasting()));
     connect(dataButton, SIGNAL(clicked()), this, SLOT(RealDataTest()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(timer, SIGNAL(timeout()), this, SLOT(broadcastDatagram()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Broadcast Sender"));
}

void Sender::startBroadcasting()
{
    startButton->setEnabled(false);
    timer->start(100);
}

void Sender::RealDataTest()
{
    startButton->setEnabled(true);
    timer->stop();


    tag_reports_t report1;
    tag_reports_t report2;
    anc_struct_t report3;

    report1.av_x = 0;
    report1.av_y = 0;
    report1.av_z = 0;
    report1.id = temp;

    report2.av_x = -2;
    report2.av_y = 0;
    report2.av_z = 0;
    report2.id = 02;

    report3.x = 0;
    report3.y = 2;
    report3.z = 3;

    maindata = CreateData(&report1,&report2,&report3);

    statusLabel->setText(tr("Handmatig bericht verzonden met echte data"));

    SendData(maindata);
    temp++;
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
    qDebug() << "CreateData: Data created";
    return data;
}

void Sender::broadcastDatagram()
{

    if(dataswitch)
    {
        DataToSend = QByteArray("test data");
        statusLabel->setText(tr("Bericht verzonden...")); //Debuggging
        udpSocket->writeDatagram(DataToSend.data(), DataToSend.size(),
        QHostAddress::Broadcast, 45454);
        dataswitch = false;
    }
    else
    {
        Testdata.IdTag++;
        SendData(Testdata);
        //dataswitch = true;
    }
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

void Sender::SendData(data_struct_t datagram)
{
    //Methode 1 is de enige die werkt naar c# toe, omdat er iets fout gaat met het de-serializen van de data.
    //we krijgen alleen willekeurige data binnen bij het casten naar de struct.
    // de "broadcastreceiver" die gemaakt is kan alles wel goed ontvangen.

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

    int method = 1;

    if(method == 1)
    {
        char jestring[100];
        snprintf(jestring, 100, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", datagram.xGlass, datagram.yGlass, datagram.zGlass, datagram.IdGlass, datagram.xTag, datagram.yTag, datagram.zTag, datagram.IdTag, datagram.xAnc0, datagram.yAnc0, datagram.zAnc0);
        DataToSend = QByteArray(jestring,sizeof(jestring));
        udpSocket->writeDatagram(DataToSend.data(), DataToSend.size(),
        QHostAddress::Broadcast, 45454);
    }
    else if(method == 2)
    {
        QByteArray bqba = serialize(datagram);
        udpSocket->writeDatagram(bqba.data(), bqba.size(),
        QHostAddress::Broadcast, 45454);
    }
}
