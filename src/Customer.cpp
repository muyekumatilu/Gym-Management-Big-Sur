// File: src/Customer.cpp
#include "Customer.h"
#include "Transaction.h"
#include "DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cstdio>

// Default constructor
Customer::Customer() 
    : customerId(0), firstName(""), lastName(""), email(""), phone(""),
      membershipType(MembershipType::DAILY_GYM), joinDate(0), expiryDate(0),
      lastPaymentDate(0), balance(0.0), customerCategory(CustomerCategory::REGULAR) {
}

// Parameterized constructor
Customer::Customer(const std::string& firstName, const std::string& lastName, 
                   const std::string& phone, MembershipType membershipType,
                   CustomerCategory category)
    : customerId(0), firstName(firstName), lastName(lastName), email(""),
      phone(phone), membershipType(membershipType), balance(0.0), 
      customerCategory(category) {
    
    // Set current time as join date and last payment date
    this->joinDate = std::time(nullptr);
    this->lastPaymentDate = std::time(nullptr);
    
    // Set expiry date based on membership type
    calculateExpiryDate();
}

void Customer::calculateExpiryDate() {
    std::time_t currentTime = std::time(nullptr);
    
    switch (membershipType) {
        case MembershipType::DAILY_GYM:
        case MembershipType::DAILY_PERSONAL_TRAINING:
        case MembershipType::DAILY_MARTIAL_ARTS:
        case MembershipType::TRAINER:
            expiryDate = 0; // No expiry for daily and trainer
            break;
        case MembershipType::WEEKLY:
            expiryDate = currentTime + (7 * 24 * 60 * 60); // 7 days
            break;
        case MembershipType::MONTHLY:
            expiryDate = currentTime + (30 * 24 * 60 * 60); // 30 days
            break;
    }
}

bool Customer::isExpired() const {
    // Daily and trainer memberships don't expire
    if (membershipType == MembershipType::DAILY_GYM || 
            membershipType == MembershipType::DAILY_PERSONAL_TRAINING ||
            membershipType == MembershipType::DAILY_MARTIAL_ARTS ||
            membershipType == MembershipType::TRAINER || 
            expiryDate == 0) {
        return false;
    }
    
    std::time_t currentTime = std::time(nullptr);
    return currentTime > expiryDate;
}

bool Customer::canAccess() const {
    // Check if customer can access the gym
    switch (membershipType) {
        case MembershipType::DAILY_GYM:
        case MembershipType::DAILY_PERSONAL_TRAINING:
        case MembershipType::DAILY_MARTIAL_ARTS:
            // Daily members need to pay each visit
            return balance >= 0; // No outstanding debt
        case MembershipType::TRAINER:
            // Trainers always have access
            return true;
        case MembershipType::WEEKLY:
        case MembershipType::MONTHLY:
            // Check if membership is not expired
            return !isExpired();
        default:
            return false;
    }
}

CustomerStatus Customer::getStatus() const {
    return isExpired() ? CustomerStatus::EXPIRED : CustomerStatus::ACTIVE;
}

int Customer::getDaysRemaining() const {
    if (membershipType == MembershipType::DAILY_GYM || 
            membershipType == MembershipType::DAILY_PERSONAL_TRAINING ||
            membershipType == MembershipType::DAILY_MARTIAL_ARTS ||
            membershipType == MembershipType::TRAINER || 
            expiryDate == 0) {
        return -1; // No expiry
    }
    
    std::time_t currentTime = std::time(nullptr);
    if (expiryDate <= currentTime) {
        return 0; // Expired
    }
    
    int secondsRemaining = static_cast<int>(expiryDate - currentTime);
    return (secondsRemaining / (24 * 60 * 60)) + 1; // Convert to days, round up
}

std::string Customer::getMembershipTypeString() const {
    return MembershipTypeHelper::toString(membershipType);
}

std::string Customer::getStatusString() const {
    return CustomerStatusHelper::toString(getStatus());
}

std::string Customer::getCategoryString() const {
    return CustomerCategoryHelper::toString(customerCategory);
}

double Customer::getMembershipPrice() const {
    return getMembershipPrice(membershipType, customerCategory);
}

