#ifndef TMS_RESERVATION_SYSTEM_H
#define TMS_RESERVATION_SYSTEM_H

#include "AuthenticationManager.h"
#include "TrainManager.h"
#include "BookingManager.h"
#include "PaymentManager.h"
#include "ReportManager.h"
#include <memory>

namespace tms {

class ReservationSystem {
private:
    std::unique_ptr<AuthenticationManager> m_authMgr;
    std::unique_ptr<TrainManager> m_trainMgr;
    std::unique_ptr<BookingManager> m_bookingMgr;
    std::unique_ptr<PaymentManager> m_paymentMgr;
    std::unique_ptr<ReportManager> m_reportMgr;
    bool m_isRunning;

public:
    ReservationSystem();
    ~ReservationSystem();

    void initialize(const std::string& dbPath = "tms.db");
    void run();

    // Menu handlers
    void showMainMenu();
    void showAdminMenu();
    void showOperatorMenu();
    void showPassengerMenu();

    // Guest operations
    void handleSearchTrains();
    void handleCheckPNR();
    void handlePassengerRegistration();
    void handleLogin(UserRole role);
};

} // namespace tms

#endif // TMS_RESERVATION_SYSTEM_H
