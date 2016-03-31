#include "ViewSettingsWidget.h"
#include "ui_ViewSettingsWidget.h"

#include "RTLSDisplayApplication.h"
#include "QPropertyModel.h"
#include "ViewSettings.h"
#include "OriginTool.h"
#include "ScaleTool.h"
#include "GraphicsView.h"
#include "GraphicsWidget.h"
#include "mainwindow.h"
#include "Sender.h"

#include <QFileDialog>
#include <QMessageBox>

ViewSettingsWidget::ViewSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ViewSettingsWidget)
{
    ui->setupUi(this);

    //ui->tabWidget->setCurrentIndex(0);
    //ui->tabWidget->removeTab(2);

    QObject::connect(ui->floorplanOpen_pb, SIGNAL(clicked()), this, SLOT(floorplanOpenClicked()));

    QObject::connect(ui->scaleX_pb, SIGNAL(clicked()), this, SLOT(scaleClicked()));
    QObject::connect(ui->scaleY_pb, SIGNAL(clicked()), this, SLOT(scaleClicked()));
    QObject::connect(ui->origin_pb, SIGNAL(clicked()), this, SLOT(originClicked()));

    QObject::connect(ui->gridShow, SIGNAL(clicked()), this, SLOT(gridShowClicked()));
    QObject::connect(ui->showOrigin, SIGNAL(clicked()), this, SLOT(originShowClicked()));
    QObject::connect(ui->showTagHistory, SIGNAL(clicked()), this, SLOT(tagHistoryShowClicked()));
    QObject::connect(ui->LogInterval, SIGNAL(clicked()), this, SLOT(LogIntervalClicked()));
    QObject::connect(ui->SendData, SIGNAL(clicked()), this, SLOT(SendDataClicked()));
    QObject::connect(ui->tagHistoryN, SIGNAL(valueChanged(int)), this, SLOT(tagHistoryNumberValueChanged(int)));
    QObject::connect(ui->SendataIntervalSB, SIGNAL(valueChanged(int)), this, SLOT(SendDataIntervalValue(int)));
    QObject::connect(ui->LogIntervalSB, SIGNAL(valueChanged(int)), this, SLOT(LogIntervalValueChanged(int)));

    QObject::connect(RTLSDisplayApplication::viewSettings(), SIGNAL(showGO(bool, bool)), this, SLOT(showOriginGrid(bool, bool)));
    QObject::connect(RTLSDisplayApplication::viewSettings(), SIGNAL(setFloorPlanPic()), this, SLOT(getFloorPlanPic()));

    RTLSDisplayApplication::connectReady(this, "onReady()");
}

void ViewSettingsWidget::onReady()
{
    QPropertyDataWidgetMapper *mapper = QPropertyModel::newMapper(RTLSDisplayApplication::viewSettings(), this);
    mapper->addMapping(ui->gridWidth_sb, "gridWidth");
    mapper->addMapping(ui->gridHeight_sb, "gridHeight");

    mapper->addMapping(ui->floorplanFlipX_cb, "floorplanFlipX", "checked");
    mapper->addMapping(ui->floorplanFlipY_cb, "floorplanFlipY", "checked");
    mapper->addMapping(ui->gridShow, "showGrid", "checked");
    mapper->addMapping(ui->showOrigin, "showOrigin", "checked");

    mapper->addMapping(ui->floorplanXOff_sb, "floorplanXOffset");
    mapper->addMapping(ui->floorplanYOff_sb, "floorplanYOffset");

    mapper->addMapping(ui->floorplanXScale_sb, "floorplanXScale");
    mapper->addMapping(ui->floorplanYScale_sb, "floorplanYScale");
    mapper->toFirst();

    QObject::connect(ui->floorplanFlipX_cb, SIGNAL(clicked()), mapper, SLOT(submit())); // Bug with QDataWidgetMapper (QTBUG-1818)
    QObject::connect(ui->floorplanFlipY_cb, SIGNAL(clicked()), mapper, SLOT(submit()));
    QObject::connect(ui->gridShow, SIGNAL(clicked()), mapper, SLOT(submit())); // Bug with QDataWidgetMapper (QTBUG-1818)
    QObject::connect(ui->showOrigin, SIGNAL(clicked()), mapper, SLOT(submit()));
    QObject::connect(ui->spinBox,SIGNAL(valueChanged(int)),RTLSDisplayApplication::safeLogging(),SLOT(Tollerance(int)));

    //by default the Geo-Fencing is OFF

    ui->showTagHistory->setChecked(true);

    ui->tabWidget->setCurrentIndex(0);
}

ViewSettingsWidget::~ViewSettingsWidget()
{
    delete ui;
}
void ViewSettingsWidget::SendDataClicked()
{
    RTLSDisplayApplication::mainWindow()->ToggleTimerSendData(ui->SendData->isChecked());
}

