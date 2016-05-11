#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>
#include <string.h>

#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonValue>

#include "RTLSClient.h"
#include "websiteclient.h"
#include "RTLSDisplayApplication.h"

#include <QWebSocket.h>

WebsiteClient::WebsiteClient(QObject *parent) :
                             QObject(parent)
{
    ReceivedMessage = -1;
    m_webSocket.open(QUrl("ws://d33f.eu/uwb/api/Webaccess/Get"));
    connect(&m_webSocket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onTextMessageReceived(QString)));
}

WebsiteClient::~WebsiteClient()
{
    //m_webSocket.close();
}

void WebsiteClient::SendJSONData(QList <tag_reports_t> *Tags)
{
    m_webSocket.sendTextMessage(WebsiteClient::ConvertToJSON(Tags));
}

void WebsiteClient::onTextMessageReceived(QString message)
{
    qDebug() << "Message received:" << message;
    ReceivedMessage = message;
}

QByteArray WebsiteClient::ConvertToJSON(QList <tag_reports_t> *Tags)
{
    //QString message = m_webSocket.textMessageReceived;
    QJsonArray Json;
    for(int i = 0; i < Tags->size(); i++)
    {

        tag_reports_t Tag = Tags->at(i);
        qDebug() << "Tag ID: " << Tag.id;
        QString ID = QString::number(Tag.id);
        if(ReceivedMessage == ID)
        {
            QJsonObject tag;
            tag.insert("ID", Tag.id);
            tag.insert("X", Tag.av_x);
            tag.insert("Y", Tag.av_y);
            tag.insert("Z", Tag.av_z);
            Json << tag;
        }
    }
    if(Json.isEmpty())
    {
        return 0;
    }
    else
    {
        QJsonDocument doc(Json);
        QByteArray bytes = doc.toJson();

        qDebug() << bytes;
        return bytes;
    }
}
