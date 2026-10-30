#include "DatabaseManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace tms {

DatabaseManager::DatabaseManager() : m_db(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

void DatabaseManager::open(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db != nullptr) {
        if (m_dbPath == dbPath) return;
        sqlite3_close(m_db);
        m_db = nullptr;
    }

    m_dbPath = dbPath;
    int rc = sqlite3_open(dbPath.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(m_db);
        m_db = nullptr;
        throw DatabaseException("Failed to open SQLite database: " + err);
    }

    // Enable foreign keys
    char* zErrMsg = nullptr;
    rc = sqlite3_exec(m_db, "PRAGMA foreign_keys = ON; PRAGMA journal_mode = WAL;", nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::string err = zErrMsg ? zErrMsg : "Unknown error";
        sqlite3_free(zErrMsg);
        throw DatabaseException("Failed to set PRAGMAs: " + err);
    }
}

void DatabaseManager::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db != nullptr) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void DatabaseManager::executeScript(const std::string& sql) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) throw DatabaseException("Database not opened");

    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::string err = zErrMsg ? zErrMsg : "Unknown SQL error";
        sqlite3_free(zErrMsg);
        throw DatabaseException("Failed executing SQL script: " + err);
    }
}

void DatabaseManager::executeFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        file.open("../" + filepath);
    }
    if (!file.is_open()) {
        throw DatabaseException("Could not open file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    executeScript(buffer.str());
}

int DatabaseManager::executeNonQuery(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) throw DatabaseException("Database not opened");

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(m_db));
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::string err = sqlite3_errmsg(m_db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Error executing non-query: " + err);
    }

    int changes = sqlite3_changes(m_db);
    sqlite3_finalize(stmt);
    return changes;
}

void DatabaseManager::executeQuery(const std::string& sql, const RowCallback& callback, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) throw DatabaseException("Database not opened");

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseException(std::string("Failed to prepare query statement: ") + sqlite3_errmsg(m_db));
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    int cols = sqlite3_column_count(stmt);
    std::vector<std::string> colNames;
    colNames.reserve(static_cast<size_t>(cols));
    for (int i = 0; i < cols; ++i) {
        const char* name = sqlite3_column_name(stmt, i);
        colNames.emplace_back(name ? name : "");
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector<std::string> rowValues;
        rowValues.reserve(static_cast<size_t>(cols));
        for (int i = 0; i < cols; ++i) {
            const unsigned char* val = sqlite3_column_text(stmt, i);
            rowValues.emplace_back(val ? reinterpret_cast<const char*>(val) : "");
        }
        if (!callback(rowValues, colNames)) {
            break; // Callback requested aborting further iteration
        }
    }

    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::string err = sqlite3_errmsg(m_db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Query execution error: " + err);
    }

    sqlite3_finalize(stmt);
}

int64_t DatabaseManager::getLastInsertedRowId() const {
    return sqlite3_last_insert_rowid(m_db);
}

// ---------------- ScopedTransaction ----------------
DatabaseManager::ScopedTransaction::ScopedTransaction(DatabaseManager& db)
    : m_dbMgr(db), m_committed(false) {
    m_dbMgr.executeNonQuery("BEGIN IMMEDIATE TRANSACTION;");
}

DatabaseManager::ScopedTransaction::~ScopedTransaction() {
    if (!m_committed) {
        try {
            rollback();
        } catch (...) {
            // Suppress exception in destructor
        }
    }
}

void DatabaseManager::ScopedTransaction::commit() {
    if (!m_committed) {
        m_dbMgr.executeNonQuery("COMMIT;");
        m_committed = true;
    }
}

void DatabaseManager::ScopedTransaction::rollback() {
    if (!m_committed) {
        m_dbMgr.executeNonQuery("ROLLBACK;");
        m_committed = true;
    }
}

} // namespace tms
