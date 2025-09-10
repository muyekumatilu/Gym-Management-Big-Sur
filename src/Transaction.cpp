// File: src/Transaction.cpp
#include "Transaction.h"
#include "DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdio>

// Default constructor
Transaction::Transaction()
    : transactionId(0), customerId(0), amount(0.0), 
      paymentType(PaymentType::CASH), transactionType(TransactionType::OTHER),
      status(TransactionStatus::PENDING), transactionDate(0),
      description(""), mpesaCode(""), notes("") {
}

// Parameterized constructor
Transaction::Transaction(int customerId, double amount, PaymentType paymentType, 
                        TransactionType transactionType, const std::string& description)
    : transactionId(0), customerId(customerId), amount(amount),
      paymentType(paymentType), transactionType(transactionType),
      status(TransactionStatus::PENDING), description(description),
      mpesaCode(""), notes("") {
    
    transactionDate = std::time(nullptr);
}

std::string Transaction::getPaymentTypeString() const {
    switch (paymentType) {
        case PaymentType::CASH: return "Cash";
        case PaymentType::MPESA: return "M-Pesa";
        default: return "Unknown";
    }
}

std::string Transaction::getTransactionTypeString() const {
    switch (transactionType) {
        case TransactionType::MEMBERSHIP_PAYMENT: return "Membership Payment";
        case TransactionType::DAILY_FEE: return "Daily Fee";
        case TransactionType::RENEWAL: return "Renewal";
        case TransactionType::REFUND: return "Refund";
        case TransactionType::OTHER: return "Other";
        default: return "Unknown";
    }
}

std::string Transaction::getStatusString() const {
    switch (status) {
        case TransactionStatus::COMPLETED: return "Completed";
        case TransactionStatus::PENDING: return "Pending";
        case TransactionStatus::FAILED: return "Failed";
        case TransactionStatus::REFUNDED: return "Refunded";
        default: return "Unknown";
    }
}

