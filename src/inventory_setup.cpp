// File: src/inventory_setup.cpp
// This file contains the inventory table creation code for DatabaseManager
// Add this to your DatabaseManager.cpp in the setupTables() function

/*
Add this to your DatabaseManager::setupTables() function:

// Create inventory table
std::string createInventoryTable = R"(
    CREATE TABLE IF NOT EXISTS inventory (
        item_id INTEGER PRIMARY KEY AUTOINCREMENT,
        item_name TEXT NOT NULL,
        category TEXT NOT NULL CHECK(category IN ('equipment', 'cleaning', 'maintenance', 'drinks', 'supplements', 'other')),
        quantity INTEGER NOT NULL DEFAULT 0,
        cost_price REAL DEFAULT 0.0,
        selling_price REAL DEFAULT 0.0,
        supplier TEXT,
        date_acquired INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
        last_updated INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
        notes TEXT,
        condition TEXT DEFAULT 'excellent' CHECK(condition IN ('excellent', 'good', 'fair', 'poor', 'out_of_order')),
        last_maintenance INTEGER DEFAULT 0,
        expiry_date INTEGER DEFAULT 0,
        original_quantity INTEGER DEFAULT 0,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)";

if (!executeQuery(createInventoryTable)) {
    std::cerr << "Error creating inventory table" << std::endl;
    return false;
}

// Create inventory history table for tracking changes
std::string createInventoryHistoryTable = R"(
    CREATE TABLE IF NOT EXISTS inventory_history (
        history_id INTEGER PRIMARY KEY AUTOINCREMENT,
        item_id INTEGER NOT NULL,
        action TEXT NOT NULL,
        old_quantity INTEGER,
        new_quantity INTEGER,
        old_condition TEXT,
        new_condition TEXT,
        notes TEXT,
        action_date INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
        FOREIGN KEY (item_id) REFERENCES inventory(item_id) ON DELETE CASCADE
    );
)";

if (!executeQuery(createInventoryHistoryTable)) {
    std::cerr << "Error creating inventory history table" << std::endl;
    return false;
}

// Create indexes for better performance
std::string createInventoryIndexes = R"(
    CREATE INDEX IF NOT EXISTS idx_inventory_category ON inventory(category);
    CREATE INDEX IF NOT EXISTS idx_inventory_name ON inventory(item_name);
    CREATE INDEX IF NOT EXISTS idx_inventory_quantity ON inventory(quantity);
    CREATE INDEX IF NOT EXISTS idx_inventory_expiry ON inventory(expiry_date);
    CREATE INDEX IF NOT EXISTS idx_inventory_maintenance ON inventory(last_maintenance);
)";

if (!executeQuery(createInventoryIndexes)) {
    std::cerr << "Error creating inventory indexes" << std::endl;
    return false;
}

*/