#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int timerId;

private:
    Ui::MainWindow *ui;
    QLineSeries *lineseries;
    QChart *chart;
    QChartView *chartView;
    int counter = 0;


protected:
    void timerEvent(QTimerEvent *event);
private slots:
    void on_LogButton_clicked();
};

#endif // MAINWINDOW_H
