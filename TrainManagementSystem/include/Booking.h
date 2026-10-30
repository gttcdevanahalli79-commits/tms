#ifndef TMS_BOOKING_H
#define TMS_BOOKING_H

#include "Common.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace tms {

struct PassengerInfo {
    int passengerId;
    std::string name;
    int age;
    std::string gender;
    BerthType berthPreference;
    std::string allocatedCoachCode; // e.g. "B1"
    int allocatedSeatNumber;        // e.g. 24
    BerthType allocatedBerthType;   // e.g. LOWER
};

class Booking {
private:
    int m_bookingId;
    std::string m_pnr; // 10-digit unique PNR
    int m_userId;
    int m_trainId;
    std::string m_journeyDate; // "YYYY-MM-DD"
    int m_sourceStationId;
    int m_destStationId;
    CoachClass m_coachClass;
    std::vector<PassengerInfo> m_passengers;
    int64_t m_totalFarePaise;
    BookingStatus m_status;
    std::string m_bookingTimestamp;

public:
    Booking(int id, std::string pnr, int userId, int trainId,
            std::string journeyDate, int srcId, int dstId,
            CoachClass cClass, int64_t farePaise, BookingStatus status,
            std::string timestamp = "");

    int getBookingId() const { return m_bookingId; }
    const std::string& getPnr() const { return m_pnr; }
    int getUserId() const { return m_userId; }
    int getTrainId() const { return m_trainId; }
    const std::string& getJourneyDate() const { return m_journeyDate; }
    int getSourceStationId() const { return m_sourceStationId; }
    int getDestStationId() const { return m_destStationId; }
    CoachClass getCoachClass() const { return m_coachClass; }
    const std::vector<PassengerInfo>& getPassengers() const { return m_passengers; }
    int64_t getTotalFarePaise() const { return m_totalFarePaise; }
    BookingStatus getStatus() const { return m_status; }
    const std::string& getBookingTimestamp() const { return m_bookingTimestamp; }

    void addPassenger(const PassengerInfo& passenger);
    void setPassengers(const std::vector<PassengerInfo>& list) { m_passengers = list; }
    void setStatus(BookingStatus status) { m_status = status; }

    static std::string generatePNR();
    void printTicket() const;
};

} // namespace tms

#endif // TMS_BOOKING_H
