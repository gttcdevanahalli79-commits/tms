#include "StationManager.h"
#include "DatabaseManager.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace tms {

namespace {
    Station parseStationRow(const std::vector<std::string>& row) {
        int id = std::stoi(row[0]);
        std::string code = row[1];
        std::string name = row[2];
        std::string city = row[3];
        std::string district = row.size() > 4 ? row[4] : "";
        std::string state = row.size() > 5 ? row[5] : "Karnataka";
        std::string zone = row.size() > 6 ? row[6] : "South Western Railway";
        std::string division = row.size() > 7 ? row[7] : "Bengaluru";
        int platforms = row.size() > 8 ? std::stoi(row[8]) : 4;
        double lat = row.size() > 9 && !row[9].empty() ? std::stod(row[9]) : 0.0;
        double lon = row.size() > 10 && !row[10].empty() ? std::stod(row[10]) : 0.0;

        return Station(id, code, name, city, district, state, zone, division, platforms, lat, lon);
    }
}

StationManager::StationManager() {}

bool StationManager::addStation(const Station& station) {
    auto& db = DatabaseManager::getInstance();
    try {
        std::string sql = "INSERT INTO Stations (code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        std::vector<std::string> params = {
            station.getCode(),
            station.getName(),
            station.getCity(),
            station.getDistrict(),
            station.getState(),
            station.getRailwayZone(),
            station.getRailwayDivision(),
            std::to_string(station.getPlatformsCount()),
            std::to_string(station.getLatitude()),
            std::to_string(station.getLongitude())
        };
        db.executeNonQuery(sql, params);
        return true;
    } catch (...) {
        return false;
    }
}

bool StationManager::updateStation(const Station& station) {
    auto& db = DatabaseManager::getInstance();
    try {
        std::string sql = "UPDATE Stations SET code=?, name=?, city=?, district=?, state=?, railway_zone=?, railway_division=?, platforms_count=?, latitude=?, longitude=? "
                          "WHERE station_id=?;";
        std::vector<std::string> params = {
            station.getCode(),
            station.getName(),
            station.getCity(),
            station.getDistrict(),
            station.getState(),
            station.getRailwayZone(),
            station.getRailwayDivision(),
            std::to_string(station.getPlatformsCount()),
            std::to_string(station.getLatitude()),
            std::to_string(station.getLongitude()),
            std::to_string(station.getStationId())
        };
        return db.executeNonQuery(sql, params) > 0;
    } catch (...) {
        return false;
    }
}

bool StationManager::deleteStation(int stationId, std::string& outErrorMessage) {
    auto& db = DatabaseManager::getInstance();

    // 1. Check if station is referenced in active Routes
    int routeRefCount = 0;
    db.executeQuery("SELECT COUNT(*) FROM Routes WHERE source_station_id=? OR dest_station_id=?;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            routeRefCount = std::stoi(row[0]);
            return true;
        }, { std::to_string(stationId), std::to_string(stationId) });

    if (routeRefCount > 0) {
        outErrorMessage = "Cannot delete station: It is designated as source or destination on active routes.";
        return false;
    }

    // 2. Check if station is referenced in RouteStations
    int stopRefCount = 0;
    db.executeQuery("SELECT COUNT(*) FROM RouteStations WHERE station_id=?;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            stopRefCount = std::stoi(row[0]);
            return true;
        }, { std::to_string(stationId) });

    if (stopRefCount > 0) {
        outErrorMessage = "Cannot delete station: It is referenced as an intermediate stop on active route corridors.";
        return false;
    }

    // 3. Check if station is referenced in Bookings
    int bookingRefCount = 0;
    db.executeQuery("SELECT COUNT(*) FROM Bookings WHERE source_station_id=? OR dest_station_id=?;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            bookingRefCount = std::stoi(row[0]);
            return true;
        }, { std::to_string(stationId), std::to_string(stationId) });

    if (bookingRefCount > 0) {
        outErrorMessage = "Cannot delete station: Existing ticket booking records reference this station.";
        return false;
    }

    // 4. Safe to delete
    try {
        db.executeNonQuery("DELETE FROM Stations WHERE station_id=?;", { std::to_string(stationId) });
        return true;
    } catch (const std::exception& e) {
        outErrorMessage = e.what();
        return false;
    }
}

std::shared_ptr<Station> StationManager::getStationById(int stationId) {
    auto& db = DatabaseManager::getInstance();
    std::shared_ptr<Station> result = nullptr;
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations WHERE station_id=?;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            result = std::make_shared<Station>(parseStationRow(row));
            return false;
        }, { std::to_string(stationId) });
    return result;
}

