#ifndef TMS_REPORT_MANAGER_H
#define TMS_REPORT_MANAGER_H

#include "Common.h"
#include <string>
#include <vector>

namespace tms {

class ReportManager {
public:
    ReportManager();

    void displayRevenueReport() const;
    void displayOccupancyReport() const;
    void displayCancellationReport() const;
    void displayAuditLogs() const;
};

} // namespace tms

#endif // TMS_REPORT_MANAGER_H
