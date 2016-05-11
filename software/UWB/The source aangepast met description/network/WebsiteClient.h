#ifndef WEBSITECLIENT_H
#define WEBSITECLIENT_H

#include "RTLSClient.h"
#include "RTLSDisplayApplication.h"

#include <QWidget>
#include <stdint.h>
#include <stdlib.h>
#include <QObject>
#include <QtWebSockets/QWebSocket>


class WebsiteClient : public QObject
{
public:
    WebsiteClient(QObject *parent = 0);
    ~WebsiteClient();
    void SendJSONData(QList <tag_reports_t> *Tags);
    QByteArray ConvertToJSON(QList <tag_reports_t> *Tags);
    QString ReceivedMessage;

private Q_SLOTS:
    void onTextMessageReceived(QString message);

private:
    QWebSocket m_webSocket;
};

#endif // WEBSITECLIENT_H