double Customer::getMembershipPrice(MembershipType type, CustomerCategory category) const {
    // Check if customer category qualifies for discount
    if (CustomerCategoryHelper::hasDiscount(category)) {
        return MembershipTypeHelper::getDiscountedPrice(type);
    } else {
        return MembershipTypeHelper::getStandardPrice(type);
    }
}

bool Customer::renewMembership(MembershipType type, CustomerCategory category, int days) {
    membershipType = type;
    customerCategory = category;
    lastPaymentDate = std::time(nullptr);
    
    if (days > 0) {
        // Custom renewal period
        expiryDate = lastPaymentDate + (days * 24 * 60 * 60);
    } else {
        // Standard renewal
        calculateExpiryDate();
    }
    
    return updateInDatabase();
}

bool Customer::checkIn() {
    if (!canAccess()) {
        std::cout << "❌ Access denied for " << getFullName() << std::endl;
        if (isExpired()) {
            std::cout << "   Reason: Membership expired" << std::endl;
        }
        return false;
    }
    
    DatabaseManager* db = DatabaseManager::getInstance();
    
    // Record check-in
    std::string query = R"(
        INSERT INTO check_ins (customer_id, check_in_time)
        VALUES (?, ?)
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return false;
    }
    
    std::time_t currentTime = std::time(nullptr);
    
    sqlite3_bind_int(statement, 1, customerId);
    sqlite3_bind_int64(statement, 2, currentTime);
    
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    sqlite3_finalize(statement);
    
    if (success) {
        std::cout << "✅ " << getFullName() << " checked in successfully" << std::endl;
        
            // For daily members, this counts as a visit
        if (MembershipTypeHelper::isDailyType(membershipType)) {
            // Could add daily fee logic here
        }
    }
    
    return success;
}

bool Customer::checkOut() {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    // Find the most recent check-in without a check-out
    std::string query = R"(
        UPDATE check_ins 
        SET check_out_time = ?
        WHERE customer_id = ? AND check_out_time IS NULL
        ORDER BY check_in_time DESC
        LIMIT 1
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return false;
    }
    
    std::time_t currentTime = std::time(nullptr);
    
    sqlite3_bind_int64(statement, 1, currentTime);
    sqlite3_bind_int(statement, 2, customerId);
    
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    sqlite3_finalize(statement);
    
    if (success) {
        std::cout << "✅ " << getFullName() << " checked out successfully" << std::endl;
    }
    
    return success;
}

bool Customer::saveToDatabase() {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    std::string query = R"(
        INSERT INTO customers (first_name, last_name, email, phone, membership_type,
                              join_date, expiry_date, last_payment_date, balance, customer_category)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return false;
    }
    
    std::string membershipTypeStr;
        switch (membershipType) {
            case MembershipType::DAILY_GYM: membershipTypeStr = "daily_gym"; break;
            case MembershipType::DAILY_PERSONAL_TRAINING: membershipTypeStr = "daily_personal_training"; break;
            case MembershipType::DAILY_MARTIAL_ARTS: membershipTypeStr = "daily_martial_arts"; break;
            case MembershipType::WEEKLY: membershipTypeStr = "weekly"; break;
            case MembershipType::MONTHLY: membershipTypeStr = "monthly"; break;
            case MembershipType::TRAINER: membershipTypeStr = "trainer"; break;
        }
        
        std::string categoryStr;
        switch (customerCategory) {
            case CustomerCategory::REGULAR: categoryStr = "regular"; break;
            case CustomerCategory::STUDENT: categoryStr = "student"; break;
            case CustomerCategory::MINOR: categoryStr = "minor"; break;
        }
    
    sqlite3_bind_text(statement, 1, firstName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, lastName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 3, email.empty() ? nullptr : email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 4, phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 5, membershipTypeStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 6, joinDate);
    sqlite3_bind_int64(statement, 7, expiryDate == 0 ? 0 : expiryDate);
    sqlite3_bind_int64(statement, 8, lastPaymentDate);
    sqlite3_bind_double(statement, 9, balance);
    sqlite3_bind_text(statement, 10, categoryStr.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    
    if (success) {
        customerId = static_cast<int>(sqlite3_last_insert_rowid(db->getDatabase()));
        std::cout << "✅ Customer " << getFullName() << " saved to database (ID: " << customerId << ")" << std::endl;
    }
    
    sqlite3_finalize(statement);
    return success;
}