std::string Transaction::getFormattedDate() const {
    if (transactionDate == 0) return "N/A";
    
    std::tm* timeInfo = std::localtime(&transactionDate);
    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Transaction::getFormattedAmount() const {
    std::ostringstream oss;
    oss << "KES " << std::fixed << std::setprecision(2) << amount;
    return oss.str();
}

bool Transaction::processPayment() {
    // Set status to completed for now
    // In a real implementation, this would integrate with M-Pesa API for M-Pesa payments
    status = TransactionStatus::COMPLETED;
    
    if (saveToDatabase()) {
        std::cout << "✅ Payment of " << getFormattedAmount() << " processed successfully" << std::endl;
        if (paymentType == PaymentType::MPESA && !mpesaCode.empty()) {
            std::cout << "   M-Pesa Code: " << mpesaCode << std::endl;
        }
        return true;
    } else {
        status = TransactionStatus::FAILED;
        std::cout << "❌ Payment processing failed" << std::endl;
        return false;
    }
}

bool Transaction::refundTransaction(const std::string& reason) {
    if (status != TransactionStatus::COMPLETED) {
        std::cout << "❌ Cannot refund transaction that is not completed" << std::endl;
        return false;
    }
    
    status = TransactionStatus::REFUNDED;
    if (!reason.empty()) {
        notes = "Refund reason: " + reason;
    }
    
    return updateInDatabase();
}

bool Transaction::updateStatus(TransactionStatus newStatus) {
    status = newStatus;
    return updateInDatabase();
}

bool Transaction::saveToDatabase() {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    std::string query = R"(
        INSERT INTO transactions (customer_id, amount, payment_type, transaction_type,
                                status, transaction_date, description, mpesa_code, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return false;
    }
    
    // Convert enums to strings for database storage
    std::string paymentTypeStr = (paymentType == PaymentType::CASH) ? "cash" : "mpesa";
    std::string transactionTypeStr;
    switch (transactionType) {
        case TransactionType::MEMBERSHIP_PAYMENT: transactionTypeStr = "membership_payment"; break;
        case TransactionType::DAILY_FEE: transactionTypeStr = "daily_fee"; break;
        case TransactionType::RENEWAL: transactionTypeStr = "renewal"; break;
        case TransactionType::REFUND: transactionTypeStr = "refund"; break;
        case TransactionType::OTHER: transactionTypeStr = "other"; break;
    }
    
    std::string statusStr;
    switch (status) {
        case TransactionStatus::COMPLETED: statusStr = "completed"; break;
        case TransactionStatus::PENDING: statusStr = "pending"; break;
        case TransactionStatus::FAILED: statusStr = "failed"; break;
        case TransactionStatus::REFUNDED: statusStr = "refunded"; break;
    }
    
    sqlite3_bind_int(statement, 1, customerId);
    sqlite3_bind_double(statement, 2, amount);
    sqlite3_bind_text(statement, 3, paymentTypeStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 4, transactionTypeStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 5, statusStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 6, transactionDate);
    sqlite3_bind_text(statement, 7, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 8, mpesaCode.empty() ? nullptr : mpesaCode.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 9, notes.empty() ? nullptr : notes.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    
    if (success) {
        transactionId = static_cast<int>(sqlite3_last_insert_rowid(db->getDatabase()));
    }
    
    sqlite3_finalize(statement);
    return success;
}

bool Transaction::loadFromDatabase(int transactionId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db) return false;
    
    const std::string query = "SELECT transaction_id, customer_id, amount, payment_type, "
                             "transaction_type, status, transaction_date, description, "
                             "mpesa_code, notes FROM transactions WHERE transaction_id = ?";
    
    sqlite3_stmt* stmt = db->prepareStatement(query);
    if (!stmt) return false;
    
    sqlite3_bind_int(stmt, 1, transactionId);
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        this->transactionId = sqlite3_column_int(stmt, 0);
        this->customerId = sqlite3_column_int(stmt, 1);
        this->amount = sqlite3_column_double(stmt, 2);
        
        // Convert payment type from integer
        int paymentInt = sqlite3_column_int(stmt, 3);
        this->paymentType = (paymentInt == 0) ? PaymentType::CASH : PaymentType::MPESA;
        
        // Convert transaction type from integer
        int transactionTypeInt = sqlite3_column_int(stmt, 4);
        switch (transactionTypeInt) {
            case 0: this->transactionType = TransactionType::MEMBERSHIP_PAYMENT; break;
            case 1: this->transactionType = TransactionType::DAILY_FEE; break;
            case 2: this->transactionType = TransactionType::RENEWAL; break;
            case 3: this->transactionType = TransactionType::REFUND; break;
            case 4: this->transactionType = TransactionType::OTHER; break;
            default: this->transactionType = TransactionType::OTHER; break;
        }
        
        // Convert status from integer
        int statusInt = sqlite3_column_int(stmt, 5);
        switch (statusInt) {
            case 0: this->status = TransactionStatus::COMPLETED; break;
            case 1: this->status = TransactionStatus::PENDING; break;
            case 2: this->status = TransactionStatus::FAILED; break;
            case 3: this->status = TransactionStatus::REFUNDED; break;
            default: this->status = TransactionStatus::COMPLETED; break;
        }
        
        this->transactionDate = sqlite3_column_int64(stmt, 6);
        
        // Handle potential NULL description
        const char* descPtr = (char*)sqlite3_column_text(stmt, 7);
        this->description = descPtr ? descPtr : "";
        
        // Handle potential NULL mpesa_code
        const char* mpesaPtr = (char*)sqlite3_column_text(stmt, 8);
        this->mpesaCode = mpesaPtr ? mpesaPtr : "";
        
        // Handle potential NULL notes
        const char* notesPtr = (char*)sqlite3_column_text(stmt, 9);
        this->notes = notesPtr ? notesPtr : "";
        
        success = true;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool Transaction::updateInDatabase() {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    std::string query = R"(
        UPDATE transactions 
        SET customer_id = ?, amount = ?, payment_type = ?, transaction_type = ?,
            status = ?, description = ?, mpesa_code = ?, notes = ?,
            updated_at = strftime('%s', 'now')
        WHERE transaction_id = ?
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return false;
    }
    
    // Convert enums to strings
    std::string paymentTypeStr = (paymentType == PaymentType::CASH) ? "cash" : "mpesa";
    std::string transactionTypeStr;
    switch (transactionType) {
        case TransactionType::MEMBERSHIP_PAYMENT: transactionTypeStr = "membership_payment"; break;
        case TransactionType::DAILY_FEE: transactionTypeStr = "daily_fee"; break;
        case TransactionType::RENEWAL: transactionTypeStr = "renewal"; break;
        case TransactionType::REFUND: transactionTypeStr = "refund"; break;
        case TransactionType::OTHER: transactionTypeStr = "other"; break;
    }
    
    std::string statusStr;
    switch (status) {
        case TransactionStatus::COMPLETED: statusStr = "completed"; break;
        case TransactionStatus::PENDING: statusStr = "pending"; break;
        case TransactionStatus::FAILED: statusStr = "failed"; break;
        case TransactionStatus::REFUNDED: statusStr = "refunded"; break;
    }
    
    sqlite3_bind_int(statement, 1, customerId);
    sqlite3_bind_double(statement, 2, amount);
    sqlite3_bind_text(statement, 3, paymentTypeStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 4, transactionTypeStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 5, statusStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 6, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 7, mpesaCode.empty() ? nullptr : mpesaCode.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 8, notes.empty() ? nullptr : notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 9, transactionId);
    
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    sqlite3_finalize(statement);
    
    return success;
}

std::vector<Transaction> Transaction::getTransactionsByCustomer(int customerId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    std::vector<Transaction> transactions;
    
    std::string query = R"(
        SELECT transaction_id, customer_id, amount, payment_type, transaction_type,
               status, transaction_date, description, mpesa_code, notes
        FROM transactions 
        WHERE customer_id = ?
        ORDER BY transaction_date DESC
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return transactions;
    }
    
    sqlite3_bind_int(statement, 1, customerId);
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Transaction transaction;
        
        transaction.setTransactionId(sqlite3_column_int(statement, 0));
        transaction.setCustomerId(sqlite3_column_int(statement, 1));
        transaction.setAmount(sqlite3_column_double(statement, 2));
        
        // Convert strings back to enums
        std::string paymentTypeStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        transaction.setPaymentType(getPaymentTypeFromString(paymentTypeStr));
        
        std::string transactionTypeStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
        transaction.setTransactionType(getTransactionTypeFromString(transactionTypeStr));
        
        std::string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        transaction.setStatus(getTransactionStatusFromString(statusStr));
        
        transaction.setTransactionDate(sqlite3_column_int64(statement, 6));
        transaction.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(statement, 7)));
        
        const char* mpesaCode = reinterpret_cast<const char*>(sqlite3_column_text(statement, 8));
        transaction.setMpesaCode(mpesaCode ? mpesaCode : "");
        
        const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(statement, 9));
        transaction.setNotes(notes ? notes : "");
        
        transactions.push_back(transaction);
    }
    
    sqlite3_finalize(statement);
    return transactions;
}

