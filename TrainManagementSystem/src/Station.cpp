#include "Station.h"
#include <iomanip>

namespace tms {

Station::Station(int id, std::string code, std::string name, std::string city,
                 std::string district, std::string state,
                 std::string zone, std::string division,
                 int platforms, double lat, double lon)
    : m_stationId(id), m_code(std::move(code)), m_name(std::move(name)),
      m_city(std::move(city)), m_district(std::move(district)),
      m_state(std::move(state)), m_railwayZone(std::move(zone)),
      m_railwayDivision(std::move(division)), m_platformsCount(platforms),
      m_latitude(lat), m_longitude(lon) {}

Station::Station(int id, std::string code, std::string name, std::string city,
                 std::string state, int platforms)
    : m_stationId(id), m_code(std::move(code)), m_name(std::move(name)),
      m_city(std::move(city)), m_district("Bengaluru Urban"),
      m_state(std::move(state)), m_railwayZone("South Western Railway"),
      m_railwayDivision("Bengaluru"), m_platformsCount(platforms),
      m_latitude(0.0), m_longitude(0.0) {}

void Station::print() const {
    std::cout << "[" << m_code << "] " << m_name << " (" << m_city << ", " << m_district
              << ") | " << m_railwayZone << " - " << m_railwayDivision
              << " Div | Platforms: " << m_platformsCount << "\n";
}

void Station::printDetailed() const {
    std::cout << "--------------------------------------------------\n"
              << "Station Code:       " << m_code << "\n"
              << "Station Name:       " << m_name << "\n"
              << "City / Town:        " << m_city << "\n"
              << "District:           " << m_district << "\n"
              << "State:              " << m_state << "\n"
              << "Railway Zone:       " << m_railwayZone << "\n"
              << "Railway Division:   " << m_railwayDivision << "\n"
              << "Platforms:          " << m_platformsCount << "\n";
    if (m_latitude != 0.0 && m_longitude != 0.0) {
        std::cout << "Coordinates:        " << std::fixed << std::setprecision(4)
                  << m_latitude << " N, " << m_longitude << " E\n";
    }
    std::cout << "--------------------------------------------------\n";
}

} // namespace tms
