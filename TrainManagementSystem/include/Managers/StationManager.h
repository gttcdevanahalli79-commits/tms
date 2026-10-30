#ifndef TMS_STATION_MANAGER_H
#define TMS_STATION_MANAGER_H

#include "Station.h"
#include <vector>
#include <memory>
#include <string>

namespace tms {

class StationManager {
public:
    StationManager();

    // Core CRUD
    bool addStation(const Station& station);
    bool updateStation(const Station& station);
    bool deleteStation(int stationId, std::string& outErrorMessage);

    // Lookups
    std::shared_ptr<Station> getStationById(int stationId);
    std::shared_ptr<Station> getStationByCode(const std::string& code);

    // Search & Filter Features (Admin & Passenger)
    std::vector<Station> getAllStations();
    std::vector<Station> getAllStationsSorted(bool ascending = true);
    std::vector<Station> searchByName(const std::string& nameQuery);
    std::vector<Station> searchByCode(const std::string& codeQuery);
    std::vector<Station> filterByDistrict(const std::string& district);
    std::vector<Station> filterByDivision(const std::string& division);
    std::vector<Station> filterByZone(const std::string& zone);

    // Statistics
    size_t getStationCount();
    std::vector<std::string> getDistinctDistricts();
    std::vector<std::string> getDistinctDivisions();

    // Display helpers
    void displayAllKarnatakaStations() const;
};

} // namespace tms

#endif // TMS_STATION_MANAGER_H
