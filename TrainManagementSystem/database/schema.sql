-- ============================================================================
-- Train Management System (TMS) - Relational Database Schema (SQLite3)
-- Strict constraints, foreign keys, triggers, and indices
-- ============================================================================

PRAGMA foreign_keys = ON;

-- 1. Users Table
CREATE TABLE IF NOT EXISTS Users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE COLLATE NOCASE,
    password_hash TEXT NOT NULL,
    full_name TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE COLLATE NOCASE,
    phone TEXT NOT NULL UNIQUE,
    role TEXT NOT NULL CHECK(role IN ('ADMIN', 'OPERATOR', 'PASSENGER')),
    extra_field TEXT DEFAULT '', -- Zone for Operator, Address for Passenger
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 2. Stations Table (Karnataka Railway Network)
CREATE TABLE IF NOT EXISTS Stations (
    station_id INTEGER PRIMARY KEY AUTOINCREMENT,
    code TEXT NOT NULL UNIQUE COLLATE NOCASE,
    name TEXT NOT NULL COLLATE NOCASE,
    city TEXT NOT NULL,
    district TEXT NOT NULL,
    state TEXT NOT NULL DEFAULT 'Karnataka' CHECK(state = 'Karnataka'),
    railway_zone TEXT NOT NULL,
    railway_division TEXT NOT NULL,
    platforms_count INTEGER DEFAULT 4 CHECK(platforms_count > 0),
    latitude REAL DEFAULT 0.0,
    longitude REAL DEFAULT 0.0
);

-- 3. Routes Table
CREATE TABLE IF NOT EXISTS Routes (
    route_id INTEGER PRIMARY KEY AUTOINCREMENT,
    route_name TEXT NOT NULL,
    source_station_id INTEGER NOT NULL REFERENCES Stations(station_id) ON DELETE RESTRICT,
    dest_station_id INTEGER NOT NULL REFERENCES Stations(station_id) ON DELETE RESTRICT,
    total_distance_km REAL NOT NULL CHECK(total_distance_km > 0)
);

-- 4. RouteStations Table (Stops and Sequence)
CREATE TABLE IF NOT EXISTS RouteStations (
    route_station_id INTEGER PRIMARY KEY AUTOINCREMENT,
    route_id INTEGER NOT NULL REFERENCES Routes(route_id) ON DELETE CASCADE,
    station_id INTEGER NOT NULL REFERENCES Stations(station_id) ON DELETE RESTRICT,
    stop_sequence INTEGER NOT NULL CHECK(stop_sequence >= 0),
    distance_from_start_km REAL NOT NULL CHECK(distance_from_start_km >= 0),
    halt_minutes INTEGER DEFAULT 5 CHECK(halt_minutes >= 0),
    UNIQUE(route_id, stop_sequence),
    UNIQUE(route_id, station_id)
);

-- 5. Trains Table
CREATE TABLE IF NOT EXISTS Trains (
    train_id INTEGER PRIMARY KEY AUTOINCREMENT,
    train_number TEXT NOT NULL UNIQUE,
    name TEXT NOT NULL,
    type TEXT NOT NULL CHECK(type IN ('LOCAL', 'EXPRESS', 'SUPERFAST', 'SLEEPER', 'RAJDHANI', 'VANDE_BHARAT')),
    source_station_id INTEGER NOT NULL REFERENCES Stations(station_id) ON DELETE RESTRICT,
    dest_station_id INTEGER NOT NULL REFERENCES Stations(station_id) ON DELETE RESTRICT,
    route_id INTEGER NOT NULL REFERENCES Routes(route_id) ON DELETE RESTRICT,
    base_fare_per_km REAL NOT NULL DEFAULT 0.50 CHECK(base_fare_per_km > 0),
    is_active INTEGER NOT NULL DEFAULT 1 CHECK(is_active IN (0, 1))
);

