#ifndef SENDER_H
#define SENDER_H

#include <QWidget>
#include <stdint.h>
#include <stdlib.h>
#include <QObject>
#include "RTLSClient.h"

QT_BEGIN_NAMESPACE
class QUdpSocket;
QT_END_NAMESPACE

#define HIS_LENGTH 100

typedef struct
{
    double xGlass, yGlass, zGlass, IdGlass;
    double xTag, yTag, zTag, IdTag;
    double xAnc0, yAnc0, zAnc0;
} data_struct_t;

class Sender
{

public:
    Sender();
    void SendData(data_struct_t datagram);
    data_struct_t CreateData(tag_reports_t *Glasses , tag_reports_t *Tag ,anc_struct_t *anc0);
    QByteArray serialize(data_struct_t datagram);
    void ChangeAlarmState();

private:
    QUdpSocket *udpSocket;
    QByteArray DataToSend;
    bool alarm;
};

#endif
