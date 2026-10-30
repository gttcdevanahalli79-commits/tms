# Train Management System (TMS) – C++ Application

An enterprise-grade, modular, menu-driven Train Management System engineered in modern C++ (C++17/20) with robust relational database persistence (SQLite3), transaction-safe concurrency, role-based access control (RBAC), and Google Test automation.

## Architecture Highlights
- **Language Standard**: C++17 / C++20 compliant with strict compiler warnings.
- **Relational Persistence**: SQLite3 with foreign keys, acid transactions, prepared statements, and parameterized queries.
- **Object-Oriented Design**: Pure abstract base classes, polymorphic roles (Admin, Operator, Passenger), encapsulation, RAII memory management via `std::unique_ptr` and `std::shared_ptr`.
- **Modern Build System**: CMake 3.16+ with modular static library (`tms_core`), application binary (`tms_app`), and unit testing binary (`tms_tests`).
- **Automated Testing**: Google Test test suites integrating with CTest.

## Directory Structure
```
TrainManagementSystem/
│
├── include/
│   ├── Common.h
│   ├── User.h
│   ├── Train.h
│   ├── Station.h
│   ├── Route.h
│   ├── Schedule.h
│   ├── Coach.h
│   ├── Seat.h
│   ├── Booking.h
│   ├── Payment.h
│   ├── Refund.h
│   └── Managers/
│       ├── DatabaseManager.h
│       ├── TrainManager.h
│       ├── BookingManager.h
│       ├── PaymentManager.h
│       ├── AuthenticationManager.h
│       ├── ReportManager.h
│       └── ReservationSystem.h
│
├── src/
│   ├── main.cpp
│   ├── User.cpp
│   ├── Train.cpp
│   ├── Station.cpp
│   ├── Booking.cpp
│   ├── Payment.cpp
│   └── Managers/
│       └── DatabaseManager.cpp
│
├── database/
│   ├── schema.sql
│   └── seed.sql
│
├── tests/
│   ├── test_main.cpp
│   └── test_phase1.cpp
│
├── CMakeLists.txt
└── README.md
```

## Compilation and Execution

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler (GCC 9+ or Clang 10+)
- SQLite3 development libraries (`libsqlite3-dev`)
- Google Test (auto-downloaded via CMake `FetchContent` if not present system-wide)

### Build Commands
```bash
# Navigate to the project directory
cd TrainManagementSystem

# Create and navigate to the build directory
mkdir -p build && cd build

# Configure CMake in Release or Debug mode
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build all targets (application, library, and test suite)
make -j$(nproc)
```

### Running the Application
```bash
./bin/tms_app
```

### Running the Unit Tests
```bash
./bin/tms_tests
# Alternatively using ctest:
ctest --output-on-failure
```
