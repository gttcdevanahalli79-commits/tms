#ifndef TMS_COMMON_H
#define TMS_COMMON_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <cmath>

namespace tms {

// System Version Constants
constexpr const char* APP_NAME = "Train Management System (TMS)";
constexpr const char* APP_VERSION = "1.0.0";
constexpr int MAX_PASSENGERS_PER_BOOKING = 6;
constexpr double AC_GST_RATE = 0.05; // 5% GST for AC classes

// User Roles
enum class UserRole {
    ADMIN,
    OPERATOR,
    PASSENGER
};

inline std::string userRoleToString(UserRole role) {
    switch (role) {
        case UserRole::ADMIN: return "ADMIN";
        case UserRole::OPERATOR: return "OPERATOR";
        case UserRole::PASSENGER: return "PASSENGER";
        default: return "UNKNOWN";
    }
}

inline UserRole stringToUserRole(const std::string& str) {
    if (str == "ADMIN") return UserRole::ADMIN;
    if (str == "OPERATOR") return UserRole::OPERATOR;
    return UserRole::PASSENGER;
}

// Train Categories
enum class TrainType {
    LOCAL,
    EXPRESS,
    SUPERFAST,
    SLEEPER,
    RAJDHANI,
    VANDE_BHARAT
};

inline std::string trainTypeToString(TrainType type) {
    switch (type) {
        case TrainType::LOCAL: return "LOCAL";
        case TrainType::EXPRESS: return "EXPRESS";
        case TrainType::SUPERFAST: return "SUPERFAST";
        case TrainType::SLEEPER: return "SLEEPER";
        case TrainType::RAJDHANI: return "RAJDHANI";
        case TrainType::VANDE_BHARAT: return "VANDE_BHARAT";
        default: return "EXPRESS";
    }
}

inline TrainType stringToTrainType(const std::string& str) {
    if (str == "LOCAL") return TrainType::LOCAL;
    if (str == "SUPERFAST") return TrainType::SUPERFAST;
    if (str == "SLEEPER") return TrainType::SLEEPER;
    if (str == "RAJDHANI") return TrainType::RAJDHANI;
    if (str == "VANDE_BHARAT") return TrainType::VANDE_BHARAT;
    return TrainType::EXPRESS;
}

inline double getTrainTypeMultiplier(TrainType type) {
    switch (type) {
        case TrainType::LOCAL: return 0.90;
        case TrainType::EXPRESS: return 1.00;
        case TrainType::SUPERFAST: return 1.15;
        case TrainType::SLEEPER: return 1.00;
        case TrainType::RAJDHANI: return 1.80;
        case TrainType::VANDE_BHARAT: return 1.70;
        default: return 1.00;
    }
}

// Coach Classes
enum class CoachClass {
    SL,  // Sleeper
    _3A, // AC 3 Tier
    _2A, // AC 2 Tier
    _1A, // AC First Class
    CC,  // AC Chair Car
    EC   // Executive Chair Car
};

inline std::string coachClassToString(CoachClass cls) {
    switch (cls) {
        case CoachClass::SL: return "SL";
        case CoachClass::_3A: return "3A";
        case CoachClass::_2A: return "2A";
        case CoachClass::_1A: return "1A";
        case CoachClass::CC: return "CC";
        case CoachClass::EC: return "EC";
        default: return "SL";
    }
}

inline CoachClass stringToCoachClass(const std::string& str) {
    if (str == "3A") return CoachClass::_3A;
    if (str == "2A") return CoachClass::_2A;
    if (str == "1A") return CoachClass::_1A;
    if (str == "CC") return CoachClass::CC;
    if (str == "EC") return CoachClass::EC;
    return CoachClass::SL;
}

inline double getCoachClassMultiplier(CoachClass cls) {
    switch (cls) {
        case CoachClass::SL: return 1.0;
        case CoachClass::_3A: return 1.6;
        case CoachClass::_2A: return 2.3;
        case CoachClass::_1A: return 3.2;
        case CoachClass::CC: return 1.3;
        case CoachClass::EC: return 2.6;
        default: return 1.0;
    }
}

inline bool isAirConditioned(CoachClass cls) {
    return cls != CoachClass::SL;
}

// Berth Types
enum class BerthType {
    LOWER,
    MIDDLE,
    UPPER,
    SIDE_LOWER,
    SIDE_UPPER,
    WINDOW_SEAT,
    AISLE_SEAT
};

inline std::string berthTypeToString(BerthType berth) {
    switch (berth) {
        case BerthType::LOWER: return "Lower";
        case BerthType::MIDDLE: return "Middle";
        case BerthType::UPPER: return "Upper";
        case BerthType::SIDE_LOWER: return "Side Lower";
        case BerthType::SIDE_UPPER: return "Side Upper";
        case BerthType::WINDOW_SEAT: return "Window";
        case BerthType::AISLE_SEAT: return "Aisle";
        default: return "General";
    }
}

inline BerthType stringToBerthType(const std::string& str) {
    if (str == "Middle") return BerthType::MIDDLE;
    if (str == "Upper") return BerthType::UPPER;
    if (str == "Side Lower") return BerthType::SIDE_LOWER;
    if (str == "Side Upper") return BerthType::SIDE_UPPER;
    if (str == "Window") return BerthType::WINDOW_SEAT;
    if (str == "Aisle") return BerthType::AISLE_SEAT;
    return BerthType::LOWER;
}

// Booking Status
enum class BookingStatus {
    CONFIRMED,
    WAITLISTED,
    CANCELLED
};

inline std::string bookingStatusToString(BookingStatus status) {
    switch (status) {
        case BookingStatus::CONFIRMED: return "CONFIRMED";
        case BookingStatus::WAITLISTED: return "WAITLISTED";
        case BookingStatus::CANCELLED: return "CANCELLED";
        default: return "PENDING";
    }
}

// Payment Methods & Status
enum class PaymentMethod {
    UPI,
    CARD,
    CASH
};

inline std::string paymentMethodToString(PaymentMethod method) {
    switch (method) {
        case PaymentMethod::UPI: return "UPI";
        case PaymentMethod::CARD: return "CARD";
        case PaymentMethod::CASH: return "CASH";
        default: return "UPI";
    }
}

enum class PaymentStatus {
    SUCCESS,
    FAILED,
    REFUNDED
};

inline std::string paymentStatusToString(PaymentStatus status) {
    switch (status) {
        case PaymentStatus::SUCCESS: return "SUCCESS";
        case PaymentStatus::FAILED: return "FAILED";
        case PaymentStatus::REFUNDED: return "REFUNDED";
        default: return "PENDING";
    }
}

// Fare calculation helper: Distance * BaseFare * ClassMultiplier * TrainMultiplier
// Stored in integer paise (1 Rupee = 100 Paise)
inline int64_t calculateFareInPaise(double distanceKm, double baseFarePerKm, CoachClass cClass, TrainType tType) {
    double rawFareRupees = distanceKm * baseFarePerKm * getCoachClassMultiplier(cClass) * getTrainTypeMultiplier(tType);
    
    // Add 5% GST for AC coaches
    if (isAirConditioned(cClass)) {
        rawFareRupees += rawFareRupees * AC_GST_RATE;
    }
    
    // Standard reservation fee of Rs. 40 (4000 paise)
    constexpr double reservationFee = 40.0;
    rawFareRupees += reservationFee;

    // Round to nearest Rupee
    int64_t roundedRupees = static_cast<int64_t>(std::round(rawFareRupees));
    return roundedRupees * 100; // convert to paise
}

inline double paiseToRupees(int64_t paise) {
    return static_cast<double>(paise) / 100.0;
}

} // namespace tms

#endif // TMS_COMMON_H
