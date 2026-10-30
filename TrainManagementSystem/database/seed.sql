-- ============================================================================
-- Train Management System (TMS) - Karnataka Railway Network Seed Data
-- 100% verified stations located exclusively in Karnataka, India
-- Zones: South Western Railway (SWR), Konkan Railway (KR), Southern Railway (SR),
--        Central Railway (CR), South Central Railway (SCR)
-- ============================================================================

-- 1. Users (Standard SHA-256 hashed passwords)
-- Admin: admin / Admin@123
-- Operator: operator1 / Oper@123
-- Passenger: passenger1 / Pass@123
INSERT OR REPLACE INTO Users (user_id, username, password_hash, full_name, email, phone, role, extra_field)
VALUES 
(1, 'admin', 'e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7', 'Chief Administrator', 'admin@karnatakarail.gov.in', '9876543210', 'ADMIN', 'SWR Rail Soudha, Hubballi'),
(2, 'operator1', '67efb5894b59fc3487c95e0c5112e5296c0953a8fe620cb29a43a0e698885b54', 'Ramesh Rao', 'operator.bengaluru@karnatakarail.gov.in', '9876543211', 'OPERATOR', 'Bengaluru Division'),
(3, 'passenger1', '7ea0dbb38a7b3db2ad51a02102553b9f8749a5b341f211516f1c4df1ec7cb232', 'Praveen Kumar', 'praveen.k@gmail.com', '9876543212', 'PASSENGER', 'Jayanagar 4th Block, Bengaluru');

-- 2. Karnataka Railway Stations (Strictly in Karnataka, official codes, divisions, and coordinates)
INSERT OR REPLACE INTO Stations (station_id, code, name, city, district, state, railway_zone, railway_division, platforms_count, latitude, longitude)
VALUES
-- Bengaluru Division and Surrounding Area (SWR)
(1, 'SBC', 'KSR Bengaluru City', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 10, 12.9784, 77.5694),
(2, 'YPR', 'Yesvantpur Junction', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 6, 13.0238, 77.5503),
(3, 'BNC', 'Bengaluru Cantt', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 12.9936, 77.5983),
(4, 'KJM', 'Krishnarajapuram', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 4, 13.0007, 77.6756),
(5, 'WFD', 'Whitefield', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 4, 12.9959, 77.7609),
(6, 'SMVB', 'SMVT Bengaluru', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 7, 13.0076, 77.6521),
(7, 'YNK', 'Yelahanka Junction', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 5, 13.1007, 77.5963),
(8, 'CSDR', 'Channasandra', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 2, 13.0336, 77.6748),
(9, 'BAND', 'Banaswadi', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 13.0117, 77.6401),
(10, 'HLE', 'Heelalige', 'Chandapura', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 2, 12.8252, 77.7287),
(11, 'AEK', 'Anekal Road', 'Anekal', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 2, 12.7214, 77.6974),
(12, 'KGI', 'Kengeri', 'Bengaluru', 'Bengaluru Urban', 'Karnataka', 'South Western Railway', 'Bengaluru', 4, 12.9126, 77.4815),
(13, 'RMGM', 'Ramanagara', 'Ramanagara', 'Ramanagara', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 12.7213, 77.2831),
(14, 'CPT', 'Channapatna', 'Channapatna', 'Ramanagara', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 12.6517, 77.2028),
(15, 'MAD', 'Maddur', 'Maddur', 'Mandya', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 12.5843, 77.0468),
(16, 'MYA', 'Mandya', 'Mandya', 'Mandya', 'Karnataka', 'South Western Railway', 'Bengaluru', 4, 12.5256, 76.8953),
(17, 'TK', 'Tumakuru', 'Tumakuru', 'Tumakuru', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 13.3444, 77.1017),
(18, 'GBB', 'Gubbi', 'Gubbi', 'Tumakuru', 'Karnataka', 'South Western Railway', 'Bengaluru', 2, 13.3134, 76.9405),
(19, 'TTR', 'Tiptur', 'Tiptur', 'Tumakuru', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 13.2575, 76.4789),
(20, 'KIGL', 'Kunigal', 'Kunigal', 'Tumakuru', 'Karnataka', 'South Western Railway', 'Bengaluru', 2, 12.9961, 77.0428),
(21, 'CBP', 'Chikkaballapur', 'Chikkaballapur', 'Chikkaballapur', 'Karnataka', 'South Western Railway', 'Bengaluru', 3, 13.4325, 77.7275),
(22, 'BWT', 'Bangarapet Junction', 'Bangarapet', 'Kolar', 'Karnataka', 'South Western Railway', 'Bengaluru', 5, 12.9814, 78.1963),
(23, 'KQZ', 'Kolar', 'Kolar', 'Kolar', 'Karnataka', 'South Western Railway', 'Bengaluru', 2, 13.1438, 78.1347),
(24, 'MLO', 'Malur', 'Malur', 'Kolar', 'Karnataka', 'South Western Railway', 'Bengaluru', 2, 13.0041, 77.9392),

