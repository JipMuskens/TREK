#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QLabel>

#include "serialconnection.h"

namespace Ui {
class MainWindow;
}
class QTimer;
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

    /**
     * createPopupMenu adds the windows actions to \a menu.
     * The are the actions to hide or show dock widgets.
     * @param menu the QMenu instance to which the actions should be added
     * @return the menu object
     */
    void saveConfigFile(QString filename, QString cfg);
    void loadConfigFile(QString filename);
    void SetSendDataInterval(int value);
    void SetLogInterval(int value);

public slots:
    void connectionStateChanged(SerialConnection::ConnectionState);
    void ShowSettings(void);
    void ToggleTimerLog(bool);
    void ToggleTimerSendData(bool);

protected slots:
    void onReady();
    void LogTimeout();
    void SendDataTimeout();
    void loadSettings();
    void saveSettings();

    void onMiniMapView();

    void onAnchorConfigAction();

    void statusBarMessage(QString status);



private:
    Ui::MainWindow *const ui;
    QLabel *_infoLabel;
    QTimer *timerLog;
    QTimer *timerSendData;
    QAction *_anchorConfigAction;
    ConnectionWidget *_cWidget;

    int SendDataInterval;
    int LogInterval;

    bool _showMainToolBar;
    bool _notConnected;
};

#endif // MAINWINDOW_H
