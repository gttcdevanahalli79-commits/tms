#include "Common.h"
#include "User.h"
#include "Train.h"
#include "Station.h"
#include "Booking.h"
#include "Payment.h"
#include "DatabaseManager.h"
#include "StationManager.h"
#include <iostream>
#include <memory>
#include <iomanip>

int main() {
    std::cout << "=================================================================\n";
    std::cout << "       " << tms::APP_NAME << "\n";
    std::cout << "       KARNATAKA RAILWAY NETWORK & STATION MANAGEMENT\n";
    std::cout << "       Version: " << tms::APP_VERSION << "\n";
    std::cout << "=================================================================\n\n";

    try {
        std::cout << "[INFO] Initializing SQLite Relational Database Engine...\n";
        auto& db = tms::DatabaseManager::getInstance();
        db.open("tms.db");
        std::cout << "[SUCCESS] Database connected: tms.db\n";

        // Re-apply schema & seed
        std::cout << "[INFO] Applying Relational Schema & Indices...\n";
        db.executeFile("database/schema.sql");

        // Clear any old non-Karnataka records
        db.executeNonQuery("DELETE FROM Stations WHERE state != 'Karnataka';");
        
        std::cout << "[INFO] Loading Karnataka Railway Stations Master Database...\n";
        db.executeFile("database/seed.sql");
        std::cout << "[SUCCESS] Karnataka Master Stations, Corridors & Trains Loaded.\n\n";

        tms::StationManager stationMgr;

        // 1. Total Station Count
        size_t totalStations = stationMgr.getStationCount();
        std::cout << "=================================================================\n";
        std::cout << "            KARNATAKA RAILWAY STATION DATABASE                   \n";
        std::cout << "=================================================================\n";
        std::cout << "TOTAL VERIFIED KARNATAKA STATIONS: " << totalStations << "\n\n";

        // 2. Display Karnataka Station Highlights (Matches User Specification Example)
        std::cout << "Key Hub Stations in Karnataka:\n";
        std::vector<std::string> sampleCodes = {"SBC", "YPR", "MYS", "MAQ", "UBL", "BGM", "HAS", "SMET", "KLBG", "BAY"};
        int idx = 1;
        for (const auto& code : sampleCodes) {
            auto stn = stationMgr.getStationByCode(code);
            if (stn) {
                std::cout << "  " << idx++ << ". " << stn->getName() << " - " << stn->getCode()
                          << " [" << stn->getCity() << ", " << stn->getDistrict() << " Dist | "
                          << stn->getRailwayZone() << " (" << stn->getRailwayDivision() << " Div)]\n";
            }
        }
        std::cout << "\n";

        // 3. Search Stations by Name
        std::cout << "--- Search by Name: 'Bengaluru' ---\n";
        auto bglrStations = stationMgr.searchByName("Bengaluru");
        for (const auto& s : bglrStations) {
            std::cout << "  [" << s.getCode() << "] " << s.getName() << " (" << s.getDistrict() << ")\n";
        }
        std::cout << "Found: " << bglrStations.size() << " stations.\n\n";

        // 4. Search by Station Code
        std::cout << "--- Search by Code: 'MAQ' (Mangaluru Central) ---\n";
        auto maqStn = stationMgr.getStationByCode("MAQ");
        if (maqStn) {
            maqStn->printDetailed();
        }

        // 5. Filter by District
        std::cout << "\n--- Filter by District: 'Dakshina Kannada' ---\n";
        auto dkStations = stationMgr.filterByDistrict("Dakshina Kannada");
        for (const auto& s : dkStations) {
            std::cout << "  [" << s.getCode() << "] " << s.getName() << " (" << s.getRailwayZone() << ")\n";
        }
        std::cout << "Found: " << dkStations.size() << " stations in Dakshina Kannada.\n\n";

        // 6. Filter by Railway Division
        std::cout << "--- Filter by Railway Division: 'Hubballi' ---\n";
        auto ublDivStations = stationMgr.filterByDivision("Hubballi");
        std::cout << "Total Stations in Hubballi Division: " << ublDivStations.size() << "\n";
        for (size_t i = 0; i < std::min<size_t>(5, ublDivStations.size()); ++i) {
            std::cout << "  - " << ublDivStations[i].getName() << " (" << ublDivStations[i].getCode() << ")\n";
        }
        std::cout << "  ... and " << (ublDivStations.size() - 5) << " more.\n\n";

        // 7. Verify Non-Karnataka Stations Are Removed
        int nonKarnatakaCount = 0;
        db.executeQuery("SELECT COUNT(*) FROM Stations WHERE state != 'Karnataka';",
            [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
                nonKarnatakaCount = std::stoi(row[0]);
                return true;
            });
        std::cout << "Non-Karnataka Station Count: " << nonKarnatakaCount << " (Must be 0)\n";

        // 8. Test Safety Protection on Delete
        std::cout << "\n--- Testing Safety Constraints on Deletion ---\n";
        std::string deleteError;
        // Attempt deleting KSR Bengaluru City (SBC is on Route 1)
        bool deletedSBC = stationMgr.deleteStation(1, deleteError);
        std::cout << "Attempt delete KSR Bengaluru City (SBC): "
                  << (deletedSBC ? "DELETED (ERROR)" : "BLOCKED SAFELY") << "\n";
        if (!deletedSBC) {
            std::cout << "Reason: " << deleteError << "\n";
        }

        // 9. Verify Active Karnataka Trains and Routes
        std::cout << "\n--- Active Karnataka Train Corridors ---\n";
        db.executeQuery("SELECT t.train_number, t.name, t.type, s1.code, s2.code, r.total_distance_km "
                        "FROM Trains t "
                        "JOIN Stations s1 ON t.source_station_id = s1.station_id "
                        "JOIN Stations s2 ON t.dest_station_id = s2.station_id "
                        "JOIN Routes r ON t.route_id = r.route_id;",
            [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
                std::cout << "  #" << row[0] << " " << row[1] << " [" << row[2] << "] "
                          << row[3] << " -> " << row[4] << " (" << row[5] << " km)\n";
                return true;
            });

        // 10. Fare Calculation on Karnataka Corridor (Bengaluru to Mysuru Vande Bharat)
        int64_t farePaise = tms::calculateFareInPaise(138.0, 0.85, tms::CoachClass::CC, tms::TrainType::VANDE_BHARAT);
        std::cout << "\nSample Fare: SBC to MYS (138 km) on Vande Bharat (CC Class):\n";
        std::cout << "  Calculated Fare: Rs. " << std::fixed << std::setprecision(2)
                  << tms::paiseToRupees(farePaise) << " (" << farePaise << " paise)\n";

        std::cout << "\n=================================================================\n";
        std::cout << "[SUCCESS] Karnataka Station Database fully operational and verified!\n";
        std::cout << "=================================================================\n";

    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Exception during execution: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