std::vector<Transaction> Transaction::getAllTransactions() {
    DatabaseManager* db = DatabaseManager::getInstance();
    std::vector<Transaction> transactions;
    
    std::string query = R"(
        SELECT transaction_id, customer_id, amount, payment_type, transaction_type,
               status, transaction_date, description, mpesa_code, notes
        FROM transactions 
        ORDER BY transaction_date DESC
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return transactions;
    }
    
    while (sqlite3_step(statement) == SQLITE_ROW) {
        Transaction transaction;
        
        transaction.setTransactionId(sqlite3_column_int(statement, 0));
        transaction.setCustomerId(sqlite3_column_int(statement, 1));
        transaction.setAmount(sqlite3_column_double(statement, 2));
        
        std::string paymentTypeStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
        transaction.setPaymentType(getPaymentTypeFromString(paymentTypeStr));
        
        std::string transactionTypeStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
        transaction.setTransactionType(getTransactionTypeFromString(transactionTypeStr));
        
        std::string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));
        transaction.setStatus(getTransactionStatusFromString(statusStr));
        
        transaction.setTransactionDate(sqlite3_column_int64(statement, 6));
        transaction.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(statement, 7)));
        
        const char* mpesaCode = reinterpret_cast<const char*>(sqlite3_column_text(statement, 8));
        transaction.setMpesaCode(mpesaCode ? mpesaCode : "");
        
        const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(statement, 9));
        transaction.setNotes(notes ? notes : "");
        
        transactions.push_back(transaction);
    }
    
    sqlite3_finalize(statement);
    return transactions;
}

