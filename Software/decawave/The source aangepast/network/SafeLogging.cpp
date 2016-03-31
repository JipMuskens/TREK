#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>

#include "SafeLogging.h"
#include "RTLSDisplayApplication.h"
#include "Sender.h"
#include "mainwindow.h"
#include "ViewSettingsWidget.h"


SafeLogging::SafeLogging()
{
    SearchItemIndex = 0;
}

void SafeLogging::SendData()
{
    int count = _tagLogList.count();
    int i = 0;
    while(i<count)
    {
        tag_reports_t temp = _tagLogList.at(i);
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
                qDebug() << "SendData: added index " << SearchItemIndex <<" to second report of datapackage";
                TagData = _tagLogList.at(SearchItemIndex);
            }
            else
            {
                qDebug() << "SendData: [temp] added index 0 to second report of datapackage!!!! not good!!";
                TagData = _tagLogList.at(0);
            }
            DataToSend = RTLSDisplayApplication::sender()->CreateData(&GlassesData,&TagData,&Anc0);
            RTLSDisplayApplication::sender()->SendData(DataToSend);
        }
        else
        {
            qDebug() << "SafeLogging: No Glasses detected!";
        }
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
            RTLSDisplayApplication::mainWindow()->viewSettingsWidget()->AddItemToComboBox(tid);
        }
        while(i<count)
        {
            if(_tagLogList.at(i).id == tid)
            {
                tag_reports_t edit = _tagLogList.at(i);
                edit.av_x = x;
                edit.av_y = y;
                edit.av_z = z;
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
        RTLSDisplayApplication::mainWindow()->viewSettingsWidget()->AddItemToComboBox(tid);
    }
}

void SafeLogging::ChangeSearchItem(int index)
{
    qDebug() << "SearchItemChanged to: " << index;
    SearchItemIndex = index;
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
         qDebug() << "SearchTagIndex: Found tag " << tid <<" at index: " << i;
         return i;
     }
     i++;
 }
 return 100;
 qDebug() << "SearchTagIndex: Tag " << tid <<" not found!";
}

void SafeLogging::LogData()
{
    qDebug() << "LogData: Gnothin to doe! :(";
}
