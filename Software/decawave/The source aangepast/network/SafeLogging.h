#ifndef LOG_H
#define LOG_H

#include <QWidget>
#include <stdint.h>
#include <stdlib.h>
#include <QObject>

#include "RTLSClient.h"

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class SafeLogging : public QWidget
{
public slots:
    void SendData();
    void LogData();
    void LogDeData();

public:
    SafeLogging();
    void TagReport(int,double,double,double);
    void ChangeSearchItem(int);

private:
    bool SearchTagReport(int);
    int SearchTagindex(int);
    QList <tag_reports_t> _tagLogList;
    int SearchItemIndex;
};

#endif // LOG_H

