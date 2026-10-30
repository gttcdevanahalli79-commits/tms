#include "User.h"
#include <iomanip>
#include <sstream>
#include <array>
#include <cstring>

namespace tms {

// Simple, self-contained standard SHA-256 implementation for portable password hashing
namespace {
    inline uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }
    inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }
    inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }
    inline uint32_t sig0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }
    inline uint32_t sig1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }
    inline uint32_t theta0(uint32_t x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }
    inline uint32_t theta1(uint32_t x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    std::string sha256(const std::string& input) {
        uint32_t H[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        std::vector<uint8_t> data(input.begin(), input.end());
        uint64_t bit_len = static_cast<uint64_t>(data.size()) * 8;

        data.push_back(0x80);
        while ((data.size() % 64) != 56) {
            data.push_back(0x00);
        }

        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF));
        }

        for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
            uint32_t W[64];
            for (size_t t = 0; t < 16; ++t) {
                W[t] = (static_cast<uint32_t>(data[chunk + t * 4]) << 24) |
                       (static_cast<uint32_t>(data[chunk + t * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(data[chunk + t * 4 + 2]) << 8) |
                       (static_cast<uint32_t>(data[chunk + t * 4 + 3]));
            }
            for (size_t t = 16; t < 64; ++t) {
                W[t] = theta1(W[t - 2]) + W[t - 7] + theta0(W[t - 15]) + W[t - 16];
            }

            uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
            uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

            for (size_t t = 0; t < 64; ++t) {
                uint32_t T1 = h + sig1(e) + ch(e, f, g) + K[t] + W[t];
                uint32_t T2 = sig0(a) + maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;
            }

            H[0] += a; H[1] += b; H[2] += c; H[3] += d;
            H[4] += e; H[5] += f; H[6] += g; H[7] += h;
        }

        std::ostringstream oss;
        for (int i = 0; i < 8; ++i) {
            oss << std::hex << std::setfill('0') << std::setw(8) << H[i];
        }
        return oss.str();
    }
} // anonymous namespace

User::User(int id, std::string username, std::string passwordHash,
           std::string fullName, std::string email, std::string phone, UserRole role)
    : m_userId(id), m_username(std::move(username)), m_passwordHash(std::move(passwordHash)),
      m_fullName(std::move(fullName)), m_email(std::move(email)),
      m_phone(std::move(phone)), m_role(role) {}

std::string User::hashPassword(const std::string& rawPassword) {
    return sha256(rawPassword);
}

bool User::verifyPassword(const std::string& rawPassword) const {
    return m_passwordHash == hashPassword(rawPassword);
}

// ---------------- Admin ----------------
Admin::Admin(int id, std::string username, std::string passwordHash,
             std::string fullName, std::string email, std::string phone)
    : User(id, std::move(username), std::move(passwordHash),
           std::move(fullName), std::move(email), std::move(phone), UserRole::ADMIN) {}

void Admin::displayMenu() const {
    std::cout << "\n========================================\n"
              << "       ADMINISTRATOR CONTROL PANEL      \n"
              << "========================================\n"
              << "1. Manage Trains (Add/Update/Delete)\n"
              << "2. Manage Stations\n"
              << "3. Manage Routes and Stops\n"
              << "4. Manage Schedules\n"
              << "5. Manage Coaches and Seats\n"
              << "6. Manage Users & Operators\n"
              << "7. View All System Bookings\n"
              << "8. View Financial Revenue Reports\n"
              << "9. View Train Occupancy Reports\n"
              << "10. View Audit Logs\n"
              << "11. Logout\n"
              << "========================================\n";
}

void Admin::printProfile() const {
    std::cout << "[Admin Profile] ID: " << m_userId
              << " | Username: " << m_username
              << " | Name: " << m_fullName
              << " | Email: " << m_email
              << " | Role: Administrator\n";
}

bool Admin::canPerformAction(const std::string& /* action */) const {
    // Admin has full system authorization
    return true;
}

// ---------------- Operator ----------------
Operator::Operator(int id, std::string username, std::string passwordHash,
                   std::string fullName, std::string email, std::string phone,
                   std::string zone)
    : User(id, std::move(username), std::move(passwordHash),
           std::move(fullName), std::move(email), std::move(phone), UserRole::OPERATOR),
      m_assignedZone(std::move(zone)) {}

void Operator::displayMenu() const {
    std::cout << "\n========================================\n"
              << "       OPERATOR OPERATIONS CONSOLE      \n"
              << "========================================\n"
              << "1. View Today's Train Schedules\n"
              << "2. Update Train Running Status\n"
              << "3. Mark Train Delayed & Set Duration\n"
              << "4. View Passenger Manifest / Chart\n"
              << "5. View Real-time Seat Occupancy\n"
              << "6. Logout\n"
              << "========================================\n";
}

void Operator::printProfile() const {
    std::cout << "[Operator Profile] ID: " << m_userId
              << " | Username: " << m_username
              << " | Name: " << m_fullName
              << " | Zone: " << m_assignedZone
              << " | Role: Railway Operator\n";
}

bool Operator::canPerformAction(const std::string& action) const {
    if (action == "UPDATE_SCHEDULE" || action == "MARK_DELAY" ||
        action == "VIEW_MANIFEST" || action == "VIEW_OCCUPANCY") {
        return true;
    }
    return false;
}

// ---------------- Passenger ----------------
Passenger::Passenger(int id, std::string username, std::string passwordHash,
                     std::string fullName, std::string email, std::string phone,
                     std::string address, int points)
    : User(id, std::move(username), std::move(passwordHash),
           std::move(fullName), std::move(email), std::move(phone), UserRole::PASSENGER),
      m_address(std::move(address)), m_loyaltyPoints(points) {}

void Passenger::displayMenu() const {
    std::cout << "\n========================================\n"
              << "        PASSENGER SERVICE PORTAL        \n"
              << "========================================\n"
              << "1. Search Trains (Routes & Timetable)\n"
              << "2. Check Seat Availability & Fare\n"
              << "3. Book Train Tickets (Up to 6 Passengers)\n"
              << "4. View My Booking History\n"
              << "5. Cancel Ticket & Request Refund\n"
              << "6. Check 10-Digit PNR Status\n"
              << "7. View Profile & Loyalty Points\n"
              << "8. Logout\n"
              << "========================================\n";
}

void Passenger::printProfile() const {
    std::cout << "[Passenger Profile] ID: " << m_userId
              << " | Username: " << m_username
              << " | Name: " << m_fullName
              << " | Phone: " << m_phone
              << " | Loyalty Points: " << m_loyaltyPoints
              << " | Address: " << m_address << "\n";
}

bool Passenger::canPerformAction(const std::string& action) const {
    if (action == "BOOK_TICKET" || action == "CANCEL_TICKET" ||
        action == "VIEW_OWN_BOOKINGS" || action == "SEARCH_TRAIN") {
        return true;
    }
    return false;
}

} // namespace tms
