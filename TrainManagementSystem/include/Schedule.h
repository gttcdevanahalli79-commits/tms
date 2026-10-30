#ifndef TMS_SCHEDULE_H
#define TMS_SCHEDULE_H

#include <string>
#include <iostream>

namespace tms {

class Schedule {
private:
    int m_scheduleId;
    int m_trainId;
    std::string m_departureTime; // Format "HH:MM"
    std::string m_arrivalTime;   // Format "HH:MM"
    std::string m_operatingDays; // e.g., "Daily", "Mon,Wed,Fri"
    int m_delayMinutes;
    bool m_isOvernight;

public:
    Schedule(int id, int trainId, std::string depTime, std::string arrTime,
             std::string days = "Daily", int delay = 0, bool overnight = false);

    int getScheduleId() const { return m_scheduleId; }
    int getTrainId() const { return m_trainId; }
    const std::string& getDepartureTime() const { return m_departureTime; }
    const std::string& getArrivalTime() const { return m_arrivalTime; }
    const std::string& getOperatingDays() const { return m_operatingDays; }
    int getDelayMinutes() const { return m_delayMinutes; }
    bool isOvernight() const { return m_isOvernight; }

    void setDepartureTime(const std::string& time) { m_departureTime = time; }
    void setArrivalTime(const std::string& time) { m_arrivalTime = time; }
    void setOperatingDays(const std::string& days) { m_operatingDays = days; }
    void setDelayMinutes(int delay) { m_delayMinutes = delay; }
    void setOvernight(bool overnight) { m_isOvernight = overnight; }

    bool runsOnDay(const std::string& dayOfWeek) const;
    void print() const;
};

} // namespace tms

#endif // TMS_SCHEDULE_H
