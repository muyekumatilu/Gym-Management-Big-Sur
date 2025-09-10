// File: include/Inventory.h
#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <vector>
#include <ctime>

enum class InventoryCategory {
    EQUIPMENT,      // Gym equipment with condition tracking
    CLEANING,       // Cleaning supplies with usage tracking
    MAINTENANCE,    // Maintenance equipment/supplies
    DRINKS,         // Beverages/drinks
    SUPPLEMENTS,    // Protein powders, vitamins etc
    OTHER          // Miscellaneous items
};

enum class EquipmentCondition {
    EXCELLENT,      // Like new
    GOOD,           // Minor wear, fully functional
    FAIR,           // Moderate wear, needs attention
    POOR,           // Heavy wear, needs repair
    OUT_OF_ORDER    // Not functional
};

class Inventory {
private:
    int itemId;
    std::string itemName;
    InventoryCategory category;
    int quantity;
    double costPrice;
    double sellingPrice;
    std::string supplier;
    std::time_t dateAcquired;
    std::time_t lastUpdated;
    std::string notes;
    
    // Equipment specific
    EquipmentCondition condition;
    std::time_t lastMaintenance;
    
    // Consumables specific (cleaning, drinks, supplements)
    std::time_t expiryDate;
    int originalQuantity;
    
public:
    // Constructors
    Inventory();
    Inventory(const std::string& name, InventoryCategory cat, int qty, double cost = 0.0);
    
    // Destructor
    ~Inventory();
    
    // Getters
    int getItemId() const { return itemId; }
    std::string getItemName() const { return itemName; }
    InventoryCategory getCategory() const { return category; }
    int getQuantity() const { return quantity; }
    double getCostPrice() const { return costPrice; }
    double getSellingPrice() const { return sellingPrice; }
    std::string getSupplier() const { return supplier; }
    std::time_t getDateAcquired() const { return dateAcquired; }
    std::time_t getLastUpdated() const { return lastUpdated; }
    std::string getNotes() const { return notes; }
    EquipmentCondition getCondition() const { return condition; }
    std::time_t getLastMaintenance() const { return lastMaintenance; }
    std::time_t getExpiryDate() const { return expiryDate; }
    int getOriginalQuantity() const { return originalQuantity; }
    
    // Setters
    void setItemName(const std::string& name) { itemName = name; }
    void setCategory(InventoryCategory cat) { category = cat; }
    void setQuantity(int qty) { quantity = qty; }
    void setCostPrice(double price) { costPrice = price; }
    void setSellingPrice(double price) { sellingPrice = price; }
    void setSupplier(const std::string& sup) { supplier = sup; }
    void setNotes(const std::string& note) { notes = note; }
    void setCondition(EquipmentCondition cond) { condition = cond; }
    void setLastMaintenance(std::time_t time) { lastMaintenance = time; }
    void setExpiryDate(std::time_t time) { expiryDate = time; }
    void setOriginalQuantity(int qty) { originalQuantity = qty; }
    
    // Utility functions
    std::string getCategoryString() const;
    std::string getConditionString() const;
    std::string getFormattedDate(std::time_t time) const;
    std::string getFormattedPrice(double price) const;
    bool isExpired() const;
    bool needsMaintenance(int daysSinceLastMaintenance = 90) const;
    double getUsagePercentage() const;
    
    // Database operations
    bool saveToDatabase();
    bool updateInDatabase();
    bool deleteFromDatabase();
    
    // Static database operations
    static std::vector<Inventory> getAllItems();
    static std::vector<Inventory> getItemsByCategory(InventoryCategory category);
    static Inventory getItemById(int itemId);
    static std::vector<Inventory> searchItems(const std::string& searchTerm);
    static bool deleteItem(int itemId);
    
    // Inventory management operations
    bool addStock(int amount, const std::string& note = "");
    bool reduceStock(int amount, const std::string& note = "");
    bool updateCondition(EquipmentCondition newCondition, const std::string& note = "");
    bool recordMaintenance(const std::string& note = "");
    
    // Display functions
    void displayInfo() const;
    void displaySummary() const;
    
    // Edit functions
    void editDetails();
    
    // Static helper functions
    static InventoryCategory stringToCategory(const std::string& categoryStr);
    static EquipmentCondition stringToCondition(const std::string& conditionStr);
};

#endif // INVENTORY_H