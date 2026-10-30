#ifndef TMS_REFUND_H
#define TMS_REFUND_H

#include "Common.h"
#include <string>
#include <iostream>

namespace tms {

class Refund {
private:
    int m_refundId;
    int m_bookingId;
    int m_paymentId;
    int64_t m_refundAmountPaise;
    int m_deductionPercentage;
    std::string m_reason;
    std::string m_timestamp;

public:
    Refund(int refundId, int bookingId, int paymentId,
           int64_t refundAmountPaise, int deductionPct,
           std::string reason, std::string timestamp = "");

    int getRefundId() const { return m_refundId; }
    int getBookingId() const { return m_bookingId; }
    int getPaymentId() const { return m_paymentId; }
    int64_t getRefundAmountPaise() const { return m_refundAmountPaise; }
    int getDeductionPercentage() const { return m_deductionPercentage; }
    const std::string& getReason() const { return m_reason; }
    const std::string& getTimestamp() const { return m_timestamp; }

    /**
     * @brief Refund policy calculation:
     * > 48 hours: 90% (10% deduction)
     * 24 - 48 hours: 75% (25% deduction)
     * 12 - 24 hours: 50% (50% deduction)
     * 4 - 12 hours: 25% (75% deduction)
     * < 4 hours: 0% (100% deduction)
     */
    static double getRefundPercentage(double hoursBeforeDeparture) {
        if (hoursBeforeDeparture > 48.0) return 0.90;
        if (hoursBeforeDeparture >= 24.0) return 0.75;
        if (hoursBeforeDeparture >= 12.0) return 0.50;
        if (hoursBeforeDeparture >= 4.0)  return 0.25;
        return 0.00;
    }

    void print() const;
};

} // namespace tms

#endif // TMS_REFUND_H
