#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include "batterymonitor.h"

BatteryMonitor bm;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->LogButton->setStyleSheet("background-color:green;");
    timerId = startTimer(1000);

    lineseries = new QLineSeries();

    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(lineseries);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *layout = new QVBoxLayout(ui->lineChartViewFrame);
    layout->addWidget(chartView);

}


MainWindow::~MainWindow()
{
    killTimer(timerId);
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    bm.updateValues();
    bm.coloumbCounter();

    ui->VoltageLabel->setText(QString().asprintf("%0.1fv",bm.getVoltage()/1000.0));
    ui->PowerLabel->setText(QString().asprintf("%0.1fw",bm.getPowerRate()/1000.0));
    ui->CurrentLabel->setText(QString().asprintf("%0.1fa",bm.getCurrent()/1000.0));
    ui->VoltageProgressBar->setValue(bm.getVoltage());
    ui->PowerPositiveProgressBar->setValue(bm.getPowerRate());
    ui->PowerNegativeProgressBar->setValue(bm.getPowerRate() * -1);
    ui->CurrentPositiveProgressBar->setValue(bm.getCurrent());
    ui->CurrentNegativeProgressBar->setValue(bm.getCurrent() * -1);
    ui->BatteryPercentageProgressBar->setValue(bm.getCapacityInPerc());
    ui->BatteryPercentageLabel->setText(QString().asprintf("%3d%",bm.getCapacityInPerc()));
    ui->CapacityWhLabel->setText(QString().asprintf("%0.1f Wh",bm.getCapacityWh()));


    if(((bm.getMode() == "Discharging" || bm.getMode() == "Discharging\n") && (bm.getPowerRate() < 0)) && (!bm.getTimeLeft().isEmpty())){
        ui->TimeLeftLabel->setText("Time to empty");
        ui->TimeToFullOrEmptyLabel->setText(bm.getTimeLeft());
    }else if((bm.getMode() == "Charging" || bm.getMode() == "Charging\n") && (bm.getPowerRate() > 0) && (!bm.getTimeLeft().isEmpty())){
        ui->TimeLeftLabel->setText("Time to full");
        ui->TimeToFullOrEmptyLabel->setText(bm.getTimeLeft());
    }else{
        ui->TimeLeftLabel->setText("Calculating...");
        ui->TimeToFullOrEmptyLabel->setText("- - : - -");
    }

    lineseries->append(counter, bm.getVoltage());
    counter++;
    chartView->repaint();
    qDebug() << counter << bm.getVoltage();
}


void MainWindow::on_LogButton_clicked()
{
    QDate date = QDate::currentDate();
    QString fileName = ui->FileNameLineEdit->text() + "_" + date.toString("dd-MM-yyyy");
    QString fileExtension = ui->FileExtensionDropdown->currentText();
    bm.enableLogging(fileName, fileExtension);
    if(bm.getIsLoggerEnabled()){
        ui->LogButton->setText("Stop log");
        ui->LogButton->setStyleSheet("background-color:red;");
        ui->FileNameLineEdit->setDisabled(true);
        ui->FileExtensionDropdown->setDisabled(true);
    }else{
        ui->LogButton->setText("Start log");
        ui->LogButton->setStyleSheet("background-color:green;");
        ui->FileNameLineEdit->setEnabled(true);
        ui->FileExtensionDropdown->setEnabled(true);
    }
}


