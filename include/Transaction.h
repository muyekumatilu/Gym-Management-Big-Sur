// File: include/Transaction.h
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <vector>
#include <ctime>
#include "AppConstants.h"

class Transaction {
private:
    int transactionId;
    int customerId;
    double amount;
    PaymentType paymentType;
    TransactionType transactionType;
    TransactionStatus status;
    std::time_t transactionDate;
    std::string description;
    std::string mpesaCode;          // For M-Pesa transactions
    std::string notes;
    
public:
    // Constructors
    Transaction();
    Transaction(int customerId, double amount, PaymentType paymentType, 
                TransactionType transactionType, const std::string& description = "");
    
    // Getters
    int getTransactionId() const { return transactionId; }
    int getCustomerId() const { return customerId; }
    double getAmount() const { return amount; }
    PaymentType getPaymentType() const { return paymentType; }
    TransactionType getTransactionType() const { return transactionType; }
    TransactionStatus getStatus() const { return status; }
    std::time_t getTransactionDate() const { return transactionDate; }
    std::string getDescription() const { return description; }
    std::string getMpesaCode() const { return mpesaCode; }
    std::string getNotes() const { return notes; }
    
    // Setters
    void setTransactionId(int id) { transactionId = id; }
    void setCustomerId(int id) { customerId = id; }
    void setAmount(double amt) { amount = amt; }
    void setPaymentType(PaymentType type) { paymentType = type; }
    void setTransactionType(TransactionType type) { transactionType = type; }
    void setStatus(TransactionStatus stat) { status = stat; }
    void setTransactionDate(std::time_t date) { transactionDate = date; }
    void setDescription(const std::string& desc) { description = desc; }
    void setMpesaCode(const std::string& code) { mpesaCode = code; }
    void setNotes(const std::string& note) { notes = note; }
    
    // String conversion methods
    std::string getPaymentTypeString() const;
    std::string getTransactionTypeString() const;
    std::string getStatusString() const;
    std::string getFormattedDate() const;
    std::string getFormattedAmount() const;
    
    // Business logic
    bool processPayment();
    bool refundTransaction(const std::string& reason = "");
    bool updateStatus(TransactionStatus newStatus);
    
    // Database operations
    bool saveToDatabase();
    bool updateInDatabase();
    bool loadFromDatabase(int transactionId);
    static std::vector<Transaction> getTransactionsByCustomer(int customerId);
    static std::vector<Transaction> getTransactionsByDate(std::time_t startDate, std::time_t endDate);
    static std::vector<Transaction> getTransactionsByType(TransactionType type);
    static std::vector<Transaction> getAllTransactions();
    static double getTotalRevenueByDate(std::time_t startDate, std::time_t endDate);
    static double getTotalRevenueByType(TransactionType type, std::time_t startDate = 0, std::time_t endDate = 0);

        // Delete operations
    bool deleteFromDatabase();
    static bool deleteTransaction(int transactionId);
    static bool deleteAllTransactions();
    static bool deleteTransactionsByCustomer(int customerId);
    static bool deleteTransactionsByDateRange(std::time_t startDate, std::time_t endDate);
    
    // Display methods
    void displayInfo() const;
    void displaySummary() const;
    
    // Static utility methods
    static PaymentType getPaymentTypeFromString(const std::string& typeStr);
    static TransactionType getTransactionTypeFromString(const std::string& typeStr);
    static TransactionStatus getTransactionStatusFromString(const std::string& statusStr);
};

#endif // TRANSACTION_H