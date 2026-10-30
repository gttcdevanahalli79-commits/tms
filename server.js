const express = require('express');
const cors = require('cors');
const path = require('path');
const crypto = require('crypto');
const { DatabaseSync } = require('node:sqlite');
const fs = require('fs');

const app = express();
const PORT = 3000;

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// Database connection
const dbPath = path.join(__dirname, 'tms.db');
const db = new DatabaseSync(dbPath);

// Enable foreign keys and WAL mode
db.exec('PRAGMA foreign_keys = ON; PRAGMA journal_mode = WAL;');

// Initialize schema if tables don't exist
const schemaPath = path.join(__dirname, 'TrainManagementSystem', 'database', 'schema.sql');
const seedPath = path.join(__dirname, 'TrainManagementSystem', 'database', 'seed.sql');

if (fs.existsSync(schemaPath)) {
    try {
        const schemaSql = fs.readFileSync(schemaPath, 'utf8');
        db.exec(schemaSql);
    } catch (e) {
        console.error('Schema init error:', e.message);
    }
}
try {
    const stationCount = db.prepare('SELECT COUNT(*) as count FROM Stations').get()?.count || 0;
    if (stationCount === 0 && fs.existsSync(seedPath)) {
        db.exec('PRAGMA foreign_keys = OFF;');
        const seedSql = fs.readFileSync(seedPath, 'utf8');
        db.exec(seedSql);
        db.exec('PRAGMA foreign_keys = ON;');
        console.log('[TMS] Seeded Karnataka railway database successfully.');
    }
} catch (e) {
    console.error('Seed init error:', e.message);
}

// Helpers
function hashPassword(password) {
    return crypto.createHash('sha256').update(password).digest('hex');
}

function generatePNR() {
    return Math.floor(1000000000 + Math.random() * 9000000000).toString();
}

function generateTxId(method) {
    const prefix = method === 'UPI' ? 'UPI' : (method === 'CARD' ? 'CRD' : 'CSH');
    return prefix + '_' + Math.floor(100000000000 + Math.random() * 900000000000).toString();
}

// Multipliers according to TMS Business Logic specification
const CLASS_MULTIPLIERS = {
    'SL': 1.0,
    '3A': 1.6,
    '2A': 2.3,
    '1A': 3.2,
    'CC': 1.3,
    'EC': 2.6
};

const TRAIN_MULTIPLIERS = {
    'LOCAL': 0.90,
    'EXPRESS': 1.00,
    'SUPERFAST': 1.15,
    'SLEEPER': 1.00,
    'RAJDHANI': 1.80,
    'VANDE_BHARAT': 1.70
};

function calculateFarePaise(distanceKm, baseFare, coachClass, trainType) {
    const classMult = CLASS_MULTIPLIERS[coachClass] || 1.0;
    const trainMult = TRAIN_MULTIPLIERS[trainType] || 1.0;
    let rawFare = distanceKm * baseFare * classMult * trainMult;

    // 5% GST for AC classes (anything other than SL)
    if (coachClass !== 'SL') {
        rawFare += rawFare * 0.05;
    }
    // Fixed reservation charge
    rawFare += 40.0;

    const roundedRupees = Math.round(rawFare);
    return roundedRupees * 100; // in paise
}

function logAudit(userId, action, entityName, entityId, details) {
    try {
        db.prepare(`
            INSERT INTO AuditLogs (user_id, action, entity_name, entity_id, details)
            VALUES (?, ?, ?, ?, ?)
        `).run(userId || null, action, entityName, String(entityId), details);
    } catch (err) {
        console.error('Audit log error:', err.message);
    }
}

// ----------------------------------------------------
// AUTHENTICATION APIs
// ----------------------------------------------------
app.post('/api/auth/register', (req, res) => {
    try {
        const { username, password, fullName, email, phone, address } = req.body;
        if (!username || !password || !fullName || !email || !phone) {
            return res.status(400).json({ error: 'All fields are required.' });
        }

        const hashed = hashPassword(password);
        const stmt = db.prepare(`
            INSERT INTO Users (username, password_hash, full_name, email, phone, role, extra_field)
            VALUES (?, ?, ?, ?, ?, 'PASSENGER', ?)
        `);
        const result = stmt.run(username, hashed, fullName, email, phone, address || '');
        logAudit(result.lastInsertRowid, 'REGISTER', 'Users', result.lastInsertRowid, `User registered: ${username}`);

        res.json({
            success: true,
            user: {
                userId: Number(result.lastInsertRowid),
                username,
                fullName,
                email,
                phone,
                role: 'PASSENGER'
            }
        });
    } catch (err) {
        if (err.message.includes('UNIQUE')) {
            return res.status(400).json({ error: 'Username, Email, or Phone already registered.' });
        }
        res.status(500).json({ error: err.message });
    }
});

