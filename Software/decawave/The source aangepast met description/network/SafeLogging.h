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
    enum LogMode
    {
        NoLogNoDebug = 0,
        LogOnly,
        DebugOnly,
        LogAndDebug
    };

public slots:
    void SendData();
    void LogData();
    void LogDeData();
    void ChangeLogMode(int Mode);
    void Tollerance(int amount);

public:
    SafeLogging();
    void TagReport(int,double,double,double);
    void ChangeSearchItem(int);
    void AddToProgramLog(QString Data);

private:
    bool SearchTagReport(int);
    int SearchTagindex(int);
    void WriteLog(QString Data, QString Filename);
    QList <tag_reports_t> _tagLogList;
    QList <tag_reports_t> _tagLoggedData;
    int SearchItemIndex;
    int logmode;
    int ChangeTollerance;
    QString filename;
};

#endif // LOG_H