-- Mysuru and Hassan Region (SWR - Mysuru Division)
(25, 'MYS', 'Mysuru Junction', 'Mysuru', 'Mysuru', 'Karnataka', 'South Western Railway', 'Mysuru', 6, 12.3160, 76.6436),
(26, 'CMNR', 'Chamarajanagar', 'Chamarajanagar', 'Chamarajanagar', 'Karnataka', 'South Western Railway', 'Mysuru', 2, 11.9261, 76.9427),
(27, 'HAS', 'Hassan Junction', 'Hassan', 'Hassan', 'Karnataka', 'South Western Railway', 'Mysuru', 5, 13.0068, 76.1158),
(28, 'SKLR', 'Sakleshpur', 'Sakleshpur', 'Hassan', 'Karnataka', 'South Western Railway', 'Mysuru', 3, 12.9734, 75.7876),
(29, 'SBHR', 'Subrahmanya Road', 'Subrahmanya', 'Dakshina Kannada', 'Karnataka', 'South Western Railway', 'Mysuru', 3, 12.6711, 75.5683),
(30, 'ASK', 'Arsikere Junction', 'Arsikere', 'Hassan', 'Karnataka', 'South Western Railway', 'Mysuru', 4, 13.3134, 76.2570),
(31, 'DRU', 'Kadur Junction', 'Kadur', 'Chikkamagaluru', 'Karnataka', 'South Western Railway', 'Mysuru', 3, 13.5518, 76.0125),
(32, 'RRB', 'Birur Junction', 'Birur', 'Chikkamagaluru', 'Karnataka', 'South Western Railway', 'Mysuru', 4, 13.6067, 75.9754),
(33, 'CMGR', 'Chikkamagaluru', 'Chikkamagaluru', 'Chikkamagaluru', 'Karnataka', 'South Western Railway', 'Mysuru', 2, 13.3262, 75.7725),
(34, 'TKE', 'Tarikere', 'Tarikere', 'Chikkamagaluru', 'Karnataka', 'South Western Railway', 'Mysuru', 2, 13.7142, 75.8164),
(35, 'BDVT', 'Bhadravati', 'Bhadravati', 'Shivamogga', 'Karnataka', 'South Western Railway', 'Mysuru', 3, 13.8407, 75.7047),
(36, 'SMET', 'Shivamogga Town', 'Shivamogga', 'Shivamogga', 'Karnataka', 'South Western Railway', 'Mysuru', 4, 13.9326, 75.5672),
(37, 'DVG', 'Davangere', 'Davangere', 'Davangere', 'Karnataka', 'South Western Railway', 'Mysuru', 3, 14.4644, 75.9218),
(38, 'HRR', 'Harihar', 'Harihar', 'Davangere', 'Karnataka', 'South Western Railway', 'Mysuru', 3, 14.5165, 75.8037),
(39, 'CTA', 'Chitradurga', 'Chitradurga', 'Chitradurga', 'Karnataka', 'South Western Railway', 'Mysuru', 2, 14.2333, 76.4000),
(40, 'HSD', 'Hosadurga Road', 'Hosadurga', 'Chitradurga', 'Karnataka', 'South Western Railway', 'Mysuru', 2, 13.8012, 76.2891),
(41, 'CHKE', 'Challakere', 'Challakere', 'Chitradurga', 'Karnataka', 'South Western Railway', 'Mysuru', 2, 14.3167, 76.6500),

