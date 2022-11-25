#include "batterymonitor.h"

BatteryMonitor::BatteryMonitor()
{
    loadCapacityCurve();
}

uint8_t  BatteryMonitor::getCapacityInPerc(){
    return capacity_in_perc;
}
uint16_t BatteryMonitor::getVoltage(){
    return voltage;
}
int32_t  BatteryMonitor::getPowerRate(){
    return power_rate;
}
int16_t  BatteryMonitor::getCurrent(){
    return current;
}
double BatteryMonitor::getCapacityWh(){
    return capacity_mWh / 1000.0;
}
double BatteryMonitor::getCapacityAh(){
    return capacity_mAh / 1000.0;
}
QString BatteryMonitor::getTimeLeft(){
    return time_left;
}
QString BatteryMonitor::getFileName(){
    return file_name;
}
void BatteryMonitor::setFileName(QString fileName){
    file_name = fileName;
}
void BatteryMonitor::setFileExtension(QString fileExtension){
    file_extension = fileExtension;
}
bool BatteryMonitor::getIsLoggerEnabled(){
    return logging_enabled;
}
QString BatteryMonitor::getMode(){
    return mode;
}
void BatteryMonitor::enableLogging(QString fileName, QString fileExtension){
    if(!fileName.isEmpty()){
        setFileName(fileName);
        setFileExtension(fileExtension);
        logging_enabled = !logging_enabled;
    }else{
        setFileName("");
        logging_enabled = false;
    }
}
void BatteryMonitor::updateValues(){
    QString tmp;

    tmp = readFile("/sys/class/power_supply/BAT0/voltage_now");
    if(!(tmp == "error")){
        voltage = tmp.toInt() / 1000;
    }

    tmp = readFile("/sys/class/power_supply/BAT0/status");
    if(!(tmp == "error")){
        mode = tmp;
    }

    tmp = readFile("/sys/class/power_supply/BAT0/power_now");
    if(!(tmp == "error")){
        if(mode == "Charging" || mode == "Charging\n"){
            power_rate = tmp.toInt() / 1000;
        }else if(mode == "Discharging" || mode == "Discharging\n"){
            power_rate = (tmp.toInt() / 1000) * -1;
        }else{
            power_rate = 0;
        }

        calculateCurrent();
    }

    calculateAvgVoltage();
    calculateAvgPowerRate();
    determineCapacity();
    calculateCapacityInPerc();
    calculateTimeLeft();
    dataLogger();
    detectModeChange();
}

void BatteryMonitor::coloumbCounter(){
    const double SECONDS_IN_HOUR = 3600.0;
    capacity_mAh += current / SECONDS_IN_HOUR;
    capacity_mWh += power_rate / SECONDS_IN_HOUR;

    if(mode == "Charging" || mode == "Charging\n"){
        double power_loss = 0.06; // 6%
        capacity_mAh -= (current / SECONDS_IN_HOUR) * power_loss;
        capacity_mWh -= (power_rate / SECONDS_IN_HOUR) * power_loss;
    }

    if(capacity_mAh > DESIGNED_CAPACITY_MAH) capacity_mAh = DESIGNED_CAPACITY_MAH;
    if(capacity_mWh > DESIGNED_CAPACITY_MWH) capacity_mWh = DESIGNED_CAPACITY_MWH;

    if(capacity_mAh < 0) capacity_mAh = 0;
    if(capacity_mWh < 0) capacity_mWh = 0;
}

// Private methods
QString BatteryMonitor::readFile(QString path){
    QFile file(path);
    if(file.exists()){
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
          QString output = file.readAll();
          file.close();
          return output;
        }
    }
    return "error";
}

void BatteryMonitor::calculateCurrent(){
    if(power_rate != 0){
        current = ((double)power_rate / voltage) * 1000;
    }else{
        current = 0;
    }
}

