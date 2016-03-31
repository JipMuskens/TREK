// -------------------------------------------------------------------------------------------------------------------
//
//  File: SerialConnection.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "SerialConnection.h"

#include <QDebug>
#include <QSerialPortInfo>
#include <QMessageBox>

#define DEVICE_STR ("STMicroelectronics Virtual COM Port")
#define INST_REPORT_LEN   (20)
#define INST_VERSION_LEN  (16)
#define INST_CONFIG_LEN   (1)

SerialConnection::SerialConnection(QObject *parent) :
    QObject(parent)
{
    _serial = new QSerialPort(this);

    connect(_serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

    connect(_serial, SIGNAL(readyRead()), this, SLOT(readData()));

    _processingData = true;

}

SerialConnection::~SerialConnection()
{
    if(_serial->isOpen())
        _serial->close();

    delete _serial;
}

QStringList SerialConnection::portsList()
{
    return _ports;
}

void SerialConnection::findSerialDevices()
{
    _portInfo.clear();
    _ports.clear();

    foreach (const QSerialPortInfo &port, QSerialPortInfo::availablePorts())
    //for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
    {
        //Their is some sorting to do for just list the port I want, with vendor Id & product Id
        qDebug() << port.portName() << port.vendorIdentifier() << port.productIdentifier()
                 << port.hasProductIdentifier() << port.hasVendorIdentifier() << port.isBusy()
                 << port.manufacturer() << port.description();


        if(port.description() == DEVICE_STR)
        {
            _portInfo += port;
            _ports += port.portName();
        }
    }
}

int SerialConnection::openSerialPort(QSerialPortInfo x)
{
    int error = 0;
    _serial->setPort(x);

    if(!_serial->isOpen())
    {
        if (_serial->open(QIODevice::ReadWrite))
        {
            _serial->setBaudRate(QSerialPort::Baud115200/*p.baudRate*/);
            _serial->setDataBits(QSerialPort::Data8/*p.dataBits*/);
            _serial->setParity(QSerialPort::NoParity/*p.parity*/);
            _serial->setStopBits(QSerialPort::OneStop/*p.stopBits*/);
            _serial->setFlowControl(QSerialPort::NoFlowControl /*p.flowControl*/);


            emit statusBarMessage(tr("Connected to %1").arg(x.portName()));

            qDebug() << "send \"deca$\"" ;
            writeData("deca$");
            //writeData("deca?");

            //emit serialOpened(); - wait until we get reply from the unit

        }
        else
        {
            //QMessageBox::critical(NULL, tr("Error"), _serial->errorString());

            emit statusBarMessage(tr("Open error"));

            qDebug() << "Serial error: " << _serial->error();

            _serial->close();

            emit serialError();

            error = 1;
        }
    }
    else
    {
        qDebug() << "port already open!";

        error = 0;
    }

    return error;
}

int SerialConnection::openConnection(int index)
{
    QSerialPortInfo x;
    int foundit = -1;
    int open = false;

    foreach (const QSerialPortInfo &port, QSerialPortInfo::availablePorts())
    {
        if(port.description() == DEVICE_STR)
        {
            foundit++;
            if(foundit==index)
            {
                x = port;
                open = true;
                break;
            }
        }
    }

    qDebug() << "is busy? " << x.isBusy() << "index " << index << " = found " << foundit;

    if(!open) return -1;

    qDebug() << "open serial port " << index << x.portName();

    //open serial port
    return openSerialPort(x);
}

void SerialConnection::closeConnection()
{
    _serial->close();
    emit statusBarMessage(tr("COM port Disconnected"));
    emit connectionStateChanged(Disconnected);

    _processingData = true;
}

void SerialConnection::writeData(const QByteArray &data)
{
    if(_serial->isOpen())
    {
        _serial->write(data);
        //waitForData = true;
    }
    else
    {
        qDebug() << "not open - can't write?";
    }

    emit connectionStateChanged(Connected);
}


void SerialConnection::cancelConnection()
{
    emit connectionStateChanged(ConnectionFailed);
}


void SerialConnection::readData(void)
{
    if(_processingData)
    {
        QByteArray data = _serial->readAll();
        int length = data.length();
        int offset = 0;

        if(length >= INST_REPORT_LEN)
        {
            while(length >= INST_REPORT_LEN)
            {
                QByteArray header = data.mid(offset, 2);

                //qDebug() << data ;

                if(header.contains("nV")) //loop here until we reach header ("nV")
                {
                    break;
                }

                offset += 2;
                length -= 2;
            }

            if(length < INST_REPORT_LEN)
            {
                return;
            }

            QByteArray instanceVer = data.mid(1, INST_VERSION_LEN);
            QByteArray instanceConf = data.mid(1+INST_VERSION_LEN, INST_CONFIG_LEN);

            //e.g. nVersion X.Y TREKZ
            _conncectionConfig = QString::fromLocal8Bit(instanceConf, INST_CONFIG_LEN);
            _connectionVersion = QString::fromLocal8Bit(instanceVer, INST_VERSION_LEN);

            _processingData = false;

            emit serialOpened(_connectionVersion, _conncectionConfig);
        }
    }
}

void SerialConnection::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        //QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        _serial->close();

        _processingData = true;
    }
}