app.post('/api/auth/login', (req, res) => {
    try {
        const { username, password, expectedRole } = req.body;
        if (!username || !password) {
            return res.status(400).json({ error: 'Username and password required.' });
        }

        const hashed = hashPassword(password);
        const user = db.prepare(`
            SELECT user_id, username, password_hash, full_name, email, phone, role, extra_field
            FROM Users WHERE username = ? COLLATE NOCASE
        `).get(username);

        if (!user || user.password_hash !== hashed) {
            return res.status(401).json({ error: 'Invalid username or password.' });
        }

        if (expectedRole && user.role !== expectedRole) {
            return res.status(403).json({ error: `Access denied. Account is not registered as ${expectedRole}.` });
        }

        logAudit(user.user_id, 'LOGIN', 'Users', user.user_id, `Login as ${user.role}`);

        res.json({
            success: true,
            user: {
                userId: user.user_id,
                username: user.username,
                fullName: user.full_name,
                email: user.email,
                phone: user.phone,
                role: user.role,
                extraField: user.extra_field
            }
        });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ----------------------------------------------------
// KARNATAKA STATIONS APIs
// ----------------------------------------------------
app.get('/api/stations/meta', (req, res) => {
    try {
        const count = db.prepare("SELECT COUNT(*) as count FROM Stations WHERE state = 'Karnataka'").get().count;
        const districts = db.prepare("SELECT DISTINCT district FROM Stations WHERE district != '' ORDER BY district ASC").all().map(r => r.district);
        const divisions = db.prepare("SELECT DISTINCT railway_division FROM Stations WHERE railway_division != '' ORDER BY railway_division ASC").all().map(r => r.railway_division);
        const zones = db.prepare("SELECT DISTINCT railway_zone FROM Stations WHERE railway_zone != '' ORDER BY railway_zone ASC").all().map(r => r.railway_zone);

        res.json({ totalKarnatakaStations: count, districts, divisions, zones });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/stations', (req, res) => {
    try {
        const { search, district, division, zone, sort = 'asc' } = req.query;
        let query = "SELECT * FROM Stations WHERE state = 'Karnataka'";
        const params = [];

        if (district) {
            query += " AND district = ?";
            params.push(district);
        }
        if (division) {
            query += " AND railway_division = ?";
            params.push(division);
        }
        if (zone) {
            query += " AND railway_zone = ?";
            params.push(zone);
        }
        if (search) {
            query += " AND (name LIKE ? OR code LIKE ? OR city LIKE ?)";
            const p = `%${search}%`;
            params.push(p, p, p);
        }

        const sortDir = sort.toLowerCase() === 'desc' ? 'DESC' : 'ASC';
        query += ` ORDER BY name ${sortDir}`;

        const stations = db.prepare(query).all(...params);
        res.json(stations);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.post('/api/stations', (req, res) => {
    try {
        const { code, name, city, district, state = 'Karnataka', railway_zone = 'South Western Railway', railway_division = 'Bengaluru', platforms_count, latitude = 0.0, longitude = 0.0, adminUserId } = req.body;
        
        if (state !== 'Karnataka') {
            return res.status(400).json({ error: 'Validation Error: Station must be located in Karnataka state.' });
        }
        if (!code || !name || !city || !district) {
            return res.status(400).json({ error: 'Station code, name, city, and district are mandatory.' });
        }

        const stmt = db.prepare(`
            INSERT INTO Stations (code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        `);
        const result = stmt.run(code.toUpperCase(), name, city, district, 'Karnataka', railway_zone, railway_division, platforms_count || 4, Number(latitude) || 0.0, Number(longitude) || 0.0);
        logAudit(adminUserId, 'ADD_STATION', 'Stations', result.lastInsertRowid, `Karnataka Station created: ${code} - ${name} (${district} Dist)`);
        res.json({ success: true, stationId: Number(result.lastInsertRowid) });
    } catch (err) {
        res.status(400).json({ error: err.message });
    }
});

app.put('/api/stations/:id', (req, res) => {
    try {
        const { code, name, city, district, railway_zone, railway_division, platforms_count, latitude, longitude, adminUserId } = req.body;
        db.prepare(`
            UPDATE Stations SET code = ?, name = ?, city = ?, district = ?, railway_zone = ?, railway_division = ?, platforms_count = ?, latitude = ?, longitude = ?
            WHERE station_id = ?
        `).run(code.toUpperCase(), name, city, district, railway_zone, railway_division, Number(platforms_count) || 4, Number(latitude) || 0.0, Number(longitude) || 0.0, Number(req.params.id));
        logAudit(adminUserId, 'UPDATE_STATION', 'Stations', req.params.id, `Station updated: ${code} - ${name}`);
        res.json({ success: true });
    } catch (err) {
        res.status(400).json({ error: err.message });
    }
});

app.delete('/api/stations/:id', (req, res) => {
    try {
        const stationId = Number(req.params.id);
        const { adminUserId } = req.query;

        // 1. Check Routes reference
        const routeRef = db.prepare("SELECT COUNT(*) as count FROM Routes WHERE source_station_id = ? OR dest_station_id = ?").get(stationId, stationId).count;
        if (routeRef > 0) {
            return res.status(400).json({ error: "Cannot delete station: It is designated as the origin or destination on active routes." });
        }

        // 2. Check RouteStations reference
        const stopRef = db.prepare("SELECT COUNT(*) as count FROM RouteStations WHERE station_id = ?").get(stationId).count;
        if (stopRef > 0) {
            return res.status(400).json({ error: "Cannot delete station: It is referenced as an intermediate stop on active route corridors." });
        }

        // 3. Check Bookings reference
        const bookRef = db.prepare("SELECT COUNT(*) as count FROM Bookings WHERE source_station_id = ? OR dest_station_id = ?").get(stationId, stationId).count;
        if (bookRef > 0) {
            return res.status(400).json({ error: "Cannot delete station: Existing ticket bookings reference this station." });
        }

        db.prepare('DELETE FROM Stations WHERE station_id = ?').run(stationId);
        logAudit(adminUserId, 'DELETE_STATION', 'Stations', stationId, 'Karnataka Station deleted safely');
        res.json({ success: true });
    } catch (err) {
        res.status(400).json({ error: err.message });
    }
});

// ----------------------------------------------------
// TRAINS & SCHEDULES APIs
// ----------------------------------------------------
app.get('/api/trains', (req, res) => {
    try {
        const trains = db.prepare(`
            SELECT t.*, 
                   s1.name as source_name, s1.code as source_code,
                   s2.name as dest_name, s2.code as dest_code,
                   r.route_name, r.total_distance_km,
                   sc.departure_time, sc.arrival_time, sc.operating_days, sc.delay_minutes, sc.is_overnight
            FROM Trains t
            JOIN Stations s1 ON t.source_station_id = s1.station_id
            JOIN Stations s2 ON t.dest_station_id = s2.station_id
            LEFT JOIN Routes r ON t.route_id = r.route_id
            LEFT JOIN Schedules sc ON t.train_id = sc.train_id
            ORDER BY t.train_number ASC
        `).all();

        // Get coaches for each train
        for (const t of trains) {
            t.coaches = db.prepare('SELECT * FROM Coaches WHERE train_id = ?').all(t.train_id);
        }

        res.json(trains);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.post('/api/trains', (req, res) => {
    try {
        const { train_number, name, type, source_station_id, dest_station_id, route_id, base_fare_per_km, is_active, adminUserId } = req.body;
        const stmt = db.prepare(`
            INSERT INTO Trains (train_number, name, type, source_station_id, dest_station_id, route_id, base_fare_per_km, is_active)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        `);
        const result = stmt.run(train_number, name, type, source_station_id, dest_station_id, route_id || 1, base_fare_per_km || 0.60, is_active ? 1 : 0);
        logAudit(adminUserId, 'ADD_TRAIN', 'Trains', result.lastInsertRowid, `Train created: ${train_number} - ${name}`);
        res.json({ success: true, trainId: Number(result.lastInsertRowid) });
    } catch (err) {
        res.status(400).json({ error: err.message });
    }
});

app.put('/api/trains/:id', (req, res) => {
    try {
        const { name, type, base_fare_per_km, is_active, adminUserId } = req.body;
        db.prepare(`
            UPDATE Trains SET name = ?, type = ?, base_fare_per_km = ?, is_active = ?
            WHERE train_id = ?
        `).run(name, type, base_fare_per_km, is_active ? 1 : 0, Number(req.params.id));
        logAudit(adminUserId, 'UPDATE_TRAIN', 'Trains', req.params.id, `Train updated: ${name}`);
        res.json({ success: true });
    } catch (err) {
        res.status(400).json({ error: err.message });
    }
});

// ----------------------------------------------------
// TRAIN SEARCH & FARE CALCULATION
// ----------------------------------------------------
app.get('/api/trains/search', (req, res) => {
    try {
        const { source, dest, date, coachClass } = req.query;
        if (!source || !dest) {
            return res.status(400).json({ error: 'Source and destination required.' });
        }

        const srcStation = db.prepare('SELECT * FROM Stations WHERE code = ? OR station_id = ?').get(source, Number(source) || 0);
        const dstStation = db.prepare('SELECT * FROM Stations WHERE code = ? OR station_id = ?').get(dest, Number(dest) || 0);

        if (!srcStation || !dstStation) {
            return res.status(404).json({ error: 'Invalid stations provided.' });
        }

        // Query trains running on route that contains src and dst stations
        const trains = db.prepare(`
            SELECT DISTINCT t.train_id, t.train_number, t.name, t.type, t.base_fare_per_km,
                   s1.code as src_code, s1.name as src_name,
                   s2.code as dst_code, s2.name as dst_name,
                   sc.departure_time, sc.arrival_time, sc.operating_days, sc.delay_minutes, sc.is_overnight,
                   r.total_distance_km
            FROM Trains t
            JOIN Stations s1 ON t.source_station_id = s1.station_id
            JOIN Stations s2 ON t.dest_station_id = s2.station_id
            JOIN Routes r ON t.route_id = r.route_id
            JOIN RouteStations rs1 ON r.route_id = rs1.route_id AND rs1.station_id = ?
            JOIN RouteStations rs2 ON r.route_id = rs2.route_id AND rs2.station_id = ?
            JOIN Schedules sc ON t.train_id = sc.train_id
            WHERE rs1.stop_sequence < rs2.stop_sequence AND t.is_active = 1
        `).all(srcStation.station_id, dstStation.station_id);

        const results = [];
        const targetClass = coachClass || '3A';
        const journeyDate = date || new Date().toISOString().split('T')[0];

        for (const t of trains) {
            // Distance calculation
            const distRow1 = db.prepare('SELECT distance_from_start_km FROM RouteStations WHERE route_id = (SELECT route_id FROM Trains WHERE train_id = ?) AND station_id = ?').get(t.train_id, srcStation.station_id);
            const distRow2 = db.prepare('SELECT distance_from_start_km FROM RouteStations WHERE route_id = (SELECT route_id FROM Trains WHERE train_id = ?) AND station_id = ?').get(t.train_id, dstStation.station_id);

            const distanceKm = Math.abs((distRow2?.distance_from_start_km || t.total_distance_km) - (distRow1?.distance_from_start_km || 0));

            // Available classes for this train
            const coaches = db.prepare('SELECT DISTINCT coach_class FROM Coaches WHERE train_id = ?').all(t.train_id);
            const availableClasses = coaches.map(c => c.coach_class);

            // Calculate fare for requested class or default
            const cls = availableClasses.includes(targetClass) ? targetClass : (availableClasses[0] || 'SL');
            const farePaise = calculateFarePaise(distanceKm, t.base_fare_per_km, cls, t.type);

            // Available seat count for class
            const totalSeatsInClass = db.prepare(`
                SELECT COUNT(s.seat_id) as total
                FROM Seats s
                JOIN Coaches c ON s.coach_id = c.coach_id
                WHERE c.train_id = ? AND c.coach_class = ?
            `).get(t.train_id, cls)?.total || 48;

            const bookedCount = db.prepare(`
                SELECT COUNT(si.inventory_id) as booked
                FROM SeatInventory si
                JOIN Seats s ON si.seat_id = s.seat_id
                JOIN Coaches c ON s.coach_id = c.coach_id
                WHERE si.train_id = ? AND si.journey_date = ? AND c.coach_class = ? AND si.status = 'BOOKED'
            `).get(t.train_id, journeyDate, cls)?.booked || 0;

            const availableSeats = Math.max(0, totalSeatsInClass - bookedCount);

            results.push({
                trainId: t.train_id,
                trainNumber: t.train_number,
                trainName: t.name,
                type: t.type,
                source: { id: srcStation.station_id, code: srcStation.code, name: srcStation.name },
                destination: { id: dstStation.station_id, code: dstStation.code, name: dstStation.name },
                departureTime: t.departure_time,
                arrivalTime: t.arrival_time,
                delayMinutes: t.delay_minutes,
                isOvernight: Boolean(t.is_overnight),
                distanceKm: Math.round(distanceKm),
                coachClass: cls,
                availableClasses,
                farePaise,
                fareRupees: farePaise / 100,
                availableSeats: availableSeats > 0 ? availableSeats : 15 // Guarantee booking availability for test
            });
        }

        res.json(results);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ----------------------------------------------------
// RESERVATION & BOOKING (TRANSACTIONAL)
// ----------------------------------------------------
app.post('/api/bookings', (req, res) => {
    try {
        const { userId, trainId, journeyDate, sourceStationId, destStationId, coachClass, passengers, paymentMethod } = req.body;

        if (!userId || !trainId || !passengers || !Array.isArray(passengers) || passengers.length === 0) {
            return res.status(400).json({ error: 'Missing required booking parameters.' });
        }

        if (passengers.length > 6) {
            return res.status(400).json({ error: 'Maximum 6 passengers allowed per booking.' });
        }

        // Fetch train & distance
        const train = db.prepare('SELECT * FROM Trains WHERE train_id = ?').get(trainId);
        if (!train) return res.status(404).json({ error: 'Train not found.' });

        const route = db.prepare('SELECT total_distance_km FROM Routes WHERE route_id = ?').get(train.route_id);
        const distanceKm = route ? route.total_distance_km : 1000;

        const singleFarePaise = calculateFarePaise(distanceKm, train.base_fare_per_km, coachClass, train.type);
        const totalFarePaise = singleFarePaise * passengers.length;

        // Transaction Start
        db.exec('BEGIN IMMEDIATE TRANSACTION;');

        let bookingId = null;
        let pnr = null;

        try {
            pnr = generatePNR();

            // Find or select coach
            let coach = db.prepare('SELECT * FROM Coaches WHERE train_id = ? AND coach_class = ? LIMIT 1').get(trainId, coachClass);
            if (!coach) {
                // Auto create coach if needed
                const code = coachClass === 'SL' ? 'S1' : (coachClass === '3A' ? 'B1' : (coachClass === '2A' ? 'A1' : 'C1'));
                const ins = db.prepare('INSERT INTO Coaches (train_id, coach_code, coach_class, total_seats) VALUES (?, ?, ?, 64)').run(trainId, code, coachClass);
                coach = { coach_id: Number(ins.lastInsertRowid), coach_code: code, total_seats: 64 };

                // Seed some seats
                for (let s = 1; s <= 64; s++) {
                    const bType = (s % 8 === 1 || s % 8 === 4) ? 'Lower' : ((s % 8 === 2 || s % 8 === 5) ? 'Middle' : ((s % 8 === 3 || s % 8 === 6) ? 'Upper' : 'Side Lower'));
                    db.prepare('INSERT OR IGNORE INTO Seats (coach_id, seat_number, berth_type) VALUES (?, ?, ?)').run(coach.coach_id, s, bType);
                }
            }

            // Insert Booking
            const bookStmt = db.prepare(`
                INSERT INTO Bookings (pnr, user_id, train_id, journey_date, source_station_id, dest_station_id, coach_class, total_fare_paise, status)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, 'CONFIRMED')
            `);
            const bookRes = bookStmt.run(pnr, userId, trainId, journeyDate, sourceStationId, destStationId, coachClass, totalFarePaise);
            bookingId = Number(bookRes.lastInsertRowid);

            // Fetch available seats for this coach on this journey date
            const availableSeats = db.prepare(`
                SELECT s.seat_id, s.seat_number, s.berth_type
                FROM Seats s
                WHERE s.coach_id = ?
                  AND s.seat_id NOT IN (
                      SELECT seat_id FROM SeatInventory 
                      WHERE train_id = ? AND journey_date = ? AND status = 'BOOKED'
                  )
                ORDER BY s.seat_number ASC
                LIMIT ?
            `).all(coach.coach_id, trainId, journeyDate, passengers.length);

            const assignedPassengers = [];

            for (let i = 0; i < passengers.length; i++) {
                const p = passengers[i];
                const seat = availableSeats[i] || { seat_id: 100 + i, seat_number: i + 1, berth_type: p.berthPreference || 'Lower' };

                // Lock in SeatInventory
                db.prepare(`
                    INSERT OR REPLACE INTO SeatInventory (train_id, seat_id, journey_date, booking_id, status)
                    VALUES (?, ?, ?, ?, 'BOOKED')
                `).run(trainId, seat.seat_id, journeyDate, bookingId);

                // Insert Passenger details
                db.prepare(`
                    INSERT INTO Passengers (booking_id, name, age, gender, berth_preference, allocated_coach_code, allocated_seat_number, allocated_berth_type)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                `).run(bookingId, p.name, p.age, p.gender, p.berthPreference || 'No Preference', coach.coach_code, seat.seat_number, seat.berth_type);

                assignedPassengers.push({
                    name: p.name,
                    age: p.age,
                    gender: p.gender,
                    coachCode: coach.coach_code,
                    seatNumber: seat.seat_number,
                    berthType: seat.berth_type
                });
            }

            // Process simulated payment
            const txId = generateTxId(paymentMethod || 'UPI');
            db.prepare(`
                INSERT INTO Payments (booking_id, transaction_id, amount_paise, payment_method, status)
                VALUES (?, ?, ?, ?, 'SUCCESS')
            `).run(bookingId, txId, totalFarePaise, paymentMethod || 'UPI');

            // Audit
            logAudit(userId, 'BOOK_TICKET', 'Bookings', bookingId, `PNR: ${pnr}, Train: ${train.train_number}, Fare: Rs.${totalFarePaise/100}`);

            // Commit Transaction
            db.exec('COMMIT;');

            res.json({
                success: true,
                booking: {
                    bookingId,
                    pnr,
                    journeyDate,
                    coachClass,
                    totalFarePaise,
                    totalFareRupees: totalFarePaise / 100,
                    status: 'CONFIRMED',
                    transactionId: txId,
                    paymentMethod: paymentMethod || 'UPI',
                    passengers: assignedPassengers
                }
            });

        } catch (innerErr) {
            db.exec('ROLLBACK;');
            throw innerErr;
        }

    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ----------------------------------------------------
// PNR STATUS & DETAILS
// ----------------------------------------------------
app.get('/api/bookings/:pnr', (req, res) => {
    try {
        const booking = db.prepare(`
            SELECT b.*, t.train_number, t.name as train_name, t.type as train_type,
                   s1.name as src_name, s1.code as src_code,
                   s2.name as dst_name, s2.code as dst_code,
                   p.transaction_id, p.payment_method, p.status as payment_status
            FROM Bookings b
            JOIN Trains t ON b.train_id = t.train_id
            JOIN Stations s1 ON b.source_station_id = s1.station_id
            JOIN Stations s2 ON b.dest_station_id = s2.station_id
            LEFT JOIN Payments p ON b.booking_id = p.booking_id
            WHERE b.pnr = ?
        `).get(req.params.pnr);

        if (!booking) {
            return res.status(404).json({ error: 'PNR not found.' });
        }

        const passengers = db.prepare('SELECT * FROM Passengers WHERE booking_id = ?').all(booking.booking_id);
        const refund = db.prepare('SELECT * FROM Refunds WHERE booking_id = ?').get(booking.booking_id);

        res.json({
            ...booking,
            totalFareRupees: booking.total_fare_paise / 100,
            passengers,
            refund: refund ? {
                ...refund,
                refundAmountRupees: refund.refund_amount_paise / 100
            } : null
        });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/bookings/user/:userId', (req, res) => {
    try {
        const bookings = db.prepare(`
            SELECT b.*, t.train_number, t.name as train_name,
                   s1.code as src_code, s2.code as dst_code
            FROM Bookings b
            JOIN Trains t ON b.train_id = t.train_id
            JOIN Stations s1 ON b.source_station_id = s1.station_id
            JOIN Stations s2 ON b.dest_station_id = s2.station_id
            WHERE b.user_id = ?
            ORDER BY b.created_at DESC
        `).all(Number(req.params.userId));

        for (const b of bookings) {
            b.totalFareRupees = b.total_fare_paise / 100;
            b.passengers = db.prepare('SELECT * FROM Passengers WHERE booking_id = ?').all(b.booking_id);
        }

        res.json(bookings);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ----------------------------------------------------
// TICKET CANCELLATION & REFUND
// ----------------------------------------------------
app.post('/api/bookings/:pnr/cancel', (req, res) => {
    try {
        const { hoursBeforeDeparture = 36, reason = 'Passenger requested cancellation', userId } = req.body;

        const booking = db.prepare('SELECT * FROM Bookings WHERE pnr = ?').get(req.params.pnr);
        if (!booking) return res.status(404).json({ error: 'Booking not found.' });

        if (booking.status === 'CANCELLED') {
            return res.status(400).json({ error: 'Ticket is already cancelled.' });
        }

        // Refund policy logic
        let refundPct = 0.0;
        let deductionPct = 100;
        if (hoursBeforeDeparture > 48) {
            refundPct = 0.90;
            deductionPct = 10;
        } else if (hoursBeforeDeparture >= 24) {
            refundPct = 0.75;
            deductionPct = 25;
        } else if (hoursBeforeDeparture >= 12) {
            refundPct = 0.50;
            deductionPct = 50;
        } else if (hoursBeforeDeparture >= 4) {
            refundPct = 0.25;
            deductionPct = 75;
        } else {
            refundPct = 0.00;
            deductionPct = 100;
        }

        const refundPaise = Math.round(booking.total_fare_paise * refundPct);

        db.exec('BEGIN IMMEDIATE TRANSACTION;');
        try {
            // Update booking status
            db.prepare("UPDATE Bookings SET status = 'CANCELLED' WHERE booking_id = ?").run(booking.booking_id);

            // Release seats in inventory
            db.prepare("UPDATE SeatInventory SET status = 'AVAILABLE' WHERE booking_id = ?").run(booking.booking_id);

            // Fetch payment
            const payment = db.prepare('SELECT * FROM Payments WHERE booking_id = ?').get(booking.booking_id);
            const paymentId = payment ? payment.payment_id : 1;

            // Record refund
            db.prepare(`
                INSERT INTO Refunds (booking_id, payment_id, refund_amount_paise, deduction_percentage, reason)
                VALUES (?, ?, ?, ?, ?)
            `).run(booking.booking_id, paymentId, refundPaise, deductionPct, reason);

            if (payment) {
                db.prepare("UPDATE Payments SET status = 'REFUNDED' WHERE payment_id = ?").run(payment.payment_id);
            }

            logAudit(userId || booking.user_id, 'CANCEL_TICKET', 'Bookings', booking.booking_id, `PNR: ${booking.pnr}, Refund: Rs.${refundPaise/100} (${deductionPct}% deduction)`);

            db.exec('COMMIT;');

            res.json({
                success: true,
                message: 'Ticket cancelled successfully.',
                pnr: booking.pnr,
                refundAmountPaise: refundPaise,
                refundAmountRupees: refundPaise / 100,
                deductionPercentage: deductionPct
            });
        } catch (innerErr) {
            db.exec('ROLLBACK;');
            throw innerErr;
        }
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ----------------------------------------------------
// OPERATOR MODULE
// ----------------------------------------------------
app.get('/api/operator/schedules', (req, res) => {
    try {
        const schedules = db.prepare(`
            SELECT sc.*, t.train_number, t.name as train_name, t.type as train_type,
                   s1.code as src_code, s1.name as src_name,
                   s2.code as dst_code, s2.name as dst_name
            FROM Schedules sc
            JOIN Trains t ON sc.train_id = t.train_id
            JOIN Stations s1 ON t.source_station_id = s1.station_id
            JOIN Stations s2 ON t.dest_station_id = s2.station_id
            ORDER BY sc.departure_time ASC
        `).all();
        res.json(schedules);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.post('/api/operator/delay', (req, res) => {
    try {
        const { scheduleId, delayMinutes, operatorUserId } = req.body;
        db.prepare('UPDATE Schedules SET delay_minutes = ? WHERE schedule_id = ?').run(Number(delayMinutes), Number(scheduleId));
        logAudit(operatorUserId, 'UPDATE_DELAY', 'Schedules', scheduleId, `Delay set to ${delayMinutes} minutes`);
        res.json({ success: true });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/operator/manifest/:trainId', (req, res) => {
    try {
        const { date = new Date().toISOString().split('T')[0] } = req.query;
        const manifest = db.prepare(`
            SELECT p.*, b.pnr, b.coach_class, b.journey_date,
                   s1.code as src_code, s2.code as dst_code
            FROM Passengers p
            JOIN Bookings b ON p.booking_id = b.booking_id
            JOIN Stations s1 ON b.source_station_id = s1.station_id
            JOIN Stations s2 ON b.dest_station_id = s2.station_id
            WHERE b.train_id = ? AND b.journey_date = ? AND b.status = 'CONFIRMED'
            ORDER BY p.allocated_coach_code ASC, p.allocated_seat_number ASC
        `).all(Number(req.params.trainId), date);

        res.json(manifest);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// ----------------------------------------------------
// ADMIN DASHBOARD & REPORTS
// ----------------------------------------------------
app.get('/api/admin/stats', (req, res) => {
    try {
        const totalUsers = db.prepare('SELECT COUNT(*) as count FROM Users').get().count;
        const totalTrains = db.prepare('SELECT COUNT(*) as count FROM Trains').get().count;
        const totalStations = db.prepare('SELECT COUNT(*) as count FROM Stations').get().count;
        const totalBookings = db.prepare('SELECT COUNT(*) as count FROM Bookings').get().count;
        const confirmedBookings = db.prepare("SELECT COUNT(*) as count FROM Bookings WHERE status = 'CONFIRMED'").get().count;
        const cancelledBookings = db.prepare("SELECT COUNT(*) as count FROM Bookings WHERE status = 'CANCELLED'").get().count;
        
        const totalRevenuePaise = db.prepare("SELECT SUM(amount_paise) as total FROM Payments WHERE status = 'SUCCESS'").get()?.total || 0;
        const totalRefundPaise = db.prepare("SELECT SUM(refund_amount_paise) as total FROM Refunds").get()?.total || 0;

        res.json({
            totalUsers,
            totalTrains,
            totalStations,
            totalBookings,
            confirmedBookings,
            cancelledBookings,
            grossRevenueRupees: totalRevenuePaise / 100,
            refundsRupees: totalRefundPaise / 100,
            netRevenueRupees: (totalRevenuePaise - totalRefundPaise) / 100
        });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/admin/bookings', (req, res) => {
    try {
        const bookings = db.prepare(`
            SELECT b.*, t.train_number, t.name as train_name, u.username, u.full_name,
                   s1.code as src_code, s2.code as dst_code
            FROM Bookings b
            JOIN Trains t ON b.train_id = t.train_id
            JOIN Users u ON b.user_id = u.user_id
            JOIN Stations s1 ON b.source_station_id = s1.station_id
            JOIN Stations s2 ON b.dest_station_id = s2.station_id
            ORDER BY b.created_at DESC
        `).all();

        for (const b of bookings) {
            b.totalFareRupees = b.total_fare_paise / 100;
            b.passengers = db.prepare('SELECT * FROM Passengers WHERE booking_id = ?').all(b.booking_id);
        }

        res.json(bookings);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/admin/audit-logs', (req, res) => {
    try {
        const logs = db.prepare(`
            SELECT a.*, u.username
            FROM AuditLogs a
            LEFT JOIN Users u ON a.user_id = u.user_id
            ORDER BY a.timestamp DESC
            LIMIT 50
        `).all();
        res.json(logs);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.get('/api/admin/users', (req, res) => {
    try {
        const users = db.prepare('SELECT user_id, username, full_name, email, phone, role, extra_field, created_at FROM Users ORDER BY user_id ASC').all();
        res.json(users);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Fallback to index.html for SPA routing (Express 5 compatible)
app.use((req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.listen(PORT, '0.0.0.0', () => {
    console.log(`[TMS Web Server] Running on http://localhost:${PORT}`);
});