void BatteryMonitor::loadCapacityCurve(){
    QFile file("capacity_curve.csv");
    if(file.exists()){
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            while(!stream.atEnd()){
                QStringList data = stream.readLine().split(",");
                int volt = data[0].toInt();
                int cap_mAh = data[1].toInt();
                int cap_mWh = data[2].toInt();
                QVector<int> d;
                d.push_back(volt);
                d.push_back(cap_mAh);
                d.push_back(cap_mWh);
                capacityCurve.push_back(d);
            }
        }
    }
}

void BatteryMonitor::determineCapacity(){
    if(firstLoad && voltage_avg_ready){
        for( QVector<int> item: capacityCurve){
            if(voltage_avg > item[0]){
                capacity_mAh = item[1];
                capacity_mWh = item[2];
                break;
            }
        }
        firstLoad = false;
    }
}

void BatteryMonitor::calculateAvgVoltage(){
    if(voltage_avg_counter < voltage_avg_range){
        voltage_avg_list.push_back(voltage);
    }else{
        voltage_avg_list.pop_front();
        voltage_avg_list.push_back(voltage);
        int tmp = 0;
        for(auto i: voltage_avg_list){
            tmp += i;
        }
        voltage_avg = tmp / voltage_avg_range;
        voltage_avg_ready = true;
    }

    if(!voltage_avg_ready){
        voltage_avg_counter++;
    }
}

void BatteryMonitor::calculateAvgPowerRate(){
    if(mode_changed){
        power_avg_list.clear();
        power_avg_counter = 0;
        mode_changed = false;
    }

    if(power_avg_counter < power_avg_range){
        power_avg_list.push_back(power_rate);
        power_avg_counter++;
    }else{
        power_avg_list.pop_front();
        power_avg_list.push_back(power_rate);
    }
    int tmp = 0;
    for(auto i: power_avg_list){
        tmp += i;
    }

    power_rate_avg = tmp / power_avg_counter;
}

void BatteryMonitor::calculateCapacityInPerc(){
    capacity_in_perc = (capacity_mWh * 100) / DESIGNED_CAPACITY_MWH;
}

void BatteryMonitor::calculateTimeLeft(){
    int seconds = 0;
    if(mode == "Discharging" || mode == "Discharging\n"){
        seconds = (int)((capacity_mWh / abs(power_rate_avg))*60*60);

    }else if(mode == "Charging" || mode == "Charging\n"){
        seconds = (int)(((DESIGNED_CAPACITY_MWH - capacity_mWh) / abs(power_rate_avg))*60*60);
    }
    if(seconds > 0){
        time_left = QDateTime::fromTime_t(seconds).toUTC().toString("hh:mm");
    }else{
        time_left = "";
    }


}

void BatteryMonitor::dataLogger(){
    if(!file_name.isEmpty() && logging_enabled){
        QDate date = QDate::currentDate();
        QTime time = QTime::currentTime();

        QDir dir(folder_name);
        if(!dir.exists()){
            dir.mkpath(".");
        }

        QFile file(folder_name + file_name + file_extension);
        if(!file.exists()){
            if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
                file.write("Time\tDate\tVoltage\tPower\tCurrent\tCapacity_mWh\tCapacity_mAh\tCapacity_percentage\tTime_Left\n");
                file.close();
            }
        }
        if(file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream stream(&file);
            stream << time.toString() << "\t" << date.toString("dd-MM-yyyy") << "\t" << voltage << "\t" << power_rate << "\t" << current << "\t" << (int)capacity_mWh << "\t" << (int)capacity_mAh << "\t" << capacity_in_perc << "\t" << time_left << "\n";
            file.close();
        }
    }
}

void BatteryMonitor::detectModeChange(){
    if((mode == "Discharging" || mode == "Discharging\n") && mode_changed_flag){
        mode_changed_flag = false;
        mode_changed = true;
    }
    if((mode == "Charging" || mode == "Charging\n") && !mode_changed_flag){
        mode_changed_flag = true;
        mode_changed = true;
    }
}