-- 6. Schedules Table
CREATE TABLE IF NOT EXISTS Schedules (
    schedule_id INTEGER PRIMARY KEY AUTOINCREMENT,
    train_id INTEGER NOT NULL REFERENCES Trains(train_id) ON DELETE CASCADE,
    departure_time TEXT NOT NULL, -- "HH:MM"
    arrival_time TEXT NOT NULL,   -- "HH:MM"
    operating_days TEXT NOT NULL DEFAULT 'Daily',
    delay_minutes INTEGER DEFAULT 0 CHECK(delay_minutes >= 0),
    is_overnight INTEGER DEFAULT 0 CHECK(is_overnight IN (0, 1))
);

-- 7. Coaches Table
CREATE TABLE IF NOT EXISTS Coaches (
    coach_id INTEGER PRIMARY KEY AUTOINCREMENT,
    train_id INTEGER NOT NULL REFERENCES Trains(train_id) ON DELETE CASCADE,
    coach_code TEXT NOT NULL, -- e.g., 'S1', 'B1', 'A1'
    coach_class TEXT NOT NULL CHECK(coach_class IN ('SL', '3A', '2A', '1A', 'CC', 'EC')),
    total_seats INTEGER NOT NULL DEFAULT 72 CHECK(total_seats > 0),
    UNIQUE(train_id, coach_code)
);

-- 8. Seats Table
CREATE TABLE IF NOT EXISTS Seats (
    seat_id INTEGER PRIMARY KEY AUTOINCREMENT,
    coach_id INTEGER NOT NULL REFERENCES Coaches(coach_id) ON DELETE CASCADE,
    seat_number INTEGER NOT NULL CHECK(seat_number > 0),
    berth_type TEXT NOT NULL CHECK(berth_type IN ('Lower', 'Middle', 'Upper', 'Side Lower', 'Side Upper', 'Window', 'Aisle')),
    UNIQUE(coach_id, seat_number)
);