Customer Customer::findByPhone(const std::string& phone) {
    DatabaseManager* db = DatabaseManager::getInstance();
    Customer customer;
    
    std::string query = R"(
        SELECT customer_id, first_name, last_name, email, phone, membership_type,
               join_date, expiry_date, last_payment_date, balance, customer_category
        FROM customers 
        WHERE phone = ?
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return customer;
    }
    
    sqlite3_bind_text(statement, 1, phone.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(statement) == SQLITE_ROW) {
        customer.setCustomerId(sqlite3_column_int(statement, 0));
        customer.setFirstName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
        customer.setLastName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        customer.setEmail(email ? email : "");
        
        customer.setPhone(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
        
        // Convert membership type string to enum
        std::string membershipStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        customer.setMembershipType(MembershipTypeHelper::fromString(membershipStr));
        
        // Convert customer category string to enum
        const char* categoryPtr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (categoryPtr) {
            customer.setCustomerCategory(CustomerCategoryHelper::fromString(std::string(categoryPtr)));
        } else {
            customer.setCustomerCategory(CustomerCategory::REGULAR); // Default fallback
        }
        
        customer.setJoinDate(sqlite3_column_int64(statement, 6));
        customer.setExpiryDate(sqlite3_column_int64(statement, 7));
        customer.setLastPaymentDate(sqlite3_column_int64(statement, 8));
        customer.setBalance(sqlite3_column_double(statement, 9));
    }
    
    sqlite3_finalize(statement);
    return customer;
}

void Customer::displayInfo() const {
    std::cout << "\n=== Customer Information ===" << std::endl;
    std::cout << "ID: " << customerId << std::endl;
    std::cout << "Name: " << getFullName() << std::endl;
    std::cout << "Phone: " << phone << std::endl;
    if (!email.empty()) {
        std::cout << "Email: " << email << std::endl;
    }
    std::cout << "Membership: " << getMembershipTypeString() << std::endl;
    std::cout << "Status: " << getStatusString() << std::endl;
    
    if (membershipType == MembershipType::WEEKLY || membershipType == MembershipType::MONTHLY) {
        int daysLeft = getDaysRemaining();
        if (daysLeft > 0) {
            std::cout << "Days Remaining: " << daysLeft << std::endl;
        } else if (daysLeft == 0) {
            std::cout << "⚠️  Expires today!" << std::endl;
        }
    }
    
    if (balance != 0) {
        std::cout << "Balance: KES " << std::fixed << std::setprecision(2) << balance << std::endl;
    }
    
    std::cout << "=========================" << std::endl;
}

void Customer::displaySummary() const {
    std::cout << std::left << std::setw(4) << customerId
              << std::setw(20) << getFullName()
              << std::setw(15) << phone
              << std::setw(10) << getMembershipTypeString()
              << std::setw(10) << getStatusString();
    
    if (membershipType == MembershipType::WEEKLY || membershipType == MembershipType::MONTHLY) {
        int days = getDaysRemaining();
        if (days >= 0) {
            std::cout << std::setw(8) << (std::to_string(days) + " days");
        } else {
            std::cout << std::setw(8) << "N/A";
        }
    } else {
        std::cout << std::setw(8) << "N/A";
    }
    
    std::cout << std::endl;
}

// Add these two methods to the end of your Customer.cpp file