-- Hubballi and North Karnataka (SWR - Hubballi Division)
(42, 'UBL', 'SSS Hubballi Junction', 'Hubballi', 'Dharwad', 'Karnataka', 'South Western Railway', 'Hubballi', 8, 15.3486, 75.1481),
(43, 'DWR', 'Dharwad', 'Dharwad', 'Dharwad', 'Karnataka', 'South Western Railway', 'Hubballi', 3, 15.4674, 75.0065),
(44, 'LWR', 'Alnavar Junction', 'Alnavar', 'Dharwad', 'Karnataka', 'South Western Railway', 'Hubballi', 3, 15.4339, 74.7351),
(45, 'GDG', 'Gadag Junction', 'Gadag', 'Gadag', 'Karnataka', 'South Western Railway', 'Hubballi', 4, 15.4285, 75.6324),
(46, 'HVR', 'Haveri', 'Haveri', 'Haveri', 'Karnataka', 'South Western Railway', 'Hubballi', 3, 14.7964, 75.4024),
(47, 'RNR', 'Ranibennur', 'Ranibennur', 'Haveri', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 14.6225, 75.6268),
(48, 'BGM', 'Belagavi', 'Belagavi', 'Belagavi', 'Karnataka', 'South Western Railway', 'Hubballi', 4, 15.8497, 74.5089),
(49, 'KNP', 'Khanapur', 'Khanapur', 'Belagavi', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 15.6371, 74.5147),
(50, 'GKK', 'Gokak Road', 'Gokak', 'Belagavi', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 16.1667, 74.8333),
(51, 'LD', 'Londa Junction', 'Londa', 'Belagavi', 'Karnataka', 'South Western Railway', 'Hubballi', 3, 15.4526, 74.5122),
(52, 'BGK', 'Bagalkot', 'Bagalkot', 'Bagalkot', 'Karnataka', 'South Western Railway', 'Hubballi', 3, 16.1812, 75.6984),
(53, 'BDM', 'Badami', 'Badami', 'Bagalkot', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 15.9187, 75.6766),
(54, 'LMT', 'Almatti', 'Almatti', 'Vijayapura', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 16.3314, 75.8856),
(55, 'BJP', 'Vijayapura', 'Vijayapura', 'Vijayapura', 'Karnataka', 'South Western Railway', 'Hubballi', 3, 16.8302, 75.7100),
(56, 'KBL', 'Koppal', 'Koppal', 'Koppal', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 15.3458, 76.1553),
(57, 'GGVT', 'Gangavathi', 'Gangavathi', 'Koppal', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 15.4312, 76.5312),
(58, 'MRB', 'Munirabad', 'Munirabad', 'Koppal', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 15.3087, 76.3289),
(59, 'HPT', 'Hosapete Junction', 'Hosapete', 'Vijayanagara', 'Karnataka', 'South Western Railway', 'Hubballi', 4, 15.2689, 76.3909),
(60, 'TNGL', 'Toranagallu', 'Toranagallu', 'Ballari', 'Karnataka', 'South Western Railway', 'Hubballi', 2, 15.2014, 76.6625),
(61, 'BAY', 'Ballari Junction', 'Ballari', 'Ballari', 'Karnataka', 'South Western Railway', 'Hubballi', 4, 15.1472, 76.9214),

