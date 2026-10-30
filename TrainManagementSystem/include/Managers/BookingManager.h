#ifndef TMS_BOOKING_MANAGER_H
#define TMS_BOOKING_MANAGER_H

#include "Booking.h"
#include <vector>
#include <memory>

namespace tms {

class BookingManager {
public:
    BookingManager();

    std::shared_ptr<Booking> bookTicket(int userId, int trainId, const std::string& journeyDate,
                                       int srcStationId, int dstStationId,
                                       CoachClass cClass, const std::vector<PassengerInfo>& passengers);

    std::shared_ptr<Booking> getBookingByPNR(const std::string& pnr);
    std::vector<Booking> getBookingsByUserId(int userId);
    std::vector<Booking> getAllBookings();
    bool cancelBooking(const std::string& pnr, double hoursBeforeDeparture, int64_t& outRefundPaise);

    int getAvailableSeatCount(int trainId, const std::string& journeyDate, CoachClass cClass);
};

} // namespace tms

#endif // TMS_BOOKING_MANAGER_H