bool Customer::updateInDatabase() {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    std::string query = R"(
        UPDATE customers 
        SET first_name = ?, last_name = ?, email = ?, phone = ?, 
            membership_type = ?, expiry_date = ?, last_payment_date = ?, 
            balance = ?, customer_category = ?, updated_at = strftime('%s', 'now')
        WHERE customer_id = ?
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return false;
    }
    
    std::string membershipTypeStr;
    switch (membershipType) {
        case MembershipType::DAILY_GYM: membershipTypeStr = "daily_gym"; break;
        case MembershipType::DAILY_PERSONAL_TRAINING: membershipTypeStr = "daily_personal_training"; break;
        case MembershipType::DAILY_MARTIAL_ARTS: membershipTypeStr = "daily_martial_arts"; break;
        case MembershipType::WEEKLY: membershipTypeStr = "weekly"; break;
        case MembershipType::MONTHLY: membershipTypeStr = "monthly"; break;
        case MembershipType::TRAINER: membershipTypeStr = "trainer"; break;
    }
    
    std::string categoryStr;
    switch (customerCategory) {
        case CustomerCategory::REGULAR: categoryStr = "regular"; break;
        case CustomerCategory::STUDENT: categoryStr = "student"; break;
        case CustomerCategory::MINOR: categoryStr = "minor"; break;
    }
    
    sqlite3_bind_text(statement, 1, firstName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, lastName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 3, email.empty() ? nullptr : email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 4, phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 5, membershipTypeStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 6, expiryDate == 0 ? 0 : expiryDate);
    sqlite3_bind_int64(statement, 7, lastPaymentDate);
    sqlite3_bind_double(statement, 8, balance);
    sqlite3_bind_text(statement, 9, categoryStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 10, customerId);
    
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    sqlite3_finalize(statement);
    
    if (success) {
        std::cout << "✅ Customer " << getFullName() << " updated in database" << std::endl;
    }
    
    return success;
}

bool Customer::loadFromDatabase(int customerId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db) return false;
    
    const std::string query = "SELECT customer_id, first_name, last_name, email, phone, "
                             "membership_type, join_date, expiry_date, last_payment_date, balance, customer_category "
                             "FROM customers WHERE customer_id = ?";
    
    sqlite3_stmt* stmt = db->prepareStatement(query);
    if (!stmt) return false;
    
    sqlite3_bind_int(stmt, 1, customerId);
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        this->customerId = sqlite3_column_int(stmt, 0);
        this->firstName = (char*)sqlite3_column_text(stmt, 1);
        this->lastName = (char*)sqlite3_column_text(stmt, 2);
        
        // Handle potential NULL email
        const char* emailPtr = (char*)sqlite3_column_text(stmt, 3);
        this->email = emailPtr ? emailPtr : "";
        
        this->phone = (char*)sqlite3_column_text(stmt, 4);
        
        // CRITICAL: Improved membership type conversion with validation
        const char* membershipStr = (char*)sqlite3_column_text(stmt, 5);
        if (membershipStr) {
            this->membershipType = MembershipTypeHelper::fromString(std::string(membershipStr));
        } else {
            std::cout << "Warning: NULL membership type for customer " << customerId 
                     << ", defaulting to DAILY_GYM" << std::endl;
            this->membershipType = MembershipType::DAILY_GYM;
        }
        
        // Handle customer category conversion
        const char* categoryStr = (char*)sqlite3_column_text(stmt, 10);
        if (categoryStr) {
            this->customerCategory = CustomerCategoryHelper::fromString(std::string(categoryStr));
        } else {
            this->customerCategory = CustomerCategory::REGULAR; // Default fallback
        }
        
        this->joinDate = sqlite3_column_int64(stmt, 6);
        this->expiryDate = sqlite3_column_int64(stmt, 7);
        this->lastPaymentDate = sqlite3_column_int64(stmt, 8);
        this->balance = sqlite3_column_double(stmt, 9);
        
        success = true;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

std::vector<Customer> Customer::getAllCustomers() {
    DatabaseManager* db = DatabaseManager::getInstance();
    std::vector<Customer> customers;
    
    std::string query = R"(
        SELECT customer_id, first_name, last_name, email, phone, membership_type,
               join_date, expiry_date, last_payment_date, balance, customer_category
        FROM customers 
        ORDER BY first_name, last_name
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return customers;
    }
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Customer customer;
        
        customer.setCustomerId(sqlite3_column_int(statement, 0));
        customer.setFirstName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
        customer.setLastName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        customer.setEmail(email ? email : "");
        
        customer.setPhone(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
        
        // Convert membership type string to enum
        std::string membershipStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        customer.setMembershipType(MembershipTypeHelper::fromString(membershipStr));
        
        // Convert customer category string to enum
        const char* categoryPtr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (categoryPtr) {
            customer.setCustomerCategory(CustomerCategoryHelper::fromString(std::string(categoryPtr)));
        } else {
            customer.setCustomerCategory(CustomerCategory::REGULAR); // Default fallback
        }
        
        customer.setJoinDate(sqlite3_column_int64(statement, 6));
        customer.setExpiryDate(sqlite3_column_int64(statement, 7));
        customer.setLastPaymentDate(sqlite3_column_int64(statement, 8));
        customer.setBalance(sqlite3_column_double(statement, 9));
        
        customers.push_back(customer);
    }
    
    sqlite3_finalize(statement);
    return customers;
}

