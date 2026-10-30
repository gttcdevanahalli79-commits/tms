#include "Payment.h"
#include "Refund.h"
#include <random>
#include <iomanip>
#include <sstream>

namespace tms {

// ---------------- Payment ----------------
Payment::Payment(int paymentId, int bookingId, std::string txId,
                 int64_t amountPaise, PaymentMethod method,
                 PaymentStatus status, std::string timestamp)
    : m_paymentId(paymentId), m_bookingId(bookingId), m_transactionId(std::move(txId)),
      m_amountPaise(amountPaise), m_method(method), m_status(status),
      m_timestamp(std::move(timestamp)) {}

std::string Payment::generateTransactionId(PaymentMethod method) {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(100000000000ULL, 999999999999ULL);

    std::string prefix = "TXN";
    if (method == PaymentMethod::UPI) prefix = "UPI";
    else if (method == PaymentMethod::CARD) prefix = "CRD";
    else if (method == PaymentMethod::CASH) prefix = "CSH";

    return prefix + "_" + std::to_string(dist(gen));
}

void Payment::printReceipt() const {
    std::cout << "\n------------------ PAYMENT RECEIPT ------------------\n"
              << "TRANSACTION ID:  " << m_transactionId << "\n"
              << "BOOKING ID:      " << m_bookingId << "\n"
              << "AMOUNT:          Rs. " << std::fixed << std::setprecision(2) << paiseToRupees(m_amountPaise) << "\n"
              << "PAYMENT METHOD:  " << paymentMethodToString(m_method) << "\n"
              << "STATUS:          " << paymentStatusToString(m_status) << "\n"
              << "DATE & TIME:     " << m_timestamp << "\n"
              << "-----------------------------------------------------\n";
}

// ---------------- Refund ----------------
Refund::Refund(int refundId, int bookingId, int paymentId,
               int64_t refundAmountPaise, int deductionPct,
               std::string reason, std::string timestamp)
    : m_refundId(refundId), m_bookingId(bookingId), m_paymentId(paymentId),
      m_refundAmountPaise(refundAmountPaise), m_deductionPercentage(deductionPct),
      m_reason(std::move(reason)), m_timestamp(std::move(timestamp)) {}

void Refund::print() const {
    std::cout << "\n------------------ REFUND ADVICE ------------------\n"
              << "REFUND ID:        " << m_refundId << "\n"
              << "BOOKING ID:       " << m_bookingId << "\n"
              << "REFUND AMOUNT:    Rs. " << std::fixed << std::setprecision(2) << paiseToRupees(m_refundAmountPaise) << "\n"
              << "DEDUCTION:        " << m_deductionPercentage << "%\n"
              << "REASON:           " << m_reason << "\n"
              << "DATE & TIME:      " << m_timestamp << "\n"
              << "---------------------------------------------------\n";
}

} // namespace tms
