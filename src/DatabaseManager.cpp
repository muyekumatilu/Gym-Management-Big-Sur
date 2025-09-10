// File: src/DatabaseManager.cpp
#include "DatabaseManager.h"
#include <iostream>
#include <cstring>

// Static instance for Singleton pattern
DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager(const std::string& dbPath) : db(nullptr), dbPath(dbPath) {
    // Constructor is private for Singleton pattern
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

DatabaseManager* DatabaseManager::getInstance(const std::string& dbPath) {
    if (instance == nullptr) {
        instance = new DatabaseManager(dbPath);
    }
    return instance;
}

bool DatabaseManager::connect() {
    int result = sqlite3_open(dbPath.c_str(), &db);
    
    if (result != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    std::cout << "Database connected successfully: " << dbPath << std::endl;
    
    // Enable foreign key constraints
    executeQuery("PRAGMA foreign_keys = ON;");
    
    // Create tables if they don't exist
    if (!createTables()) {
        std::cerr << "Failed to create database tables" << std::endl;
        return false;
    }
    
    return true;
}

void DatabaseManager::disconnect() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
        std::cout << "Database disconnected" << std::endl;
    }
}

bool DatabaseManager::executeQuery(const std::string& query) {
    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errorMessage);
    
    if (result != SQLITE_OK) {
        std::cerr << "SQL Error: " << errorMessage << std::endl;
        std::cerr << "Query: " << query << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }
    
    return true;
}

sqlite3_stmt* DatabaseManager::prepareStatement(const std::string& query) {
    sqlite3_stmt* statement = nullptr;
    int result = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, nullptr);
    
    if (result != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        std::cerr << "Query: " << query << std::endl;
        return nullptr;
    }
    
    return statement;
}

bool DatabaseManager::createTables() {
    // Create customers table
    std::string createCustomersTable = R"(
        CREATE TABLE IF NOT EXISTS customers (
            customer_id INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            email TEXT,
            phone TEXT NOT NULL,
            membership_type TEXT NOT NULL CHECK(membership_type IN ('daily', 'weekly', 'monthly', 'trainer')),
            join_date INTEGER NOT NULL,
            expiry_date INTEGER,
            last_payment_date INTEGER NOT NULL,
            balance REAL DEFAULT 0.0,
            created_at INTEGER DEFAULT (strftime('%s', 'now')),
            updated_at INTEGER DEFAULT (strftime('%s', 'now'))
        );
    )";
    
    if (!executeQuery(createCustomersTable)) {
        return false;
    }
    
    // Create check_ins table
    std::string createCheckInsTable = R"(
        CREATE TABLE IF NOT EXISTS check_ins (
            check_in_id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_id INTEGER NOT NULL,
            check_in_time INTEGER NOT NULL,
            check_out_time INTEGER,
            created_at INTEGER DEFAULT (strftime('%s', 'now')),
            FOREIGN KEY (customer_id) REFERENCES customers (customer_id)
        );
    )";
    
    if (!executeQuery(createCheckInsTable)) {
        return false;
    }
    
    // Create transactions table
    std::string createTransactionsTable = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_id INTEGER NOT NULL,
            amount REAL NOT NULL,
            payment_type TEXT NOT NULL CHECK (payment_type IN ('cash', 'mpesa')),
            transaction_type TEXT NOT NULL CHECK (transaction_type IN ('membership_payment', 'daily_fee', 'renewal', 'refund', 'other')),
            status TEXT NOT NULL CHECK (status IN ('completed', 'pending', 'failed', 'refunded')),
            transaction_date INTEGER NOT NULL,
            description TEXT,
            mpesa_code TEXT,
            notes TEXT,
            created_at INTEGER DEFAULT (strftime('%s', 'now')),
            updated_at INTEGER DEFAULT (strftime('%s', 'now')),
            FOREIGN KEY (customer_id) REFERENCES customers(customer_id)
        )
    )";

if (!executeQuery(createTransactionsTable)) {
    return false;
}
    
    // Create inventory table with updated schema
    std::string createInventoryTable = R"(
        CREATE TABLE IF NOT EXISTS inventory (
            item_id INTEGER PRIMARY KEY AUTOINCREMENT,
            item_name TEXT NOT NULL,
            category TEXT NOT NULL CHECK(category IN ('equipment', 'cleaning', 'maintenance', 'drinks', 'supplements', 'other')),
            quantity INTEGER DEFAULT 0,
            cost_price REAL DEFAULT 0.0,
            selling_price REAL DEFAULT 0.0,
            supplier TEXT,
            date_acquired INTEGER NOT NULL,
            last_updated INTEGER DEFAULT (strftime('%s', 'now')),
            notes TEXT,
            condition TEXT CHECK(condition IN ('excellent', 'good', 'fair', 'poor', 'out_of_order')),
            last_maintenance INTEGER,
            expiry_date INTEGER,
            original_quantity INTEGER DEFAULT 0,
            created_at INTEGER DEFAULT (strftime('%s', 'now'))
        );
    )";
    
    if (!executeQuery(createInventoryTable)) {
        return false;
    }
    
    // Create indexes for better performance
    executeQuery("CREATE INDEX IF NOT EXISTS idx_customers_phone ON customers(phone);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_customers_membership ON customers(membership_type);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_checkins_customer ON check_ins(customer_id);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_checkins_date ON check_ins(check_in_time);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_transactions_customer ON transactions(customer_id);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(transaction_date);");
    
    std::cout << "Database tables created successfully" << std::endl;
    return true;
}

bool DatabaseManager::tableExists(const std::string& tableName) {
    std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt* statement = prepareStatement(query);
    
    if (statement == nullptr) {
        return false;
    }
    
    sqlite3_bind_text(statement, 1, tableName.c_str(), -1, SQLITE_STATIC);
    
    bool exists = (sqlite3_step(statement) == SQLITE_ROW);
    
    sqlite3_finalize(statement);
    return exists;
}

void DatabaseManager::beginTransaction() {
    executeQuery("BEGIN TRANSACTION;");
}

void DatabaseManager::commitTransaction() {
    executeQuery("COMMIT;");
}

void DatabaseManager::rollbackTransaction() {
    executeQuery("ROLLBACK;");
}