bool Customer::deleteFromDatabase() {
    if (customerId == 0) {
        std::cout << "❌ Cannot delete customer: Invalid customer ID" << std::endl;
        return false;
    }
    
    return deleteCustomer(customerId);
}

bool Customer::deleteCustomer(int customerId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    // Start transaction for data integrity
    db->beginTransaction();
    
    try {
        // Get customer name for logging before deletion
        std::string customerName;
        std::string getNameQuery = "SELECT first_name, last_name FROM customers WHERE customer_id = ?";
        sqlite3_stmt* nameStatement = db->prepareStatement(getNameQuery);
        if (nameStatement != nullptr) {
            sqlite3_bind_int(nameStatement, 1, customerId);
            if (sqlite3_step(nameStatement) == SQLITE_ROW) {
                std::string firstName = reinterpret_cast<const char*>(sqlite3_column_text(nameStatement, 0));
                std::string lastName = reinterpret_cast<const char*>(sqlite3_column_text(nameStatement, 1));
                customerName = firstName + " " + lastName;
            }
            sqlite3_finalize(nameStatement);
        }

        // First, delete related check-ins
        std::string deleteCheckInsQuery = "DELETE FROM check_ins WHERE customer_id = ?";
        sqlite3_stmt* checkInStatement = db->prepareStatement(deleteCheckInsQuery);
        if (checkInStatement == nullptr) {
            db->rollbackTransaction();
            return false;
        }
        
        sqlite3_bind_int(checkInStatement, 1, customerId);
        bool checkInsDeleted = (sqlite3_step(checkInStatement) == SQLITE_DONE);
        sqlite3_finalize(checkInStatement);
        
        if (!checkInsDeleted) {
            std::cout << "❌ Failed to delete customer check-in records" << std::endl;
            db->rollbackTransaction();
            return false;
        }
        
        // Delete related transactions
        std::string deleteTransactionsQuery = "DELETE FROM transactions WHERE customer_id = ?";
        sqlite3_stmt* transactionStatement = db->prepareStatement(deleteTransactionsQuery);
        if (transactionStatement == nullptr) {
            db->rollbackTransaction();
            return false;
        }
        
        sqlite3_bind_int(transactionStatement, 1, customerId);
        bool transactionsDeleted = (sqlite3_step(transactionStatement) == SQLITE_DONE);
        sqlite3_finalize(transactionStatement);
        
        if (!transactionsDeleted) {
            std::cout << "❌ Failed to delete customer transaction records" << std::endl;
            db->rollbackTransaction();
            return false;
        }
        
        // Finally, delete the customer
        std::string deleteCustomerQuery = "DELETE FROM customers WHERE customer_id = ?";
        sqlite3_stmt* customerStatement = db->prepareStatement(deleteCustomerQuery);
        if (customerStatement == nullptr) {
            db->rollbackTransaction();
            return false;
        }
        
        sqlite3_bind_int(customerStatement, 1, customerId);
        bool customerDeleted = (sqlite3_step(customerStatement) == SQLITE_DONE);
        sqlite3_finalize(customerStatement);
        
        if (!customerDeleted) {
            std::cout << "❌ Failed to delete customer record" << std::endl;
            db->rollbackTransaction();
            return false;
        }
        
        // Check if any rows were actually affected
        int changes = sqlite3_changes(db->getDatabase());
        if (changes == 0) {
            std::cout << "❌ No customer found with ID: " << customerId << std::endl;
            db->rollbackTransaction();
            return false;
        }
        
        // Commit the transaction
        db->commitTransaction();
        std::cout << "✅ Customer " << customerName << " and all related records deleted successfully" << std::endl;
        return true;
        
    } catch (...) {
        db->rollbackTransaction();
        std::cout << "❌ Error occurred during customer deletion" << std::endl;
        return false;
    }
}

