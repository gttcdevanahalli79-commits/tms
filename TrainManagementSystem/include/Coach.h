#ifndef TMS_COACH_H
#define TMS_COACH_H

#include "Common.h"
#include "Seat.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace tms {

class Coach {
private:
    int m_coachId;
    int m_trainId;
    std::string m_coachCode; // e.g., "S1", "B1", "A1", "C1"
    CoachClass m_coachClass;
    int m_totalSeats;
    std::vector<Seat> m_seats;

public:
    Coach(int coachId, int trainId, std::string code, CoachClass cClass, int totalSeats = 72);

    int getCoachId() const { return m_coachId; }
    int getTrainId() const { return m_trainId; }
    const std::string& getCoachCode() const { return m_coachCode; }
    CoachClass getCoachClass() const { return m_coachClass; }
    int getTotalSeats() const { return m_totalSeats; }
    const std::vector<Seat>& getSeats() const { return m_seats; }

    void addSeat(const Seat& seat) { m_seats.push_back(seat); }
    void generateDefaultSeats();

    void print() const;
};

} // namespace tms

#endif // TMS_COACH_H
