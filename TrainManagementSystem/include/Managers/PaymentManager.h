#ifndef TMS_PAYMENT_MANAGER_H
#define TMS_PAYMENT_MANAGER_H

#include "Payment.h"
#include "Refund.h"
#include <memory>
#include <vector>

namespace tms {

class PaymentManager {
public:
    PaymentManager();

    std::shared_ptr<Payment> processPayment(int bookingId, int64_t amountPaise, PaymentMethod method);
    std::shared_ptr<Refund> processRefund(int bookingId, int paymentId, int64_t refundAmountPaise, int deductionPct, const std::string& reason);
    std::shared_ptr<Payment> getPaymentByBookingId(int bookingId);
};

} // namespace tms

#endif // TMS_PAYMENT_MANAGER_H