bool Customer::editDetails() {
    std::cout << "\n=== Edit Customer Details ===" << std::endl;
    displayInfo();
    
    std::cout << "\nWhat would you like to edit?" << std::endl;
    std::cout << "[1] First Name (Current: " << firstName << ")" << std::endl;
    std::cout << "[2] Last Name (Current: " << lastName << ")" << std::endl;
    std::cout << "[3] Email (Current: " << (email.empty() ? "Not set" : email) << ")" << std::endl;
    std::cout << "[4] Phone (Current: " << phone << ")" << std::endl;
    std::cout << "[5] Membership Type (Current: " << getMembershipTypeString() << ")" << std::endl;
    std::cout << "[0] Cancel" << std::endl;
    
    // We'll need the input helper functions, so we need to add them as parameters
    // For now, let's implement a basic version
    int choice;
    std::cout << "Enter choice (0-5): ";
    std::cin >> choice;
    std::cin.ignore(); // Clear buffer
    
    bool modified = false;
    std::string input;
    
    switch (choice) {
        case 1:
            std::cout << "Enter new first name: ";
            std::getline(std::cin, input);
            if (!input.empty()) {
                firstName = input;
                modified = true;
            }
            break;
            
        case 2:
            std::cout << "Enter new last name: ";
            std::getline(std::cin, input);
            if (!input.empty()) {
                lastName = input;
                modified = true;
            }
            break;
            
        case 3:
            std::cout << "Enter new email (or press Enter to clear): ";
            std::getline(std::cin, input);
            email = input; // Allow empty to clear email
            modified = true;
            break;
            
        case 4:
            std::cout << "Enter new phone number: ";
            std::getline(std::cin, input);
            if (!input.empty()) {
                // Basic phone validation
                if (input.length() >= 10 && input.length() <= 13) {
                    phone = input;
                    modified = true;
                } else {
                    std::cout << "❌ Invalid phone number format." << std::endl;
                    return false;
                }
            }
            break;
            
        case 5: {
            std::cout << "\nMembership Types:" << std::endl;
            std::cout << "[1] Daily Gym" << std::endl;
            std::cout << "[2] Daily Personal Training" << std::endl;
            std::cout << "[3] Daily Martial Arts" << std::endl;
            std::cout << "[4] Weekly" << std::endl;
            std::cout << "[5] Monthly" << std::endl;
            std::cout << "[6] Trainer" << std::endl;
            
            int typeChoice;
            std::cout << "Select new membership type (1-6): ";
            std::cin >> typeChoice;
            std::cin.ignore();
            
            MembershipType oldType = membershipType;
            switch (typeChoice) {
                case 1: membershipType = MembershipType::DAILY_GYM; break;
                case 2: membershipType = MembershipType::DAILY_PERSONAL_TRAINING; break;
                case 3: membershipType = MembershipType::DAILY_MARTIAL_ARTS; break;
                case 4: membershipType = MembershipType::WEEKLY; break;
                case 5: membershipType = MembershipType::MONTHLY; break;
                case 6: membershipType = MembershipType::TRAINER; break;
                default:
                    std::cout << "❌ Invalid choice." << std::endl;
                    return false;
            }
            
            // If membership type changed, recalculate expiry
            if (oldType != membershipType) {
                calculateExpiryDate();
                modified = true;
            }
            break;
        }
        
        case 0:
            std::cout << "Edit cancelled." << std::endl;
            return false;
            
        default:
            std::cout << "❌ Invalid choice." << std::endl;
            return false;
    }
    
    if (modified) {
        if (updateInDatabase()) {
            std::cout << "\n✅ Customer details updated successfully!" << std::endl;
            displayInfo();
            return true;
        } else {
            std::cout << "\n❌ Failed to update customer in database." << std::endl;
            return false;
        }
    }
    
    return false;
}

