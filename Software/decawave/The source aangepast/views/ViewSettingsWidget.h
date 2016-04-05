// -------------------------------------------------------------------------------------------------------------------
//
//  File: ViewSettingsWidget.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef VIEWSETTINGSWIDGET_H
#define VIEWSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class ViewSettingsWidget;
}

class ViewSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewSettingsWidget(QWidget *parent = 0);
    ~ViewSettingsWidget();

    int applyFloorPlanPic(const QString &path);

protected slots:
    void onReady();

    void floorplanOpenClicked();


    void originClicked();
    void scaleClicked();

    void gridShowClicked();
    void originShowClicked();
    void tagHistoryShowClicked();
    void tagAncTableShowClicked();
    void showGeoFencingModeClicked();
    void showNavigationModeClicked();
    void alarmSetClicked();

    void zone1ValueChanged(double);
    void zone2ValueChanged(double);
    void zone1EditFinished(void);
    void zone2EditFinished(void);
    void tagHistoryNumberValueChanged(int);

    void showOriginGrid(bool orig, bool grid);
    void getFloorPlanPic(void);

    void setTagHistory(int h);

private:
    Ui::ViewSettingsWidget *ui;
};

#endif // VIEWSETTINGSWIDGET_H