void ViewSettingsWidget::SendDataIntervalValue(int value)
{
    RTLSDisplayApplication::mainWindow()->SetSendDataInterval(value);
}

void ViewSettingsWidget::LogIntervalValueChanged(int value)
{
    //qDebug() << "LogIntervalValueChanged" << value;
    RTLSDisplayApplication::mainWindow()->SetLogInterval(value);
}

int ViewSettingsWidget::applyFloorPlanPic(const QString &path)
{
    QPixmap pm(path);

    if (pm.isNull())
    {
        //QMessageBox::critical(this, "Could not load floor plan", QString("Failed to load image : %1").arg(path));
        return -1;
    }

    ui->floorplanPath_lb->setText(QFileInfo(path).fileName());
    RTLSDisplayApplication::viewSettings()->setFloorplanPixmap(pm);

    return 0;
}

void ViewSettingsWidget::AddItemToComboBox(int value)
{
    QString temp = RTLSDisplayApplication::mainWindow()->graphicsWidget()->SearchListIndex(value);
    qDebug() << "Adding item to combobox: " << temp;
    ui->comboBox->addItem(temp);
}

void ViewSettingsWidget::TagLabelChanged(int index, QString label)
{
    qDebug() << "Label: " << index << " into: " << label;
    int count = ui->comboBox->count();
    if(!count)
    {
        qDebug() << "combobox empty";
        return;
    }
    //ui->comboBox->setItemText(index,label);
    ui->comboBox->setItemText(index,label);

}

void ViewSettingsWidget::getFloorPlanPic()
{
    applyFloorPlanPic(RTLSDisplayApplication::viewSettings()->getFloorplanPath());
}

void ViewSettingsWidget::floorplanOpenClicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open Bitmap", QString(), "Image (*.png *.jpg *.jpeg *.bmp)");
    if (path.isNull()) return;

    if(applyFloorPlanPic(path) == 0) //if OK set/save the path string
        RTLSDisplayApplication::viewSettings()->setFloorplanPath(path);
}

void ViewSettingsWidget::showOriginGrid(bool orig, bool grid)
{
    Q_UNUSED(orig)

    ui->gridShow->setChecked(grid);
    ui->showOrigin->setChecked(orig);
}

void ViewSettingsWidget::gridShowClicked()
{
    RTLSDisplayApplication::viewSettings()->setShowGrid(ui->gridShow->isChecked());
}

void ViewSettingsWidget::originShowClicked()
{
    RTLSDisplayApplication::viewSettings()->setShowOrigin(ui->showOrigin->isChecked());
}

void ViewSettingsWidget::tagHistoryNumberValueChanged(int value)
{
    RTLSDisplayApplication::graphicsWidget()->tagHistoryNumber(value);
}

void ViewSettingsWidget::zone1ValueChanged(double value)
{
    RTLSDisplayApplication::graphicsWidget()->zone1Value(value);
}

void ViewSettingsWidget::zone2ValueChanged(double value)
{
    RTLSDisplayApplication::graphicsWidget()->zone2Value(value);
}
void ViewSettingsWidget::setTagHistory(int h)
{
    ui->tagHistoryN->setValue(h);
}


void ViewSettingsWidget::tagHistoryShowClicked()
{
    RTLSDisplayApplication::graphicsWidget()->setShowTagHistory(ui->showTagHistory->isChecked());
}

void ViewSettingsWidget::LogIntervalClicked()
{
    RTLSDisplayApplication::mainWindow()->ToggleTimerLog(ui->LogInterval->isChecked());
}

void ViewSettingsWidget::originClicked()
{
    OriginTool *tool = new OriginTool(this);
    QObject::connect(tool, SIGNAL(done()), tool, SLOT(deleteLater()));
    RTLSDisplayApplication::graphicsView()->setTool(tool);
}

void ViewSettingsWidget::scaleClicked()
{
    ScaleTool *tool = NULL;

    if (QObject::sender() == ui->scaleX_pb)
        tool = new ScaleTool(ScaleTool::XAxis, this);
    else if (QObject::sender() == ui->scaleY_pb)
        tool = new ScaleTool(ScaleTool::YAxis, this);

    QObject::connect(tool, SIGNAL(done()), tool, SLOT(deleteLater()));
    RTLSDisplayApplication::graphicsView()->setTool(tool);
}


void ViewSettingsWidget::on_comboBox_currentIndexChanged(int index)
{
    RTLSDisplayApplication::safeLogging()->ChangeSearchItem(index);
}

void ViewSettingsWidget::on_comboBox_2_currentIndexChanged(int index)
{
    RTLSDisplayApplication::safeLogging()->ChangeLogMode(index);
}

void ViewSettingsWidget::on_spinBox_valueChanged(int arg1)
{
    RTLSDisplayApplication::safeLogging()->Tollerance(arg1);
}

void ViewSettingsWidget::on_pbAlarm_clicked()
{
    RTLSDisplayApplication::sender()->ChangeAlarmState();
}