-- 9. Bookings Table
CREATE TABLE IF NOT EXISTS Bookings (
    booking_id INTEGER PRIMARY KEY AUTOINCREMENT,
    pnr TEXT NOT NULL UNIQUE,
    user_id INTEGER NOT NULL REFERENCES Users(user_id) ON DELETE RESTRICT,
    train_id INTEGER NOT NULL REFERENCES Trains(train_id) ON DELETE RESTRICT,
    journey_date TEXT NOT NULL, -- "YYYY-MM-DD"
    source_station_id INTEGER NOT NULL REFERENCES Stations(station_id) ON DELETE RESTRICT,
    dest_station_id INTEGER NOT NULL REFERENCES Stations(station_id) ON DELETE RESTRICT,
    coach_class TEXT NOT NULL CHECK(coach_class IN ('SL', '3A', '2A', '1A', 'CC', 'EC')),
    total_fare_paise INTEGER NOT NULL CHECK(total_fare_paise >= 0),
    status TEXT NOT NULL CHECK(status IN ('CONFIRMED', 'WAITLISTED', 'CANCELLED')),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 10. Passengers Table
CREATE TABLE IF NOT EXISTS Passengers (
    passenger_id INTEGER PRIMARY KEY AUTOINCREMENT,
    booking_id INTEGER NOT NULL REFERENCES Bookings(booking_id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    age INTEGER NOT NULL CHECK(age > 0 AND age <= 120),
    gender TEXT NOT NULL CHECK(gender IN ('M', 'F', 'O')),
    berth_preference TEXT NOT NULL,
    allocated_coach_code TEXT NOT NULL,
    allocated_seat_number INTEGER NOT NULL,
    allocated_berth_type TEXT NOT NULL
);

-- 11. Payments Table
CREATE TABLE IF NOT EXISTS Payments (
    payment_id INTEGER PRIMARY KEY AUTOINCREMENT,
    booking_id INTEGER NOT NULL REFERENCES Bookings(booking_id) ON DELETE CASCADE,
    transaction_id TEXT NOT NULL UNIQUE,
    amount_paise INTEGER NOT NULL CHECK(amount_paise >= 0),
    payment_method TEXT NOT NULL CHECK(payment_method IN ('UPI', 'CARD', 'CASH')),
    status TEXT NOT NULL CHECK(status IN ('SUCCESS', 'FAILED', 'REFUNDED')),
    payment_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 12. Refunds Table
CREATE TABLE IF NOT EXISTS Refunds (
    refund_id INTEGER PRIMARY KEY AUTOINCREMENT,
    booking_id INTEGER NOT NULL REFERENCES Bookings(booking_id) ON DELETE CASCADE,
    payment_id INTEGER NOT NULL REFERENCES Payments(payment_id) ON DELETE CASCADE,
    refund_amount_paise INTEGER NOT NULL CHECK(refund_amount_paise >= 0),
    deduction_percentage INTEGER NOT NULL CHECK(deduction_percentage >= 0 AND deduction_percentage <= 100),
    reason TEXT NOT NULL,
    refund_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(booking_id) -- Prevent repeated refunds for same booking
);

-- 13. SeatInventory Table (Transactional Seat Locking and State Tracking)
CREATE TABLE IF NOT EXISTS SeatInventory (
    inventory_id INTEGER PRIMARY KEY AUTOINCREMENT,
    train_id INTEGER NOT NULL REFERENCES Trains(train_id) ON DELETE CASCADE,
    seat_id INTEGER NOT NULL REFERENCES Seats(seat_id) ON DELETE CASCADE,
    journey_date TEXT NOT NULL, -- "YYYY-MM-DD"
    booking_id INTEGER REFERENCES Bookings(booking_id) ON DELETE SET NULL,
    status TEXT NOT NULL CHECK(status IN ('AVAILABLE', 'BOOKED', 'LOCKED')),
    UNIQUE(train_id, seat_id, journey_date)
);

-- 14. AuditLogs Table
CREATE TABLE IF NOT EXISTS AuditLogs (
    log_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER REFERENCES Users(user_id) ON DELETE SET NULL,
    action TEXT NOT NULL,
    entity_name TEXT NOT NULL,
    entity_id TEXT NOT NULL,
    details TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- Performance Indices
-- ============================================================================
CREATE INDEX IF NOT EXISTS idx_users_username ON Users(username);
CREATE INDEX IF NOT EXISTS idx_stations_code ON Stations(code);
CREATE INDEX IF NOT EXISTS idx_stations_name ON Stations(name);
CREATE INDEX IF NOT EXISTS idx_stations_district ON Stations(district);
CREATE INDEX IF NOT EXISTS idx_stations_division ON Stations(railway_division);
CREATE INDEX IF NOT EXISTS idx_trains_number ON Trains(train_number);
CREATE INDEX IF NOT EXISTS idx_trains_route ON Trains(route_id);
CREATE INDEX IF NOT EXISTS idx_routestations_route ON RouteStations(route_id, stop_sequence);
CREATE INDEX IF NOT EXISTS idx_schedules_train ON Schedules(train_id);
CREATE INDEX IF NOT EXISTS idx_coaches_train ON Coaches(train_id);
CREATE INDEX IF NOT EXISTS idx_seats_coach ON Seats(coach_id);
CREATE INDEX IF NOT EXISTS idx_bookings_pnr ON Bookings(pnr);
CREATE INDEX IF NOT EXISTS idx_bookings_user ON Bookings(user_id);
CREATE INDEX IF NOT EXISTS idx_bookings_train_date ON Bookings(train_id, journey_date);
CREATE INDEX IF NOT EXISTS idx_inventory_lookup ON SeatInventory(train_id, journey_date, status);
CREATE INDEX IF NOT EXISTS idx_payments_booking ON Payments(booking_id);
CREATE INDEX IF NOT EXISTS idx_audit_timestamp ON AuditLogs(timestamp);