-- Coastal Karnataka (Southern Railway & Konkan Railway)
(62, 'MAQ', 'Mangaluru Central', 'Mangaluru', 'Dakshina Kannada', 'Karnataka', 'Southern Railway', 'Palakkad', 4, 12.8634, 74.8436),
(63, 'MAJN', 'Mangaluru Junction', 'Mangaluru', 'Dakshina Kannada', 'Karnataka', 'Southern Railway', 'Palakkad', 3, 12.8711, 74.8762),
(64, 'SL', 'Surathkal', 'Surathkal', 'Dakshina Kannada', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 13.0118, 74.7928),
(65, 'UD', 'Udupi', 'Udupi', 'Udupi', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 13.3409, 74.7421),
(66, 'KUDA', 'Kundapura', 'Kundapura', 'Udupi', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 13.6277, 74.6934),
(67, 'BYNR', 'Byndoor Mookambika Road', 'Byndoor', 'Udupi', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 13.8741, 74.6312),
(68, 'BTJL', 'Bhatkal', 'Bhatkal', 'Uttara Kannada', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 13.9789, 74.5511),
(69, 'HNA', 'Honnavar', 'Honnavar', 'Uttara Kannada', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 14.2819, 74.4447),
(70, 'KT', 'Kumta', 'Kumta', 'Uttara Kannada', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 14.4262, 74.4187),
(71, 'GOK', 'Gokarna Road', 'Gokarna', 'Uttara Kannada', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 14.5367, 74.3792),
(72, 'ANKL', 'Ankola', 'Ankola', 'Uttara Kannada', 'Karnataka', 'Konkan Railway', 'Karwar', 2, 14.6547, 74.3014),
(73, 'KAWR', 'Karwar', 'Karwar', 'Uttara Kannada', 'Karnataka', 'Konkan Railway', 'Karwar', 3, 14.8185, 74.1481),

-- Hyderabad-Karnataka / Kalyana Karnataka Region (Central Railway & South Central Railway)
(74, 'KLBG', 'Kalaburagi', 'Kalaburagi', 'Kalaburagi', 'Karnataka', 'Central Railway', 'Solapur', 4, 17.3364, 76.8378),
(75, 'SDB', 'Shahabad', 'Shahabad', 'Kalaburagi', 'Karnataka', 'Central Railway', 'Solapur', 3, 17.1311, 76.9389),
(76, 'WADI', 'Wadi Junction', 'Wadi', 'Kalaburagi', 'Karnataka', 'Central Railway', 'Solapur', 4, 17.0542, 76.9922),
(77, 'SEM', 'Sedam', 'Sedam', 'Kalaburagi', 'Karnataka', 'South Central Railway', 'Secunderabad', 2, 17.1812, 77.2912),
(78, 'CT', 'Chitapur', 'Chitapur', 'Kalaburagi', 'Karnataka', 'South Central Railway', 'Secunderabad', 2, 17.1189, 77.0867),
(79, 'YG', 'Yadgir', 'Yadgir', 'Yadgir', 'Karnataka', 'South Central Railway', 'Guntakal', 3, 16.7689, 77.1389),
(80, 'RC', 'Raichur', 'Raichur', 'Raichur', 'Karnataka', 'South Central Railway', 'Guntakal', 3, 16.2076, 77.3556),
(81, 'BIDR', 'Bidar', 'Bidar', 'Bidar', 'Karnataka', 'South Central Railway', 'Secunderabad', 3, 17.9104, 77.5199);

-- 3. Realistic Karnataka Train Routes
-- Route 1: KSR Bengaluru (SBC) - Mysuru (MYS) (138 km)
INSERT OR REPLACE INTO Routes (route_id, route_name, source_station_id, dest_station_id, total_distance_km)
VALUES
(1, 'Bengaluru - Mysuru Kaveri Line', 1, 25, 138.0),
(2, 'Bengaluru - Hubballi - Belagavi Chennamma Corridor', 1, 48, 611.0),
(3, 'SMVT Bengaluru - Mangaluru Central Netravati Route', 6, 62, 355.0),
(4, 'Bengaluru - Shivamogga Malnad Express Route', 1, 36, 268.0),
(5, 'Mangaluru Central - Karwar Konkan Coastal Line', 62, 73, 260.0),
(6, 'Bengaluru - Kalaburagi Express Route', 1, 74, 570.0);

