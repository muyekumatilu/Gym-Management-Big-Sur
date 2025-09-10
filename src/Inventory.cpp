// File: src/Inventory.cpp
#include "Inventory.h"
#include "DatabaseManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <limits>

// Constructor
Inventory::Inventory() : itemId(0), itemName(""), category(InventoryCategory::OTHER),
                        quantity(0), costPrice(0.0), sellingPrice(0.0), supplier(""),
                        dateAcquired(0), lastUpdated(0), notes(""),
                        condition(EquipmentCondition::EXCELLENT), lastMaintenance(0),
                        expiryDate(0), originalQuantity(0) {
}

Inventory::Inventory(const std::string& name, InventoryCategory cat, int qty, double cost)
    : itemId(0), itemName(name), category(cat), quantity(qty), costPrice(cost),
      sellingPrice(0.0), supplier(""), dateAcquired(std::time(nullptr)),
      lastUpdated(std::time(nullptr)), notes(""), condition(EquipmentCondition::EXCELLENT),
      lastMaintenance(0), expiryDate(0), originalQuantity(qty) {
}

Inventory::~Inventory() {
}

// Utility functions
std::string Inventory::getCategoryString() const {
    switch (category) {
        case InventoryCategory::EQUIPMENT: return "Equipment";
        case InventoryCategory::CLEANING: return "Cleaning";
        case InventoryCategory::MAINTENANCE: return "Maintenance";
        case InventoryCategory::DRINKS: return "Drinks";
        case InventoryCategory::SUPPLEMENTS: return "Supplements";
        case InventoryCategory::OTHER: return "Other";
        default: return "Unknown";
    }
}

std::string Inventory::getConditionString() const {
    switch (condition) {
        case EquipmentCondition::EXCELLENT: return "Excellent";
        case EquipmentCondition::GOOD: return "Good";
        case EquipmentCondition::FAIR: return "Fair";
        case EquipmentCondition::POOR: return "Poor";
        case EquipmentCondition::OUT_OF_ORDER: return "Out of Order";
        default: return "Unknown";
    }
}

std::string Inventory::getFormattedDate(std::time_t time) const {
    if (time == 0) return "Not set";
    
    std::tm* timeInfo = std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%Y-%m-%d %H:%M");
    return oss.str();
}

std::string Inventory::getFormattedPrice(double price) const {
    std::ostringstream oss;
    oss << "KES " << std::fixed << std::setprecision(2) << price;
    return oss.str();
}

bool Inventory::isExpired() const {
    if (expiryDate == 0) return false;
    return std::time(nullptr) > expiryDate;
}

bool Inventory::needsMaintenance(int daysSinceLastMaintenance) const {
    if (category != InventoryCategory::EQUIPMENT) return false;
    if (lastMaintenance == 0) return true; // Never maintained
    
    std::time_t now = std::time(nullptr);
    double daysDiff = std::difftime(now, lastMaintenance) / (24 * 60 * 60);
    return daysDiff > daysSinceLastMaintenance;
}

double Inventory::getUsagePercentage() const {
    if (originalQuantity == 0) return 0.0;
    return ((double)(originalQuantity - quantity) / originalQuantity) * 100.0;
}

