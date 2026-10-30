#ifndef TMS_DATABASE_MANAGER_H
#define TMS_DATABASE_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>
#include <mutex>

namespace tms {

class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& msg) : std::runtime_error(msg) {}
};

class DatabaseManager {
private:
    sqlite3* m_db;
    std::string m_dbPath;
    mutable std::mutex m_mutex;

    DatabaseManager();

public:
    ~DatabaseManager();

    // Singleton instance access
    static DatabaseManager& getInstance();

    // Prevent copy and assignment
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Database operations
    void open(const std::string& dbPath = "tms.db");
    void close();
    bool isOpen() const { return m_db != nullptr; }
    sqlite3* getRawDb() const { return m_db; }

    // Execute raw SQL script (e.g., schema.sql, seed.sql)
    void executeScript(const std::string& sql);
    void executeFile(const std::string& filepath);

    // RAII Transaction helper
    class ScopedTransaction {
    private:
        DatabaseManager& m_dbMgr;
        bool m_committed;

    public:
        explicit ScopedTransaction(DatabaseManager& db);
        ~ScopedTransaction();

        void commit();
        void rollback();
    };

    ScopedTransaction beginTransaction() {
        return ScopedTransaction(*this);
    }

    // Execute SQL with prepared statement and parameters
    int executeNonQuery(const std::string& sql, const std::vector<std::string>& params = {});
    
    // Query with callback per row: callback(vector_of_column_values, vector_of_column_names)
    using RowCallback = std::function<bool(const std::vector<std::string>& values, const std::vector<std::string>& colNames)>;
    void executeQuery(const std::string& sql, const RowCallback& callback, const std::vector<std::string>& params = {});

    int64_t getLastInsertedRowId() const;
};

} // namespace tms

#endif // TMS_DATABASE_MANAGER_H