-- Route Stations for Route 1 (Bengaluru -> Kengeri -> Ramanagara -> Channapatna -> Maddur -> Mandya -> Mysuru)
INSERT OR REPLACE INTO RouteStations (route_station_id, route_id, station_id, stop_sequence, distance_from_start_km, halt_minutes)
VALUES
(1, 1, 1, 1, 0.0, 0),
(2, 1, 12, 2, 12.0, 2),
(3, 1, 13, 3, 45.0, 2),
(4, 1, 14, 4, 56.0, 2),
(5, 1, 15, 5, 74.0, 2),
(6, 1, 16, 6, 93.0, 3),
(7, 1, 25, 7, 138.0, 0);

-- Route Stations for Route 2 (Bengaluru -> Yesvantpur -> Tumakuru -> Tiptur -> Arsikere -> Davangere -> Haveri -> Hubballi -> Dharwad -> Londa -> Belagavi)
INSERT OR REPLACE INTO RouteStations (route_station_id, route_id, station_id, stop_sequence, distance_from_start_km, halt_minutes)
VALUES
(8, 2, 1, 1, 0.0, 0),
(9, 2, 2, 2, 6.0, 2),
(10, 2, 17, 3, 70.0, 3),
(11, 2, 19, 4, 141.0, 2),
(12, 2, 30, 5, 166.0, 5),
(13, 2, 37, 6, 326.0, 5),
(14, 2, 46, 7, 395.0, 2),
(15, 2, 42, 8, 470.0, 10),
(16, 2, 43, 9, 490.0, 3),
(17, 2, 51, 10, 550.0, 2),
(18, 2, 48, 11, 611.0, 0);

-- Route Stations for Route 3 (SMVT Bengaluru -> Yesvantpur -> Kunigal -> Hassan -> Sakleshpur -> Subrahmanya Road -> Mangaluru Jn -> Mangaluru Central)
INSERT OR REPLACE INTO RouteStations (route_station_id, route_id, station_id, stop_sequence, distance_from_start_km, halt_minutes)
VALUES
(19, 3, 6, 1, 0.0, 0),
(20, 3, 2, 2, 15.0, 5),
(21, 3, 20, 3, 70.0, 2),
(22, 3, 27, 4, 180.0, 5),
(23, 3, 28, 5, 222.0, 10),
(24, 3, 29, 6, 275.0, 5),
(25, 3, 63, 7, 349.0, 3),
(26, 3, 62, 8, 355.0, 0);

-- Route Stations for Route 4 (Bengaluru -> Yesvantpur -> Tumakuru -> Arsikere -> Tarikere -> Bhadravati -> Shivamogga)
INSERT OR REPLACE INTO RouteStations (route_station_id, route_id, station_id, stop_sequence, distance_from_start_km, halt_minutes)
VALUES
(27, 4, 1, 1, 0.0, 0),
(28, 4, 2, 2, 6.0, 2),
(29, 4, 17, 3, 70.0, 3),
(30, 4, 30, 4, 166.0, 5),
(31, 4, 34, 5, 220.0, 2),
(32, 4, 35, 6, 241.0, 2),
(33, 4, 36, 7, 268.0, 0);

-- Route Stations for Route 5 (Mangaluru Central -> Surathkal -> Udupi -> Kundapura -> Byndoor -> Bhatkal -> Honnavar -> Kumta -> Gokarna -> Ankola -> Karwar)
INSERT OR REPLACE INTO RouteStations (route_station_id, route_id, station_id, stop_sequence, distance_from_start_km, halt_minutes)
VALUES
(34, 5, 62, 1, 0.0, 0),
(35, 5, 64, 2, 16.0, 2),
(36, 5, 65, 3, 58.0, 3),
(37, 5, 66, 4, 90.0, 2),
(38, 5, 67, 5, 124.0, 2),
(39, 5, 68, 6, 139.0, 2),
(40, 5, 69, 7, 180.0, 2),
(41, 5, 70, 8, 194.0, 2),
(42, 5, 71, 9, 213.0, 2),
(43, 5, 72, 10, 221.0, 2),
(44, 5, 73, 11, 260.0, 0);