// Database operations
bool Inventory::saveToDatabase() {
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db || !db->getDatabase()) {
        std::cerr << "Database not connected" << std::endl;
        return false;
    }
    
    std::string query = R"(
        INSERT INTO inventory (
            item_name, category, quantity, cost_price, selling_price,
            supplier, date_acquired, notes, condition, last_maintenance,
            expiry_date, original_quantity
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (!statement) return false;
    
    // Convert category to string
    std::string categoryStr;
    switch (category) {
        case InventoryCategory::EQUIPMENT: categoryStr = "equipment"; break;
        case InventoryCategory::CLEANING: categoryStr = "cleaning"; break;
        case InventoryCategory::MAINTENANCE: categoryStr = "maintenance"; break;
        case InventoryCategory::DRINKS: categoryStr = "drinks"; break;
        case InventoryCategory::SUPPLEMENTS: categoryStr = "supplements"; break;
        case InventoryCategory::OTHER: categoryStr = "other"; break;
    }
    
    // Convert condition to string
    std::string conditionStr;
    switch (condition) {
        case EquipmentCondition::EXCELLENT: conditionStr = "excellent"; break;
        case EquipmentCondition::GOOD: conditionStr = "good"; break;
        case EquipmentCondition::FAIR: conditionStr = "fair"; break;
        case EquipmentCondition::POOR: conditionStr = "poor"; break;
        case EquipmentCondition::OUT_OF_ORDER: conditionStr = "out_of_order"; break;
    }
    
    // Bind parameters
    sqlite3_bind_text(statement, 1, itemName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, categoryStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 3, quantity);
    sqlite3_bind_double(statement, 4, costPrice);
    sqlite3_bind_double(statement, 5, sellingPrice);
    sqlite3_bind_text(statement, 6, supplier.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 7, dateAcquired);
    sqlite3_bind_text(statement, 8, notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 9, conditionStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 10, lastMaintenance);
    sqlite3_bind_int64(statement, 11, expiryDate);
    sqlite3_bind_int(statement, 12, originalQuantity);
    
    int result = sqlite3_step(statement);
    
    if (result == SQLITE_DONE) {
        itemId = sqlite3_last_insert_rowid(db->getDatabase());
        lastUpdated = std::time(nullptr);
        sqlite3_finalize(statement);
        return true;
    }
    
    std::cerr << "Error saving inventory item: " << sqlite3_errmsg(db->getDatabase()) << std::endl;
    sqlite3_finalize(statement);
    return false;
}

bool Inventory::updateInDatabase() {
    if (itemId == 0) {
        std::cerr << "Cannot update: Item not saved to database yet" << std::endl;
        return false;
    }
    
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db || !db->getDatabase()) {
        std::cerr << "Database not connected" << std::endl;
        return false;
    }
    
    std::string query = R"(
        UPDATE inventory SET
            item_name = ?, category = ?, quantity = ?, cost_price = ?,
            selling_price = ?, supplier = ?, notes = ?, condition = ?,
            last_maintenance = ?, expiry_date = ?, original_quantity = ?,
            last_updated = strftime('%s', 'now')
        WHERE item_id = ?
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (!statement) return false;
    
    // Convert enums to strings
    std::string categoryStr;
    switch (category) {
        case InventoryCategory::EQUIPMENT: categoryStr = "equipment"; break;
        case InventoryCategory::CLEANING: categoryStr = "cleaning"; break;
        case InventoryCategory::MAINTENANCE: categoryStr = "maintenance"; break;
        case InventoryCategory::DRINKS: categoryStr = "drinks"; break;
        case InventoryCategory::SUPPLEMENTS: categoryStr = "supplements"; break;
        case InventoryCategory::OTHER: categoryStr = "other"; break;
    }
    
    std::string conditionStr;
    switch (condition) {
        case EquipmentCondition::EXCELLENT: conditionStr = "excellent"; break;
        case EquipmentCondition::GOOD: conditionStr = "good"; break;
        case EquipmentCondition::FAIR: conditionStr = "fair"; break;
        case EquipmentCondition::POOR: conditionStr = "poor"; break;
        case EquipmentCondition::OUT_OF_ORDER: conditionStr = "out_of_order"; break;
    }
    
    // Bind parameters
    sqlite3_bind_text(statement, 1, itemName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, categoryStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 3, quantity);
    sqlite3_bind_double(statement, 4, costPrice);
    sqlite3_bind_double(statement, 5, sellingPrice);
    sqlite3_bind_text(statement, 6, supplier.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 7, notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 8, conditionStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 9, lastMaintenance);
    sqlite3_bind_int64(statement, 10, expiryDate);
    sqlite3_bind_int(statement, 11, originalQuantity);
    sqlite3_bind_int(statement, 12, itemId);
    
    int result = sqlite3_step(statement);
    sqlite3_finalize(statement);
    
    if (result == SQLITE_DONE) {
        lastUpdated = std::time(nullptr);
        return true;
    }
    
    std::cerr << "Error updating inventory item: " << sqlite3_errmsg(db->getDatabase()) << std::endl;
    return false;
}