std::vector<Customer> Customer::findByName(const std::string& name) {
    DatabaseManager* db = DatabaseManager::getInstance();
    std::vector<Customer> customers;
    
    std::string query = R"(
        SELECT customer_id, first_name, last_name, email, phone, membership_type,
               join_date, expiry_date, last_payment_date, balance, customer_category
        FROM customers 
        WHERE LOWER(first_name) LIKE LOWER(?) OR LOWER(last_name) LIKE LOWER(?) 
           OR LOWER(first_name || ' ' || last_name) LIKE LOWER(?)
        ORDER BY first_name, last_name
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return customers;
    }
    
    std::string searchTerm = "%" + name + "%";
    sqlite3_bind_text(statement, 1, searchTerm.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, searchTerm.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 3, searchTerm.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Customer customer;
        
        customer.setCustomerId(sqlite3_column_int(statement, 0));
        customer.setFirstName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
        customer.setLastName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        customer.setEmail(email ? email : "");
        
        customer.setPhone(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
        
        // Convert membership type string to enum
        std::string membershipStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        customer.setMembershipType(MembershipTypeHelper::fromString(membershipStr));
        
        // Convert customer category string to enum
        const char* categoryPtr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (categoryPtr) {
            customer.setCustomerCategory(CustomerCategoryHelper::fromString(std::string(categoryPtr)));
        } else {
            customer.setCustomerCategory(CustomerCategory::REGULAR); // Default fallback
        }
        
        customer.setJoinDate(sqlite3_column_int64(statement, 6));
        customer.setExpiryDate(sqlite3_column_int64(statement, 7));
        customer.setLastPaymentDate(sqlite3_column_int64(statement, 8));
        customer.setBalance(sqlite3_column_double(statement, 9));
        
        customers.push_back(customer);
    }
    
    sqlite3_finalize(statement);
    return customers;
}

void Customer::recordPayment(double amount, PaymentType paymentType, TransactionType transactionType, const std::string& description) {
    Transaction transaction(customerId, amount, paymentType, transactionType, description);
    
    if (transaction.processPayment()) {
        // Update customer balance based on payment type
        if (transactionType == TransactionType::MEMBERSHIP_PAYMENT || 
            transactionType == TransactionType::RENEWAL) {
            balance = 0.0; // Clear outstanding balance
            updateInDatabase();
        }
    }
}

bool Customer::phoneExists(const std::string& phone, int excludeCustomerId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db || !db->isConnected()) {
        return false;
    }
    
    std::string sql = "SELECT COUNT(*) FROM customers WHERE phone = ?";
    if (excludeCustomerId != -1) {
        sql += " AND customer_id != ?";
    }
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db->getDatabase(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_STATIC);
    if (excludeCustomerId != -1) {
        sqlite3_bind_int(stmt, 2, excludeCustomerId);
    }
    
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

bool Customer::customerExists(const std::string& firstName, const std::string& lastName, const std::string& phone, int excludeCustomerId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db || !db->isConnected()) {
        return false;
    }
    
    std::string sql = "SELECT COUNT(*) FROM customers WHERE (phone = ? OR (first_name = ? AND last_name = ?))";
    if (excludeCustomerId != -1) {
        sql += " AND customer_id != ?";
    }
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db->getDatabase(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, firstName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, lastName.c_str(), -1, SQLITE_STATIC);
    if (excludeCustomerId != -1) {
        sqlite3_bind_int(stmt, 4, excludeCustomerId);
    }
    
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

std::vector<Customer> Customer::getCustomersByCategory(CustomerCategory category) {
    DatabaseManager* db = DatabaseManager::getInstance();
    std::vector<Customer> customers;
    
    std::string query = R"(
        SELECT customer_id, first_name, last_name, email, phone, membership_type,
               join_date, expiry_date, last_payment_date, balance, customer_category
        FROM customers 
        WHERE customer_category = ?
        ORDER BY first_name, last_name
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return customers;
    }
    
    std::string categoryStr = CustomerCategoryHelper::toString(category);
    std::transform(categoryStr.begin(), categoryStr.end(), categoryStr.begin(), ::tolower);
    
    sqlite3_bind_text(statement, 1, categoryStr.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Customer customer;
        
        customer.setCustomerId(sqlite3_column_int(statement, 0));
        customer.setFirstName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
        customer.setLastName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        customer.setEmail(email ? email : "");
        
        customer.setPhone(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
        
        // Convert membership type string to enum
        std::string membershipStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        customer.setMembershipType(MembershipTypeHelper::fromString(membershipStr));
        
        customer.setJoinDate(sqlite3_column_int64(statement, 6));
        customer.setExpiryDate(sqlite3_column_int64(statement, 7));
        customer.setLastPaymentDate(sqlite3_column_int64(statement, 8));
        customer.setBalance(sqlite3_column_double(statement, 9));
        
        // Convert customer category string to enum
        const char* categoryPtr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (categoryPtr) {
            customer.setCustomerCategory(CustomerCategoryHelper::fromString(std::string(categoryPtr)));
        } else {
            customer.setCustomerCategory(CustomerCategory::REGULAR);
        }
        
        customers.push_back(customer);
    }
    
    sqlite3_finalize(statement);
    return customers;
}

