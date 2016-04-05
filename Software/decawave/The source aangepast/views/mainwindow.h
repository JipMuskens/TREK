// -------------------------------------------------------------------------------------------------------------------
//
//  File: MainWindow.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QLabel>
#include "serialconnection.h"

namespace Ui {
class MainWindow;
}

class SerialConnection;
class GraphicsWidget;
class ConnectionWidget;
class ViewSettingsWidget;
/**
 * The MainWindow class is the RTLS Controller Main Window widget.
 *
 * It is responsible for setting up all the dock widgets inside it, as weel as the central widget.
 * It also handles global shortcuts (Select All) and the Menu bar.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    GraphicsWidget *graphicsWidget();

    ViewSettingsWidget *viewSettingsWidget();

    void saveConfigFile(QString filename, QString cfg);
    void loadConfigFile(QString filename);

public slots:
    void connectionStateChanged(SerialConnection::ConnectionState);

protected slots:
    void onReady();

    void loadSettings();
    void saveSettings();

    void onMiniMapView();

    void onAnchorConfigAction();

    void statusBarMessage(QString status);

private:
    Ui::MainWindow *const ui;
    QLabel *_infoLabel;

    QAction *_anchorConfigAction;
    ConnectionWidget *_cWidget;

    bool _showMainToolBar;
    bool _notConnected;
};

#endif // MAINWINDOW_H
