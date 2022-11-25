#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <inttypes.h>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QDir>

class BatteryMonitor
{
public:
    BatteryMonitor();
    uint8_t getCapacityInPerc();
    uint16_t getVoltage();
    int32_t getPowerRate();
    int16_t getCurrent();
    double getCapacityWh();
    double getCapacityAh();
    QString getTimeLeft();
    void updateValues();
    void coloumbCounter();
    QString getFileName();
    void setFileName(QString fileName);
    void setFileExtension(QString fileExtension);
    bool getIsLoggerEnabled();
    void enableLogging(QString fileName, QString fileExtension);
    QString getMode();

private:
    const uint16_t DESIGNED_CAPACITY_MAH = 10000;
    const uint32_t DESIGNED_CAPACITY_MWH = 110000;
    const uint8_t voltage_avg_range = 20;
    const uint8_t power_avg_range = 60;

    bool firstLoad = true;
    bool voltage_avg_ready = false;
    bool logging_enabled = false;
    bool mode_changed = false;
    bool mode_changed_flag = false;

    QString mode;
    uint8_t capacity_in_perc = 0;
    uint16_t voltage = 0;
    uint16_t voltage_avg = 0;
    int32_t power_rate = 0;
    int32_t power_rate_avg = 0;
    int16_t current = 0;
    QString time_left;
    double capacity_mAh = 0.0;
    double capacity_mWh = 0.0;
    QVector<QVector <int>> capacityCurve;
    QVector<int> voltage_avg_list;
    QVector<int> power_avg_list;
    uint8_t voltage_avg_counter = 0;
    uint8_t power_avg_counter = 0;
    QString file_name;
    QString file_extension;
    const QString folder_name = "battery_monitor_logs/";

    QString readFile(QString path);
    void calculateCurrent();
    void loadCapacityCurve();
    void determineCapacity();
    void calculateAvgVoltage();
    void calculateAvgPowerRate();
    void calculateCapacityInPerc();
    void calculateTimeLeft();
    void dataLogger();
    void detectModeChange();
};

#endif // BATTERYMONITOR_H