-- 4. Karnataka Trains
INSERT OR REPLACE INTO Trains (train_id, train_number, name, type, source_station_id, dest_station_id, route_id, base_fare_per_km, is_active)
VALUES
(1, '20607', 'Vande Bharat Express', 'VANDE_BHARAT', 1, 25, 1, 0.85, 1),
(2, '12079', 'Siddhaganga Intercity Express', 'SUPERFAST', 1, 42, 2, 0.55, 1),
(3, '16589', 'Rani Chennamma Express', 'EXPRESS', 1, 48, 2, 0.50, 1),
(4, '16515', 'Karwar Express', 'EXPRESS', 2, 73, 3, 0.55, 1),
(5, '12065', 'Jan Shatabdi Express', 'SUPERFAST', 1, 36, 4, 0.60, 1),
(6, '16595', 'Panchaganga SF Express', 'SUPERFAST', 1, 73, 5, 0.60, 1);

-- 5. Karnataka Train Schedules
INSERT OR REPLACE INTO Schedules (schedule_id, train_id, departure_time, arrival_time, operating_days, delay_minutes, is_overnight)
VALUES
(1, 1, '05:45', '07:30', 'Wed,Thu,Fri,Sat,Sun,Mon', 0, 0),
(2, 2, '06:00', '13:00', 'Daily', 0, 0),
(3, 3, '23:00', '08:15', 'Daily', 5, 1),
(4, 4, '07:00', '18:45', 'Mon,Wed,Fri', 0, 0),
(5, 5, '17:15', '21:40', 'Mon,Tue,Wed,Thu,Fri,Sat', 0, 0),
(6, 6, '18:50', '08:25', 'Daily', 0, 1);

-- 6. Karnataka Train Coaches
-- Train 1: Vande Bharat (CC, EC)
INSERT OR REPLACE INTO Coaches (coach_id, train_id, coach_code, coach_class, total_seats)
VALUES
(1, 1, 'C1', 'CC', 78),
(2, 1, 'C2', 'CC', 78),
(3, 1, 'E1', 'EC', 52);

-- Train 2: Siddhaganga (CC, 2S/SL)
INSERT OR REPLACE INTO Coaches (coach_id, train_id, coach_code, coach_class, total_seats)
VALUES
(4, 2, 'C1', 'CC', 78),
(5, 2, 'S1', 'SL', 72),
(6, 2, 'S2', 'SL', 72);

-- Train 3: Rani Chennamma (1A, 2A, 3A, SL)
INSERT OR REPLACE INTO Coaches (coach_id, train_id, coach_code, coach_class, total_seats)
VALUES
(7, 3, 'H1', '1A', 24),
(8, 3, 'A1', '2A', 48),
(9, 3, 'B1', '3A', 64),
(10, 3, 'S1', 'SL', 72);

-- 7. Sample Seats for Coaches
-- B1 (3A) for Rani Chennamma
INSERT OR REPLACE INTO Seats (coach_id, seat_number, berth_type) VALUES
(9, 1, 'Lower'), (9, 2, 'Middle'), (9, 3, 'Upper'),
(9, 4, 'Lower'), (9, 5, 'Middle'), (9, 6, 'Upper'),
(9, 7, 'Side Lower'), (9, 8, 'Side Upper'),
(9, 9, 'Lower'), (9, 10, 'Middle'), (9, 11, 'Upper'),
(9, 12, 'Lower'), (9, 13, 'Middle'), (9, 14, 'Upper'),
(9, 15, 'Side Lower'), (9, 16, 'Side Upper');

-- C1 (CC) for Vande Bharat
INSERT OR REPLACE INTO Seats (coach_id, seat_number, berth_type) VALUES
(1, 1, 'Window'), (1, 2, 'Aisle'), (1, 3, 'Middle'), (1, 4, 'Aisle'), (1, 5, 'Window'),
(1, 6, 'Window'), (1, 7, 'Aisle'), (1, 8, 'Middle'), (1, 9, 'Aisle'), (1, 10, 'Window');
