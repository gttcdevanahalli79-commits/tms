#include <gtest/gtest.h>
#include "Common.h"
#include "User.h"
#include "Train.h"
#include "Coach.h"
#include "Seat.h"
#include "Booking.h"
#include "Payment.h"
#include "Refund.h"
#include "DatabaseManager.h"
#include "StationManager.h"
#include <fstream>
#include <sstream>
#include <unordered_set>

using namespace tms;

// Test Suite 1: User Authentication & Role Permissions
TEST(Phase1Tests, UserPasswordHashingAndRoles) {
    std::string rawPass = "SecretPass#2026";
    std::string hash = User::hashPassword(rawPass);
    EXPECT_FALSE(hash.empty());
    EXPECT_EQ(hash.length(), 64U); // 64 hex characters for SHA-256

    // Deterministic hashing check
    EXPECT_EQ(hash, User::hashPassword(rawPass));

    // Admin role
    Admin admin(1, "admin", hash, "Administrator", "admin@tms.com", "9999999999");
    EXPECT_TRUE(admin.verifyPassword(rawPass));
    EXPECT_FALSE(admin.verifyPassword("WrongPassword"));
    EXPECT_EQ(admin.getRole(), UserRole::ADMIN);
    EXPECT_TRUE(admin.canPerformAction("ANY_ACTION"));

    // Passenger role
    Passenger passenger(2, "john_doe", hash, "John Doe", "john@tms.com", "9888888888", "123 Street", 100);
    EXPECT_EQ(passenger.getRole(), UserRole::PASSENGER);
    EXPECT_TRUE(passenger.canPerformAction("BOOK_TICKET"));
    EXPECT_FALSE(passenger.canPerformAction("DELETE_TRAIN"));
    EXPECT_EQ(passenger.getLoyaltyPoints(), 100);
    passenger.addLoyaltyPoints(50);
    EXPECT_EQ(passenger.getLoyaltyPoints(), 150);
}

// Test Suite 2: Fare Calculation & Paise Precision
TEST(Phase1Tests, FareCalculationAccuracy) {
    // Distance 138 km (Bengaluru - Mysuru), Base fare Rs. 0.85/km, Class CC (1.3x), Train VANDE_BHARAT (1.7x)
    // Base: 138 * 0.85 * 1.3 * 1.7 = 259.1835
    // AC GST 5%: 259.1835 * 0.05 = 12.959
    // Reservation fee: 40.0
    // Total raw: 259.1835 + 12.959 + 40 = 312.14 -> 312 rounded
    // 312 * 100 = 31200 paise
    int64_t fareCC = calculateFareInPaise(138.0, 0.85, CoachClass::CC, TrainType::VANDE_BHARAT);
    EXPECT_EQ(fareCC, 31200);
    EXPECT_DOUBLE_EQ(paiseToRupees(fareCC), 312.0);

    // Non-AC Class SL: 1000 km, 0.50 base, SL 1.0x, EXPRESS 1.0x -> 500 + 40 = 540
    int64_t fareSL = calculateFareInPaise(1000.0, 0.50, CoachClass::SL, TrainType::EXPRESS);
    EXPECT_EQ(fareSL, 54000);
    EXPECT_DOUBLE_EQ(paiseToRupees(fareSL), 540.0);
}

// Test Suite 3: Berth Allocation Rules
TEST(Phase1Tests, BerthAllocationLogic) {
    // Sleeper / 3A layout (8 per compartment)
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::SL, 1), BerthType::LOWER);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::SL, 2), BerthType::MIDDLE);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::SL, 3), BerthType::UPPER);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::SL, 4), BerthType::LOWER);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::SL, 7), BerthType::SIDE_LOWER);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::SL, 8), BerthType::SIDE_UPPER);

    // 2A layout (6 per bay)
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::_2A, 1), BerthType::LOWER);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::_2A, 2), BerthType::UPPER);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::_2A, 5), BerthType::SIDE_LOWER);
    EXPECT_EQ(Seat::deduceBerthType(CoachClass::_2A, 6), BerthType::SIDE_UPPER);
}

// Test Suite 4: Refund Calculation Policy
TEST(Phase1Tests, RefundPolicyRules) {
    // > 48 hours: 90%
    EXPECT_DOUBLE_EQ(Refund::getRefundPercentage(72.0), 0.90);
    // 24 to 48 hours: 75%
    EXPECT_DOUBLE_EQ(Refund::getRefundPercentage(30.0), 0.75);
    // 12 to 24 hours: 50%
    EXPECT_DOUBLE_EQ(Refund::getRefundPercentage(18.0), 0.50);
    // 4 to 12 hours: 25%
    EXPECT_DOUBLE_EQ(Refund::getRefundPercentage(6.0), 0.25);
    // < 4 hours: 0%
    EXPECT_DOUBLE_EQ(Refund::getRefundPercentage(2.0), 0.00);
}

// Test Suite 5: 10-Digit PNR Generation & Uniqueness
TEST(Phase1Tests, PnrGenerationAndUniqueness) {
    std::unordered_set<std::string> pnrs;
    for (int i = 0; i < 50; ++i) {
        std::string pnr = Booking::generatePNR();
        EXPECT_EQ(pnr.length(), 10U);
        for (char c : pnr) {
            EXPECT_TRUE(std::isdigit(c));
        }
        EXPECT_TRUE(pnrs.find(pnr) == pnrs.end());
        pnrs.insert(pnr);
    }
}