double Transaction::getTotalRevenueByDate(std::time_t startDate, std::time_t endDate) {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    std::string query = R"(
        SELECT COALESCE(SUM(amount), 0) 
        FROM transactions 
        WHERE status = 'completed' 
          AND transaction_date >= ? 
          AND transaction_date <= ?
          AND transaction_type != 'refund'
    )";
    
    sqlite3_stmt* statement = db->prepareStatement(query);
    if (statement == nullptr) {
        return 0.0;
    }
    
    sqlite3_bind_int64(statement, 1, startDate);
    sqlite3_bind_int64(statement, 2, endDate);
    
    double total = 0.0;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        total = sqlite3_column_double(statement, 0);
    }
    
    sqlite3_finalize(statement);
    return total;
}

void Transaction::displayInfo() const {
    std::cout << "\n=== Transaction Details ===" << std::endl;
    std::cout << "Transaction ID: " << transactionId << std::endl;
    std::cout << "Customer ID: " << customerId << std::endl;
    std::cout << "Amount: " << getFormattedAmount() << std::endl;
    std::cout << "Payment Type: " << getPaymentTypeString() << std::endl;
    std::cout << "Transaction Type: " << getTransactionTypeString() << std::endl;
    std::cout << "Status: " << getStatusString() << std::endl;
    std::cout << "Date: " << getFormattedDate() << std::endl;
    
    if (!description.empty()) {
        std::cout << "Description: " << description << std::endl;
    }
    
    if (paymentType == PaymentType::MPESA && !mpesaCode.empty()) {
        std::cout << "M-Pesa Code: " << mpesaCode << std::endl;
    }
    
    if (!notes.empty()) {
        std::cout << "Notes: " << notes << std::endl;
    }
    
    std::cout << "==========================" << std::endl;
}

void Transaction::displaySummary() const {
    std::cout << std::left 
              << std::setw(6) << transactionId
              << std::setw(8) << customerId
              << std::setw(12) << getFormattedAmount()
              << std::setw(10) << getPaymentTypeString()
              << std::setw(18) << getTransactionTypeString()
              << std::setw(12) << getStatusString()
              << getFormattedDate().substr(0, 16) << std::endl;
}

// Static utility methods
PaymentType Transaction::getPaymentTypeFromString(const std::string& typeStr) {
    if (typeStr == "cash") return PaymentType::CASH;
    if (typeStr == "mpesa") return PaymentType::MPESA;
    return PaymentType::CASH; // Default
}

TransactionType Transaction::getTransactionTypeFromString(const std::string& typeStr) {
    if (typeStr == "membership_payment") return TransactionType::MEMBERSHIP_PAYMENT;
    if (typeStr == "daily_fee") return TransactionType::DAILY_FEE;
    if (typeStr == "renewal") return TransactionType::RENEWAL;
    if (typeStr == "refund") return TransactionType::REFUND;
    if (typeStr == "other") return TransactionType::OTHER;
    return TransactionType::OTHER; // Default
}

TransactionStatus Transaction::getTransactionStatusFromString(const std::string& statusStr) {
    if (statusStr == "completed") return TransactionStatus::COMPLETED;
    if (statusStr == "pending") return TransactionStatus::PENDING;
    if (statusStr == "failed") return TransactionStatus::FAILED;
    if (statusStr == "refunded") return TransactionStatus::REFUNDED;
    return TransactionStatus::PENDING; // Default
}

