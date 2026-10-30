#include "Train.h"
#include "Coach.h"
#include "Seat.h"
#include "Route.h"
#include "Schedule.h"
#include <iomanip>

namespace tms {

// ---------------- Train ----------------
Train::Train(int id, std::string number, std::string name, TrainType type,
             int srcId, int dstId, int routeId, double baseFare, bool active)
    : m_trainId(id), m_trainNumber(std::move(number)), m_trainName(std::move(name)),
      m_type(type), m_sourceStationId(srcId), m_destStationId(dstId),
      m_routeId(routeId), m_baseFarePerKm(baseFare), m_isActive(active) {}

void Train::print() const {
    std::cout << "#" << m_trainNumber << " " << m_trainName
              << " [" << trainTypeToString(m_type) << "]"
              << " | Base Fare: Rs." << std::fixed << std::setprecision(2) << m_baseFarePerKm << "/km"
              << " | Status: " << (m_isActive ? "ACTIVE" : "INACTIVE") << "\n";
}

// ---------------- Seat ----------------
Seat::Seat(int seatId, int coachId, int seatNum, BerthType berthType)
    : m_seatId(seatId), m_coachId(coachId), m_seatNumber(seatNum), m_berthType(berthType) {}

BerthType Seat::deduceBerthType(CoachClass cls, int seatNumber) {
    if (cls == CoachClass::CC || cls == CoachClass::EC) {
        int mod = seatNumber % 5;
        if (mod == 1 || mod == 0) return BerthType::WINDOW_SEAT;
        if (mod == 2 || mod == 4) return BerthType::AISLE_SEAT;
        return BerthType::MIDDLE;
    }
    if (cls == CoachClass::_2A) {
        int mod = seatNumber % 6;
        if (mod == 1 || mod == 3) return BerthType::LOWER;
        if (mod == 2 || mod == 4) return BerthType::UPPER;
        if (mod == 5) return BerthType::SIDE_LOWER;
        return BerthType::SIDE_UPPER;
    }
    // Default SL and 3A (8 seats per bay)
    int mod = seatNumber % 8;
    if (mod == 1 || mod == 4) return BerthType::LOWER;
    if (mod == 2 || mod == 5) return BerthType::MIDDLE;
    if (mod == 3 || mod == 6) return BerthType::UPPER;
    if (mod == 7) return BerthType::SIDE_LOWER;
    return BerthType::SIDE_UPPER;
}

void Seat::print() const {
    std::cout << "Seat #" << m_seatNumber << " (" << berthTypeToString(m_berthType) << ")";
}

// ---------------- Coach ----------------
Coach::Coach(int coachId, int trainId, std::string code, CoachClass cClass, int totalSeats)
    : m_coachId(coachId), m_trainId(trainId), m_coachCode(std::move(code)),
      m_coachClass(cClass), m_totalSeats(totalSeats) {}

void Coach::generateDefaultSeats() {
    m_seats.clear();
    m_seats.reserve(static_cast<size_t>(m_totalSeats));
    for (int i = 1; i <= m_totalSeats; ++i) {
        BerthType bType = Seat::deduceBerthType(m_coachClass, i);
        m_seats.emplace_back(0, m_coachId, i, bType);
    }
}

void Coach::print() const {
    std::cout << "Coach " << m_coachCode << " [" << coachClassToString(m_coachClass)
              << "] Total Seats: " << m_totalSeats << "\n";
}

// ---------------- Route ----------------
Route::Route(int id, std::string name, int srcId, int dstId, double totalDist)
    : m_routeId(id), m_routeName(std::move(name)), m_sourceStationId(srcId),
      m_destStationId(dstId), m_totalDistanceKm(totalDist) {}

void Route::addStop(const RouteStop& stop) {
    m_stops.push_back(stop);
}

void Route::setStops(const std::vector<RouteStop>& stops) {
    m_stops = stops;
}

double Route::getDistanceBetweenStations(int startStationId, int endStationId) const {
    double startDist = -1.0;
    double endDist = -1.0;
    for (const auto& stop : m_stops) {
        if (stop.stationId == startStationId) startDist = stop.distanceFromStartKm;
        if (stop.stationId == endStationId) endDist = stop.distanceFromStartKm;
    }
    if (startDist >= 0.0 && endDist >= 0.0 && endDist >= startDist) {
        return endDist - startDist;
    }
    return m_totalDistanceKm;
}

bool Route::containsSegment(int fromStationId, int toStationId) const {
    int fromSeq = -1;
    int toSeq = -1;
    for (const auto& stop : m_stops) {
        if (stop.stationId == fromStationId) fromSeq = stop.stopOrder;
        if (stop.stationId == toStationId) toSeq = stop.stopOrder;
    }
    return (fromSeq != -1 && toSeq != -1 && toSeq > fromSeq);
}

void Route::print() const {
    std::cout << "Route: " << m_routeName << " (" << m_totalDistanceKm << " km)\n";
    for (const auto& stop : m_stops) {
        std::cout << "  [" << stop.stopOrder << "] " << stop.stationCode
                  << " - " << stop.stationName
                  << " (" << stop.distanceFromStartKm << " km, Halt: "
                  << stop.haltMinutes << " mins)\n";
    }
}

// ---------------- Schedule ----------------
Schedule::Schedule(int id, int trainId, std::string depTime, std::string arrTime,
                   std::string days, int delay, bool overnight)
    : m_scheduleId(id), m_trainId(trainId), m_departureTime(std::move(depTime)),
      m_arrivalTime(std::move(arrTime)), m_operatingDays(std::move(days)),
      m_delayMinutes(delay), m_isOvernight(overnight) {}

bool Schedule::runsOnDay(const std::string& dayOfWeek) const {
    if (m_operatingDays == "Daily") return true;
    return m_operatingDays.find(dayOfWeek) != std::string::npos;
}

void Schedule::print() const {
    std::cout << "Dep: " << m_departureTime << " | Arr: " << m_arrivalTime
              << " | Days: " << m_operatingDays
              << " | Delay: " << m_delayMinutes << " mins"
              << (m_isOvernight ? " (Overnight)" : "") << "\n";
}

} // namespace tms