bool Inventory::deleteFromDatabase() {
    if (itemId == 0) {
        std::cerr << "Cannot delete: Item not in database" << std::endl;
        return false;
    }
    
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db || !db->getDatabase()) {
        std::cerr << "Database not connected" << std::endl;
        return false;
    }
    
    std::string query = "DELETE FROM inventory WHERE item_id = ?";
    sqlite3_stmt* statement = db->prepareStatement(query);
    
    if (!statement) return false;
    
    sqlite3_bind_int(statement, 1, itemId);
    
    int result = sqlite3_step(statement);
    sqlite3_finalize(statement);
    
    if (result == SQLITE_DONE) {
        std::cout << "✅ Item '" << itemName << "' deleted successfully" << std::endl;
        return true;
    }
    
    std::cerr << "Error deleting item: " << sqlite3_errmsg(db->getDatabase()) << std::endl;
    return false;
}

// Static database operations
std::vector<Inventory> Inventory::getAllItems() {
    std::vector<Inventory> items;
    DatabaseManager* db = DatabaseManager::getInstance();
    
    if (!db || !db->getDatabase()) {
        return items;
    }
    
    std::string query = "SELECT * FROM inventory ORDER BY item_name";
    sqlite3_stmt* statement = db->prepareStatement(query);
    
    if (!statement) return items;
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Inventory item;
        item.itemId = sqlite3_column_int(statement, 0);
        item.itemName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        
        // Convert category string to enum
        std::string categoryStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
        item.category = stringToCategory(categoryStr);
        
        item.quantity = sqlite3_column_int(statement, 3);
        item.costPrice = sqlite3_column_double(statement, 4);
        item.sellingPrice = sqlite3_column_double(statement, 5);
        
        const char* supplier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 6));
        item.supplier = supplier ? supplier : "";
        
        item.dateAcquired = sqlite3_column_int64(statement, 7);
        item.lastUpdated = sqlite3_column_int64(statement, 8);
        
        const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(statement, 9));
        item.notes = notes ? notes : "";
        
        // Convert condition string to enum
        const char* conditionStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (conditionStr) {
            item.condition = stringToCondition(conditionStr);
        }
        
        item.lastMaintenance = sqlite3_column_int64(statement, 11);
        item.expiryDate = sqlite3_column_int64(statement, 12);
        item.originalQuantity = sqlite3_column_int(statement, 13);
        
        items.push_back(item);
    }
    
    sqlite3_finalize(statement);
    return items;
}

std::vector<Inventory> Inventory::getItemsByCategory(InventoryCategory category) {
    std::vector<Inventory> items;
    DatabaseManager* db = DatabaseManager::getInstance();
    
    if (!db || !db->getDatabase()) {
        return items;
    }
    
    std::string categoryStr;
    switch (category) {
        case InventoryCategory::EQUIPMENT: categoryStr = "equipment"; break;
        case InventoryCategory::CLEANING: categoryStr = "cleaning"; break;
        case InventoryCategory::MAINTENANCE: categoryStr = "maintenance"; break;
        case InventoryCategory::DRINKS: categoryStr = "drinks"; break;
        case InventoryCategory::SUPPLEMENTS: categoryStr = "supplements"; break;
        case InventoryCategory::OTHER: categoryStr = "other"; break;
    }
    
    std::string query = "SELECT * FROM inventory WHERE category = ? ORDER BY item_name";
    sqlite3_stmt* statement = db->prepareStatement(query);
    
    if (!statement) return items;
    
    sqlite3_bind_text(statement, 1, categoryStr.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Inventory item;
        item.itemId = sqlite3_column_int(statement, 0);
        item.itemName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        item.category = stringToCategory(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        item.quantity = sqlite3_column_int(statement, 3);
        item.costPrice = sqlite3_column_double(statement, 4);
        item.sellingPrice = sqlite3_column_double(statement, 5);
        
        const char* supplier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 6));
        item.supplier = supplier ? supplier : "";
        
        item.dateAcquired = sqlite3_column_int64(statement, 7);
        item.lastUpdated = sqlite3_column_int64(statement, 8);
        
        const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(statement, 9));
        item.notes = notes ? notes : "";
        
        const char* conditionStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (conditionStr) {
            item.condition = stringToCondition(conditionStr);
        }
        
        item.lastMaintenance = sqlite3_column_int64(statement, 11);
        item.expiryDate = sqlite3_column_int64(statement, 12);
        item.originalQuantity = sqlite3_column_int(statement, 13);
        
        items.push_back(item);
    }
    
    sqlite3_finalize(statement);
    return items;
}

