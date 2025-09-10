// File: include/DatabaseManager.h
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <memory>
#include <vector>

class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;
    static DatabaseManager* instance;
    
    // Private constructor for Singleton pattern
    DatabaseManager(const std::string& dbPath);
    
    // Create all database tables
    bool createTables();
    
public:
    // Singleton access
    static DatabaseManager* getInstance(const std::string& dbPath = "data/gym_management.db");
    
    // Destructor
    ~DatabaseManager();
    
    // Database connection management
    bool connect();
    void disconnect();
    
    // Query execution
    bool executeQuery(const std::string& query);
    sqlite3_stmt* prepareStatement(const std::string& query);
    
    // Transaction management
    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();
    
    // Utility functions
    bool tableExists(const std::string& tableName);
    
    // Database access
    sqlite3* getDatabase() const { return db; }
    
    // Delete copy constructor and assignment operator (Singleton)
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool isConnected() const { return db != nullptr; }
};

#endif // DATABASE_MANAGER_H