std::vector<Customer> Customer::getCustomersByMembershipAndCategory(MembershipType type, CustomerCategory category) {
    DatabaseManager* db = DatabaseManager::getInstance();
    std::vector<Customer> customers;
    
    std::string query = R"(
        SELECT customer_id, first_name, last_name, email, phone, membership_type,
               join_date, expiry_date, last_payment_date, balance, customer_category
        FROM customers 
        WHERE membership_type = ? AND customer_category = ?
        ORDER BY first_name, last_name
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return customers;
    }
    
    // Convert membership type to database string
    std::string membershipTypeStr;
    switch (type) {
        case MembershipType::DAILY_GYM: membershipTypeStr = "daily_gym"; break;
        case MembershipType::DAILY_PERSONAL_TRAINING: membershipTypeStr = "daily_personal_training"; break;
        case MembershipType::DAILY_MARTIAL_ARTS: membershipTypeStr = "daily_martial_arts"; break;
        case MembershipType::WEEKLY: membershipTypeStr = "weekly"; break;
        case MembershipType::MONTHLY: membershipTypeStr = "monthly"; break;
        case MembershipType::TRAINER: membershipTypeStr = "trainer"; break;
    }
    
    std::string categoryStr = CustomerCategoryHelper::toString(category);
    std::transform(categoryStr.begin(), categoryStr.end(), categoryStr.begin(), ::tolower);
    
    sqlite3_bind_text(statement, 1, membershipTypeStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, categoryStr.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Customer customer;
        
        customer.setCustomerId(sqlite3_column_int(statement, 0));
        customer.setFirstName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
        customer.setLastName(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
        
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        customer.setEmail(email ? email : "");
        
        customer.setPhone(reinterpret_cast<const char*>(sqlite3_column_text(statement, 4)));
        
        // Convert membership type string to enum
        std::string membershipStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        customer.setMembershipType(MembershipTypeHelper::fromString(membershipStr));
        
        customer.setJoinDate(sqlite3_column_int64(statement, 6));
        customer.setExpiryDate(sqlite3_column_int64(statement, 7));
        customer.setLastPaymentDate(sqlite3_column_int64(statement, 8));
        customer.setBalance(sqlite3_column_double(statement, 9));
        
        // Convert customer category string to enum
        const char* categoryPtr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 10));
        if (categoryPtr) {
            customer.setCustomerCategory(CustomerCategoryHelper::fromString(std::string(categoryPtr)));
        } else {
            customer.setCustomerCategory(CustomerCategory::REGULAR);
        }
        
        customers.push_back(customer);
    }
    
    sqlite3_finalize(statement);
    return customers;
}

bool Customer::isValidMembershipCombination(MembershipType type, CustomerCategory category) {
    // All membership types are valid with regular customers
    if (category == CustomerCategory::REGULAR) {
        return true;
    }
    
    // For students and minors, check if discounts are available
    if (category == CustomerCategory::STUDENT || category == CustomerCategory::MINOR) {
        switch (type) {
            case MembershipType::DAILY_GYM:              // Has discount
            case MembershipType::DAILY_MARTIAL_ARTS:     // Has discount
            case MembershipType::MONTHLY:                // Has discount
                return true;
            case MembershipType::DAILY_PERSONAL_TRAINING: // No discount yet
            case MembershipType::WEEKLY:                 // No discount
            case MembershipType::TRAINER:                // No discount
                return true; // Valid but no discount applied
            default:
                return false;
        }
    }
    
    return false; // Invalid category
}