Inventory Inventory::getItemById(int itemId) {
    Inventory item;
    DatabaseManager* db = DatabaseManager::getInstance();
    
    if (!db || !db->getDatabase()) {
        return item;
    }
    
    std::string query = "SELECT * FROM inventory WHERE item_id = ?";
    sqlite3_stmt* statement = db->prepareStatement(query);
    
    if (!statement) return item;
    
    sqlite3_bind_int(statement, 1, itemId);
    
    if (sqlite3_step(statement) == SQLITE_ROW) {
        item.itemId = sqlite3_column_int(statement, 0);
        item.itemName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        item.category = stringToCategory(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        item.quantity = sqlite3_column_int(statement, 3);
        item.costPrice = sqlite3_column_double(statement, 4);
        item.sellingPrice = sqlite3_column_double(statement, 5);
        
        const char* supplier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 6));
        item.supplier = supplier ? supplier : "";
        
        item.dateAcquired = sqlite3_column_int64(statement, 7);
        item.lastUpdated = sqlite3_column_int64(statement, 8);
        
        const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(statement, 9));
        item.notes = notes ? notes : "";
        
        const char* conditionStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (conditionStr) {
            item.condition = stringToCondition(conditionStr);
        }
        
        item.lastMaintenance = sqlite3_column_int64(statement, 11);
        item.expiryDate = sqlite3_column_int64(statement, 12);
        item.originalQuantity = sqlite3_column_int(statement, 13);
    }
    
    sqlite3_finalize(statement);
    return item;
}

std::vector<Inventory> Inventory::searchItems(const std::string& searchTerm) {
    std::vector<Inventory> items;
    DatabaseManager* db = DatabaseManager::getInstance();
    
    if (!db || !db->getDatabase()) {
        return items;
    }
    
    std::string query = R"(
        SELECT * FROM inventory 
        WHERE item_name LIKE ? OR supplier LIKE ? OR notes LIKE ?
        ORDER BY item_name
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (!statement) return items;
    
    std::string searchPattern = "%" + searchTerm + "%";
    sqlite3_bind_text(statement, 1, searchPattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, searchPattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 3, searchPattern.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Inventory item;
        item.itemId = sqlite3_column_int(statement, 0);
        item.itemName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        item.category = stringToCategory(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        item.quantity = sqlite3_column_int(statement, 3);
        item.costPrice = sqlite3_column_double(statement, 4);
        item.sellingPrice = sqlite3_column_double(statement, 5);
        
        const char* supplier = reinterpret_cast<const char*>(sqlite3_column_text(statement, 6));
        item.supplier = supplier ? supplier : "";
        
        item.dateAcquired = sqlite3_column_int64(statement, 7);
        item.lastUpdated = sqlite3_column_int64(statement, 8);
        
        const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(statement, 9));
        item.notes = notes ? notes : "";
        
        const char* conditionStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (conditionStr) {
            item.condition = stringToCondition(conditionStr);
        }
        
        item.lastMaintenance = sqlite3_column_int64(statement, 11);
        item.expiryDate = sqlite3_column_int64(statement, 12);
        item.originalQuantity = sqlite3_column_int(statement, 13);
        
        items.push_back(item);
    }
    
    sqlite3_finalize(statement);
    return items;
}

bool Inventory::deleteItem(int itemId) {
    Inventory item = getItemById(itemId);
    if (item.getItemId() == 0) {
        std::cerr << "Item not found" << std::endl;
        return false;
    }
    
    return item.deleteFromDatabase();
}

// Inventory management operations
bool Inventory::addStock(int amount, const std::string& note) {
    if (amount <= 0) {
        std::cerr << "Invalid amount" << std::endl;
        return false;
    }
    
    quantity += amount;
    if (!note.empty()) {
        notes += "\n[" + getFormattedDate(std::time(nullptr)) + "] Added " + 
                std::to_string(amount) + " units. " + note;
    }
    
    return updateInDatabase();
}

