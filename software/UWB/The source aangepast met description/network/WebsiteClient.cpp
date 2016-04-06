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

WebsiteClient::WebsiteClient()
{
    m_webSocket.open(QUrl("ws://uwb.azurewebsites.net/api/Webaccess/Get"));
}

WebsiteClient::~WebsiteClient()
{
    m_webSocket.close();
}

void WebsiteClient::SendJSONData(QList <tag_reports_t> *Tags)
{
    m_webSocket.sendTextMessage(WebsiteClient::ConvertToJSON(Tags));
}

QByteArray WebsiteClient::ConvertToJSON(QList <tag_reports_t> *Tags)
{
    QJsonArray Json;
    for(int i = 0; i < Tags->size(); i++)
    {
        tag_reports_t Tag = Tags->at(i);
        QJsonObject tag;
        tag.insert("ID", Tag.id);
        tag.insert("X", Tag.av_x);
        tag.insert("Y", Tag.av_y);
        tag.insert("Z", Tag.av_z);
        Json << tag;
    }
    QJsonDocument doc(Json);
    QByteArray bytes = doc.toJson();

    qDebug() << bytes;
    return bytes;
}