bool Transaction::deleteFromDatabase() {
    if (transactionId == 0) {
        std::cout << "❌ Cannot delete transaction: Invalid transaction ID" << std::endl;
        return false;
    }
    
    return deleteTransaction(transactionId);
}

bool Transaction::deleteTransaction(int transactionId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    std::string query = "DELETE FROM transactions WHERE transaction_id = ?";
    sqlite3_stmt* statement = db->prepareStatement(query);
    
    if (statement == nullptr) {
        return false;
    }
    
    sqlite3_bind_int(statement, 1, transactionId);
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    sqlite3_finalize(statement);
    
    if (success) {
        int changes = sqlite3_changes(db->getDatabase());
        if (changes > 0) {
            std::cout << "✅ Transaction deleted successfully" << std::endl;
        } else {
            std::cout << "❌ No transaction found with ID: " << transactionId << std::endl;
            return false;
        }
    }
    
    return success;
}

bool Transaction::deleteAllTransactions() {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    // Get count first for confirmation
    std::string countQuery = "SELECT COUNT(*) FROM transactions";
    sqlite3_stmt* countStatement = db->prepareStatement(countQuery);
    
    int totalCount = 0;
    if (countStatement != nullptr) {
        if (sqlite3_step(countStatement) == SQLITE_ROW) {
            totalCount = sqlite3_column_int(countStatement, 0);
        }
        sqlite3_finalize(countStatement);
    }
    
    if (totalCount == 0) {
        std::cout << "No transactions to delete." << std::endl;
        return true;
    }
    
    std::string query = "DELETE FROM transactions";
    bool success = db->executeQuery(query);
    
    if (success) {
        std::cout << "✅ All " << totalCount << " transactions deleted successfully" << std::endl;
    } else {
        std::cout << "❌ Failed to delete transactions" << std::endl;
    }
    
    return success;
}

bool Transaction::deleteTransactionsByCustomer(int customerId) {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    // Get count first
    std::string countQuery = "SELECT COUNT(*) FROM transactions WHERE customer_id = ?";
    sqlite3_stmt* countStatement = db->prepareStatement(countQuery);
    
    int count = 0;
    if (countStatement != nullptr) {
        sqlite3_bind_int(countStatement, 1, customerId);
        if (sqlite3_step(countStatement) == SQLITE_ROW) {
            count = sqlite3_column_int(countStatement, 0);
        }
        sqlite3_finalize(countStatement);
    }
    
    if (count == 0) {
        std::cout << "No transactions found for customer ID: " << customerId << std::endl;
        return true;
    }
    
    std::string query = "DELETE FROM transactions WHERE customer_id = ?";
    sqlite3_stmt* statement = db->prepareStatement(query);
    
    if (statement == nullptr) {
        return false;
    }
    
    sqlite3_bind_int(statement, 1, customerId);
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    sqlite3_finalize(statement);
    
    if (success) {
        std::cout << "✅ " << count << " transactions deleted for customer ID: " << customerId << std::endl;
    }
    
    return success;
}

bool Transaction::deleteTransactionsByDateRange(std::time_t startDate, std::time_t endDate) {
    DatabaseManager* db = DatabaseManager::getInstance();
    
    std::string query = "DELETE FROM transactions WHERE transaction_date >= ? AND transaction_date <= ?";
    sqlite3_stmt* statement = db->prepareStatement(query);
    
    if (statement == nullptr) {
        return false;
    }
    
    sqlite3_bind_int64(statement, 1, startDate);
    sqlite3_bind_int64(statement, 2, endDate);
    
    bool success = (sqlite3_step(statement) == SQLITE_DONE);
    int deletedCount = sqlite3_changes(db->getDatabase());
    sqlite3_finalize(statement);
    
    if (success) {
        std::cout << "✅ " << deletedCount << " transactions deleted in date range" << std::endl;
    }
    
    return success;
}