bool Inventory::reduceStock(int amount, const std::string& note) {
    if (amount <= 0) {
        std::cerr << "Invalid amount" << std::endl;
        return false;
    }
    
    if (quantity < amount) {
        std::cerr << "Insufficient stock" << std::endl;
        return false;
    }
    
    quantity -= amount;
    if (!note.empty()) {
        notes += "\n[" + getFormattedDate(std::time(nullptr)) + "] Used " + 
                std::to_string(amount) + " units. " + note;
    }
    
    return updateInDatabase();
}

bool Inventory::updateCondition(EquipmentCondition newCondition, const std::string& note) {
    if (category != InventoryCategory::EQUIPMENT) {
        std::cerr << "Condition updates only apply to equipment" << std::endl;
        return false;
    }
    
    std::string oldConditionStr = getConditionString();
    condition = newCondition;
    
    if (!note.empty()) {
        notes += "\n[" + getFormattedDate(std::time(nullptr)) + "] Condition changed from " + 
                oldConditionStr + " to " + getConditionString() + ". " + note;
    }
    
    return updateInDatabase();
}

bool Inventory::recordMaintenance(const std::string& note) {
    if (category != InventoryCategory::EQUIPMENT) {
        std::cerr << "Maintenance records only apply to equipment" << std::endl;
        return false;
    }
    
    lastMaintenance = std::time(nullptr);
    
    if (!note.empty()) {
        notes += "\n[" + getFormattedDate(std::time(nullptr)) + "] Maintenance performed. " + note;
    }
    
    return updateInDatabase();
}