std::shared_ptr<Station> StationManager::getStationByCode(const std::string& code) {
    auto& db = DatabaseManager::getInstance();
    std::shared_ptr<Station> result = nullptr;
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations WHERE code=? COLLATE NOCASE;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            result = std::make_shared<Station>(parseStationRow(row));
            return false;
        }, { code });
    return result;
}

std::vector<Station> StationManager::getAllStations() {
    auto& db = DatabaseManager::getInstance();
    std::vector<Station> list;
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations ORDER BY station_id ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            list.push_back(parseStationRow(row));
            return true;
        });
    return list;
}

std::vector<Station> StationManager::getAllStationsSorted(bool ascending) {
    auto list = getAllStations();
    std::sort(list.begin(), list.end(), [ascending](const Station& a, const Station& b) {
        return ascending ? (a.getName() < b.getName()) : (a.getName() > b.getName());
    });
    return list;
}

std::vector<Station> StationManager::searchByName(const std::string& nameQuery) {
    auto& db = DatabaseManager::getInstance();
    std::vector<Station> list;
    std::string pattern = "%" + nameQuery + "%";
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations WHERE name LIKE ? COLLATE NOCASE ORDER BY name ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            list.push_back(parseStationRow(row));
            return true;
        }, { pattern });
    return list;
}

std::vector<Station> StationManager::searchByCode(const std::string& codeQuery) {
    auto& db = DatabaseManager::getInstance();
    std::vector<Station> list;
    std::string pattern = "%" + codeQuery + "%";
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations WHERE code LIKE ? COLLATE NOCASE ORDER BY code ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            list.push_back(parseStationRow(row));
            return true;
        }, { pattern });
    return list;
}

std::vector<Station> StationManager::filterByDistrict(const std::string& district) {
    auto& db = DatabaseManager::getInstance();
    std::vector<Station> list;
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations WHERE district=? COLLATE NOCASE ORDER BY name ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            list.push_back(parseStationRow(row));
            return true;
        }, { district });
    return list;
}

std::vector<Station> StationManager::filterByDivision(const std::string& division) {
    auto& db = DatabaseManager::getInstance();
    std::vector<Station> list;
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations WHERE railway_division=? COLLATE NOCASE ORDER BY name ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            list.push_back(parseStationRow(row));
            return true;
        }, { division });
    return list;
}

std::vector<Station> StationManager::filterByZone(const std::string& zone) {
    auto& db = DatabaseManager::getInstance();
    std::vector<Station> list;
    db.executeQuery("SELECT station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude "
                    "FROM Stations WHERE railway_zone=? COLLATE NOCASE ORDER BY name ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            list.push_back(parseStationRow(row));
            return true;
        }, { zone });
    return list;
}

size_t StationManager::getStationCount() {
    auto& db = DatabaseManager::getInstance();
    size_t count = 0;
    db.executeQuery("SELECT COUNT(*) FROM Stations;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            count = static_cast<size_t>(std::stoul(row[0]));
            return true;
        });
    return count;
}

std::vector<std::string> StationManager::getDistinctDistricts() {
    auto& db = DatabaseManager::getInstance();
    std::vector<std::string> districts;
    db.executeQuery("SELECT DISTINCT district FROM Stations WHERE district != '' ORDER BY district ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            districts.push_back(row[0]);
            return true;
        });
    return districts;
}

std::vector<std::string> StationManager::getDistinctDivisions() {
    auto& db = DatabaseManager::getInstance();
    std::vector<std::string> divisions;
    db.executeQuery("SELECT DISTINCT railway_division FROM Stations WHERE railway_division != '' ORDER BY railway_division ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            divisions.push_back(row[0]);
            return true;
        });
    return divisions;
}

void StationManager::displayAllKarnatakaStations() const {
    auto& db = DatabaseManager::getInstance();
    std::cout << "\n=========================================================================\n"
              << "                   KARNATAKA RAILWAY STATION DATABASE                    \n"
              << "=========================================================================\n";
    int index = 1;
    db.executeQuery("SELECT code, name, city, district, railway_division, platforms_count FROM Stations ORDER BY name ASC;",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            std::cout << std::setw(2) << index++ << ". " << row[1] << " - " << row[0]
                      << " (" << row[3] << " Dist, " << row[4] << " Div, " << row[5] << " Pltfs)\n";
            return true;
        });
    std::cout << "=========================================================================\n";
}

} // namespace tms