// Test Suite 6: SQLite Schema and Database Transactions
TEST(Phase1Tests, SQLiteDatabaseAndTransactions) {
    auto& db = DatabaseManager::getInstance();
    // Use an in-memory database for isolated test execution
    db.open(":memory:");
    EXPECT_TRUE(db.isOpen());

    // Read schema.sql from either current or parent directory
    std::ifstream schemaFile("database/schema.sql");
    if (!schemaFile.is_open()) {
        schemaFile.open("../database/schema.sql");
    }
    ASSERT_TRUE(schemaFile.is_open());
    std::stringstream buffer;
    buffer << schemaFile.rdbuf();
    db.executeScript(buffer.str());

    // Verify Users table exists
    int rowCount = 0;
    db.executeQuery("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='Users';",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            rowCount = std::stoi(row[0]);
            return true;
        });
    EXPECT_EQ(rowCount, 1);

    // Test Transaction Commit
    {
        auto tx = db.beginTransaction();
        db.executeNonQuery("INSERT INTO Stations (code, name, city, district, state, railway_zone, railway_division, platforms_count) "
                           "VALUES ('TEST1', 'Test Stn 1', 'City1', 'Dist1', 'Karnataka', 'SWR', 'Bengaluru', 2);");
        tx.commit();
    }

    int stationCount = 0;
    db.executeQuery("SELECT count(*) FROM Stations WHERE code='TEST1';",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            stationCount = std::stoi(row[0]);
            return true;
        });
    EXPECT_EQ(stationCount, 1);

    // Test Transaction Rollback
    {
        auto tx = db.beginTransaction();
        db.executeNonQuery("INSERT INTO Stations (code, name, city, district, state, railway_zone, railway_division, platforms_count) "
                           "VALUES ('TEST2', 'Test Stn 2', 'City2', 'Dist2', 'Karnataka', 'SWR', 'Bengaluru', 2);");
        tx.rollback();
    }

    int rolledBackCount = 0;
    db.executeQuery("SELECT count(*) FROM Stations WHERE code='TEST2';",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            rolledBackCount = std::stoi(row[0]);
            return true;
        });
    EXPECT_EQ(rolledBackCount, 0); // Must be rolled back
}

// Test Suite 7: Karnataka Railway Station Management
TEST(Phase1Tests, KarnatakaStationManagement) {
    auto& db = DatabaseManager::getInstance();
    db.open(":memory:");

    // Apply schema
    std::ifstream schemaFile("database/schema.sql");
    if (!schemaFile.is_open()) schemaFile.open("../database/schema.sql");
    ASSERT_TRUE(schemaFile.is_open());
    std::stringstream sBuf;
    sBuf << schemaFile.rdbuf();
    db.executeScript(sBuf.str());

    // Apply seed
    std::ifstream seedFile("database/seed.sql");
    if (!seedFile.is_open()) seedFile.open("../database/seed.sql");
    ASSERT_TRUE(seedFile.is_open());
    std::stringstream seedBuf;
    seedBuf << seedFile.rdbuf();
    db.executeScript(seedBuf.str());

    StationManager mgr;
    size_t count = mgr.getStationCount();
    // Verify extensive Karnataka stations loaded
    EXPECT_GE(count, 70U);

    // Verify all stations have state == 'Karnataka'
    int nonKt = 0;
    db.executeQuery("SELECT COUNT(*) FROM Stations WHERE state != 'Karnataka';",
        [&](const std::vector<std::string>& row, const std::vector<std::string>&) {
            nonKt = std::stoi(row[0]);
            return true;
        });
    EXPECT_EQ(nonKt, 0);

    // Search by code: SBC
    auto sbc = mgr.getStationByCode("SBC");
    ASSERT_NE(sbc, nullptr);
    EXPECT_EQ(sbc->getCode(), "SBC");
    EXPECT_EQ(sbc->getName(), "KSR Bengaluru City");
    EXPECT_EQ(sbc->getState(), "Karnataka");
    EXPECT_EQ(sbc->getRailwayZone(), "South Western Railway");
    EXPECT_EQ(sbc->getRailwayDivision(), "Bengaluru");

    // Search by name: "Hubballi"
    auto ublResults = mgr.searchByName("Hubballi");
    EXPECT_FALSE(ublResults.empty());
    EXPECT_EQ(ublResults[0].getCode(), "UBL");

    // Filter by district: "Dakshina Kannada"
    auto dkStations = mgr.filterByDistrict("Dakshina Kannada");
    EXPECT_GE(dkStations.size(), 3U); // MAQ, MAJN, SL, SBHR

    // Filter by division: "Karwar"
    auto karwarDiv = mgr.filterByDivision("Karwar");
    EXPECT_GE(karwarDiv.size(), 8U); // Konkan railway coastal stations

    // Test Deletion Reference Protection
    std::string errMsg;
    bool delSuccess = mgr.deleteStation(1, errMsg); // SBC is on Route 1
    EXPECT_FALSE(delSuccess);
    EXPECT_FALSE(errMsg.empty());

    // Test Alphabetical Sorting
    auto sorted = mgr.getAllStationsSorted(true);
    ASSERT_GE(sorted.size(), 2U);
    EXPECT_LE(sorted.front().getName(), sorted.back().getName());
}