// Display functions
void Inventory::displayInfo() const {
    std::cout << "\n=== Item Details ===" << std::endl;
    std::cout << "ID: " << itemId << std::endl;
    std::cout << "Name: " << itemName << std::endl;
    std::cout << "Category: " << getCategoryString() << std::endl;
    std::cout << "Quantity: " << quantity;
    
    if (category != InventoryCategory::EQUIPMENT && originalQuantity > 0) {
        double usagePercent = getUsagePercentage();
        std::cout << " / " << originalQuantity << " (" << std::fixed << std::setprecision(1) 
                  << usagePercent << "% used)";
    }
    std::cout << std::endl;
    
    if (costPrice > 0) {
        std::cout << "Cost Price: " << getFormattedPrice(costPrice) << std::endl;
    }
    
    if (sellingPrice > 0) {
        std::cout << "Selling Price: " << getFormattedPrice(sellingPrice) << std::endl;
    }
    
    if (!supplier.empty()) {
        std::cout << "Supplier: " << supplier << std::endl;
    }
    
    std::cout << "Date Acquired: " << getFormattedDate(dateAcquired) << std::endl;
    
    // Category-specific information
    if (category == InventoryCategory::EQUIPMENT) {
        std::cout << "Condition: " << getConditionString();
        if (condition == EquipmentCondition::POOR || condition == EquipmentCondition::OUT_OF_ORDER) {
            std::cout << " ⚠️";
        }
        std::cout << std::endl;
        
        if (lastMaintenance > 0) {
            std::cout << "Last Maintenance: " << getFormattedDate(lastMaintenance);
            if (needsMaintenance()) {
                std::cout << " (⚠️ Maintenance due)";
            }
            std::cout << std::endl;
        } else {
            std::cout << "Last Maintenance: Never (⚠️ Maintenance needed)" << std::endl;
        }
    }
    
    // Show expiry for consumables
    if (category == InventoryCategory::CLEANING || category == InventoryCategory::DRINKS || 
        category == InventoryCategory::SUPPLEMENTS) {
        if (expiryDate > 0) {
            std::cout << "Expiry Date: " << getFormattedDate(expiryDate);
            if (isExpired()) {
                std::cout << " (⚠️ EXPIRED)";
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << "Last Updated: " << getFormattedDate(lastUpdated) << std::endl;
    
    if (!notes.empty()) {
        std::cout << "Notes: " << notes << std::endl;
    }
}

void Inventory::displaySummary() const {
    std::cout << std::left 
              << std::setw(4) << itemId
              << std::setw(25) << itemName.substr(0, 24)
              << std::setw(12) << getCategoryString()
              << std::setw(8) << quantity;
    
    if (category == InventoryCategory::EQUIPMENT) {
        std::cout << std::setw(12) << getConditionString();
        if (needsMaintenance()) {
            std::cout << " ⚠️";
        }
    } else {
        if (originalQuantity > 0) {
            double usage = getUsagePercentage();
            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << usage << "%";
        } else {
            std::cout << std::setw(8) << "-";
        }
        
        if (isExpired()) {
            std::cout << " ⚠️";
        }
    }
    
    std::cout << std::endl;
}

// Edit functions
void Inventory::editDetails() {
    std::cout << "\n=== Edit Item Details ===" << std::endl;
    displayInfo();
    
    std::cout << "\nWhat would you like to edit?" << std::endl;
    std::cout << "[1] Item Name" << std::endl;
    std::cout << "[2] Category" << std::endl;
    std::cout << "[3] Quantity" << std::endl;
    std::cout << "[4] Cost Price" << std::endl;
    std::cout << "[5] Selling Price" << std::endl;
    std::cout << "[6] Supplier" << std::endl;
    std::cout << "[7] Notes" << std::endl;
    
    if (category == InventoryCategory::EQUIPMENT) {
        std::cout << "[8] Condition" << std::endl;
        std::cout << "[9] Record Maintenance" << std::endl;
    }
    
    if (category != InventoryCategory::EQUIPMENT) {
        std::cout << "[8] Expiry Date" << std::endl;
        std::cout << "[9] Original Quantity" << std::endl;
    }
    
    std::cout << "[0] Cancel" << std::endl;
    
    int choice;
    std::cout << "Enter choice: ";
    std::cin >> choice;
    std::cin.ignore();
    
    bool updated = false;
    
    switch (choice) {
        case 1: {
            std::cout << "Current name: " << itemName << std::endl;
            std::cout << "Enter new name: ";
            std::string newName;
            std::getline(std::cin, newName);
            if (!newName.empty()) {
                itemName = newName;
                updated = true;
            }
            break;
        }
        
        case 2: {
            std::cout << "Current category: " << getCategoryString() << std::endl;
            std::cout << "[1] Equipment\n[2] Cleaning\n[3] Maintenance\n[4] Drinks\n[5] Supplements\n[6] Other" << std::endl;
            std::cout << "Select new category (1-6): ";
            int catChoice;
            std::cin >> catChoice;
            std::cin.ignore();
            
            switch (catChoice) {
                case 1: category = InventoryCategory::EQUIPMENT; updated = true; break;
                case 2: category = InventoryCategory::CLEANING; updated = true; break;
                case 3: category = InventoryCategory::MAINTENANCE; updated = true; break;
                case 4: category = InventoryCategory::DRINKS; updated = true; break;
                case 5: category = InventoryCategory::SUPPLEMENTS; updated = true; break;
                case 6: category = InventoryCategory::OTHER; updated = true; break;
                default: std::cout << "Invalid choice" << std::endl; break;
            }
            break;
        }
        
        case 3: {
            std::cout << "Current quantity: " << quantity << std::endl;
            std::cout << "Enter new quantity: ";
            int newQty;
            std::cin >> newQty;
            std::cin.ignore();
            if (newQty >= 0) {
                quantity = newQty;
                updated = true;
            }
            break;
        }
        
        case 4: {
            std::cout << "Current cost price: " << getFormattedPrice(costPrice) << std::endl;
            std::cout << "Enter new cost price (KES): ";
            double newPrice;
            std::cin >> newPrice;
            std::cin.ignore();
            if (newPrice >= 0) {
                costPrice = newPrice;
                updated = true;
            }
            break;
        }
        
        case 5: {
            std::cout << "Current selling price: " << getFormattedPrice(sellingPrice) << std::endl;
            std::cout << "Enter new selling price (KES): ";
            double newPrice;
            std::cin >> newPrice;
            std::cin.ignore();
            if (newPrice >= 0) {
                sellingPrice = newPrice;
                updated = true;
            }
            break;
        }
        
        case 6: {
            std::cout << "Current supplier: " << supplier << std::endl;
            std::cout << "Enter new supplier: ";
            std::string newSupplier;
            std::getline(std::cin, newSupplier);
            supplier = newSupplier;
            updated = true;
            break;
        }
        
        case 7: {
            std::cout << "Current notes: " << notes << std::endl;
            std::cout << "Enter additional note: ";
            std::string newNote;
            std::getline(std::cin, newNote);
            if (!newNote.empty()) {
                notes += "\n[" + getFormattedDate(std::time(nullptr)) + "] " + newNote;
                updated = true;
            }
            break;
        }
        
        case 8: {
            if (category == InventoryCategory::EQUIPMENT) {
                std::cout << "Current condition: " << getConditionString() << std::endl;
                std::cout << "[1] Excellent\n[2] Good\n[3] Fair\n[4] Poor\n[5] Out of Order" << std::endl;
                std::cout << "Select new condition (1-5): ";
                int condChoice;
                std::cin >> condChoice;
                std::cin.ignore();
                
                std::cout << "Enter maintenance note (optional): ";
                std::string note;
                std::getline(std::cin, note);
                
                switch (condChoice) {
                    case 1: updateCondition(EquipmentCondition::EXCELLENT, note); break;
                    case 2: updateCondition(EquipmentCondition::GOOD, note); break;
                    case 3: updateCondition(EquipmentCondition::FAIR, note); break;
                    case 4: updateCondition(EquipmentCondition::POOR, note); break;
                    case 5: updateCondition(EquipmentCondition::OUT_OF_ORDER, note); break;
                    default: std::cout << "Invalid choice" << std::endl; break;
                }
                return; // updateCondition already saves to database
            } else {
                // Set expiry date for consumables
                std::cout << "Set expiry date (YYYY-MM-DD format, or 'none' to clear): ";
                std::string dateStr;
                std::getline(std::cin, dateStr);
                
                if (dateStr == "none") {
                    expiryDate = 0;
                    updated = true;
                } else {
                    // Parse date string (basic parsing - you might want to improve this)
                    std::tm tm = {};
                    if (strptime(dateStr.c_str(), "%Y-%m-%d", &tm)) {
                        expiryDate = mktime(&tm);
                        updated = true;
                    } else {
                        std::cout << "Invalid date format" << std::endl;
                    }
                }
            }
            break;
        }
        
        case 9: {
            if (category == InventoryCategory::EQUIPMENT) {
                std::cout << "Enter maintenance note: ";
                std::string note;
                std::getline(std::cin, note);
                recordMaintenance(note);
                return; // recordMaintenance already saves to database
            } else {
                std::cout << "Current original quantity: " << originalQuantity << std::endl;
                std::cout << "Enter original quantity: ";
                int newOrigQty;
                std::cin >> newOrigQty;
                std::cin.ignore();
                if (newOrigQty >= 0) {
                    originalQuantity = newOrigQty;
                    updated = true;
                }
            }
            break;
        }
        
        case 0:
            std::cout << "Edit cancelled" << std::endl;
            return;
            
        default:
            std::cout << "Invalid choice" << std::endl;
            return;
    }
    
    if (updated) {
        if (updateInDatabase()) {
            std::cout << "✅ Item updated successfully!" << std::endl;
        } else {
            std::cout << "❌ Failed to update item" << std::endl;
        }
    }
}

// Static helper functions
InventoryCategory Inventory::stringToCategory(const std::string& categoryStr) {
    if (categoryStr == "equipment") return InventoryCategory::EQUIPMENT;
    if (categoryStr == "cleaning") return InventoryCategory::CLEANING;
    if (categoryStr == "maintenance") return InventoryCategory::MAINTENANCE;
    if (categoryStr == "drinks") return InventoryCategory::DRINKS;
    if (categoryStr == "supplements") return InventoryCategory::SUPPLEMENTS;
    return InventoryCategory::OTHER;
}

EquipmentCondition Inventory::stringToCondition(const std::string& conditionStr) {
    if (conditionStr == "excellent") return EquipmentCondition::EXCELLENT;
    if (conditionStr == "good") return EquipmentCondition::GOOD;
    if (conditionStr == "fair") return EquipmentCondition::FAIR;
    if (conditionStr == "poor") return EquipmentCondition::POOR;
    if (conditionStr == "out_of_order") return EquipmentCondition::OUT_OF_ORDER;
    return EquipmentCondition::EXCELLENT;
}