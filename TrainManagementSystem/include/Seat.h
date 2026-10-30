#ifndef TMS_SEAT_H
#define TMS_SEAT_H

#include "Common.h"
#include <string>
#include <iostream>

namespace tms {

class Seat {
private:
    int m_seatId;
    int m_coachId;
    int m_seatNumber;
    BerthType m_berthType;

public:
    Seat(int seatId, int coachId, int seatNum, BerthType berthType);

    int getSeatId() const { return m_seatId; }
    int getCoachId() const { return m_coachId; }
    int getSeatNumber() const { return m_seatNumber; }
    BerthType getBerthType() const { return m_berthType; }

    static BerthType deduceBerthType(CoachClass cls, int seatNumber);
    void print() const;
};

} // namespace tms

#endif // TMS_SEAT_H
