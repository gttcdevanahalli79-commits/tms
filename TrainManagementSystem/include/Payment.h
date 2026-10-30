#ifndef TMS_PAYMENT_H
#define TMS_PAYMENT_H

#include "Common.h"
#include <string>
#include <iostream>

namespace tms {

class Payment {
private:
    int m_paymentId;
    int m_bookingId;
    std::string m_transactionId;
    int64_t m_amountPaise;
    PaymentMethod m_method;
    PaymentStatus m_status;
    std::string m_timestamp;

public:
    Payment(int paymentId, int bookingId, std::string txId,
            int64_t amountPaise, PaymentMethod method,
            PaymentStatus status, std::string timestamp = "");

    int getPaymentId() const { return m_paymentId; }
    int getBookingId() const { return m_bookingId; }
    const std::string& getTransactionId() const { return m_transactionId; }
    int64_t getAmountPaise() const { return m_amountPaise; }
    PaymentMethod getMethod() const { return m_method; }
    PaymentStatus getStatus() const { return m_status; }
    const std::string& getTimestamp() const { return m_timestamp; }

    void setStatus(PaymentStatus status) { m_status = status; }

    static std::string generateTransactionId(PaymentMethod method);
    void printReceipt() const;
};

} // namespace tms

#endif // TMS_PAYMENT_H
