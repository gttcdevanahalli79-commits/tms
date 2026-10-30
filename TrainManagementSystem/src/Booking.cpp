#include "Booking.h"
#include <random>
#include <iomanip>

namespace tms {

Booking::Booking(int id, std::string pnr, int userId, int trainId,
                 std::string journeyDate, int srcId, int dstId,
                 CoachClass cClass, int64_t farePaise, BookingStatus status,
                 std::string timestamp)
    : m_bookingId(id), m_pnr(std::move(pnr)), m_userId(userId), m_trainId(trainId),
      m_journeyDate(std::move(journeyDate)), m_sourceStationId(srcId),
      m_destStationId(dstId), m_coachClass(cClass), m_totalFarePaise(farePaise),
      m_status(status), m_bookingTimestamp(std::move(timestamp)) {}

void Booking::addPassenger(const PassengerInfo& passenger) {
    m_passengers.push_back(passenger);
}

std::string Booking::generatePNR() {
    // Generate a 10-digit PNR starting with a zone digit (e.g. 2, 4, 6, 8)
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(1000000000ULL, 9999999999ULL);
    return std::to_string(dist(gen));
}

void Booking::printTicket() const {
    std::cout << "\n=======================================================\n"
              << "            ELECTRONIC RESERVATION SLIP (ERS)           \n"
              << "=======================================================\n"
              << "PNR NUMBER:       " << m_pnr << "\n"
              << "TRAIN ID:         " << m_trainId << "\n"
              << "DATE OF JOURNEY:  " << m_journeyDate << "\n"
              << "CLASS:            " << coachClassToString(m_coachClass) << "\n"
              << "STATUS:           " << bookingStatusToString(m_status) << "\n"
              << "TOTAL FARE:       Rs. " << std::fixed << std::setprecision(2) << paiseToRupees(m_totalFarePaise) << "\n"
              << "BOOKING TIME:     " << m_bookingTimestamp << "\n"
              << "-------------------------------------------------------\n"
              << "PASSENGER DETAILS (" << m_passengers.size() << "):\n";
    for (size_t i = 0; i < m_passengers.size(); ++i) {
        const auto& p = m_passengers[i];
        std::cout << "  " << (i + 1) << ". " << p.name
                  << " | Age: " << p.age << " | " << p.gender
                  << " | Seat: " << p.allocatedCoachCode << "-" << p.allocatedSeatNumber
                  << " (" << berthTypeToString(p.allocatedBerthType) << ")\n";
    }
    std::cout << "=======================================================\n";
}

} // namespace tms
