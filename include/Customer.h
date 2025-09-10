// File: include/Customer.h
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>
#include <vector>
#include <ctime>
#include "AppConstants.h"

class Customer {
private:
    int customerId;
    std::string firstName;
    std::string lastName;
    std::string email;
    std::string phone;
    MembershipType membershipType;
    std::time_t joinDate;
    std::time_t expiryDate;        // NULL for daily/trainer memberships
    std::time_t lastPaymentDate;
    double balance;                // Outstanding amount owed
    CustomerCategory customerCategory;     // Customer pricing category (regular/student/minor)
    
public:
    // Constructors
    Customer();
    Customer(const std::string& firstName, const std::string& lastName, 
            const std::string& phone, MembershipType membershipType, 
            CustomerCategory category = CustomerCategory::REGULAR);
        
    // Getters
    int getCustomerId() const { return customerId; }
    std::string getFirstName() const { return firstName; }
    std::string getLastName() const { return lastName; }
    std::string getFullName() const { return firstName + " " + lastName; }
    std::string getEmail() const { return email; }
    std::string getPhone() const { return phone; }
    MembershipType getMembershipType() const { return membershipType; }
    std::time_t getJoinDate() const { return joinDate; }
    std::time_t getExpiryDate() const { return expiryDate; }
    std::time_t getLastPaymentDate() const { return lastPaymentDate; }
    double getBalance() const { return balance; }
    CustomerCategory getCustomerCategory() const { return customerCategory; }
    
    // Setters
    void setCustomerId(int id) { customerId = id; }
    void setFirstName(const std::string& name) { firstName = name; }
    void setLastName(const std::string& name) { lastName = name; }
    void setEmail(const std::string& email) { this->email = email; }
    void setPhone(const std::string& phone) { this->phone = phone; }
    void setMembershipType(MembershipType type) { membershipType = type; }
    void setJoinDate(std::time_t date) { joinDate = date; }
    void setExpiryDate(std::time_t date) { expiryDate = date; }
    void setLastPaymentDate(std::time_t date) { lastPaymentDate = date; }
    void setBalance(double balance) { this->balance = balance; }
    void calculateExpiryDate(); // Calculate expiry based on membership type
    void recordPayment(double amount, PaymentType paymentType, TransactionType transactionType, const std::string& description = "");
    void setCustomerCategory(CustomerCategory category) { customerCategory = category; }

    // Business logic
    bool checkIn();
    bool checkOut();
    bool isExpired() const;              // Check if weekly/monthly has expired
    bool canAccess() const;              // Check if can access gym today
    CustomerStatus getStatus() const;     // Calculate status based on expiry
    int getDaysRemaining() const;        // Days left on membership
    std::string getMembershipTypeString() const;
    std::string getCategoryString() const;           // Get customer category as string
    double getMembershipPrice() const;               // Get price based on type and category
    double getMembershipPrice(MembershipType type, CustomerCategory category) const; // Static price calculator
    std::string getStatusString() const;
    bool validateData() const {
        return !firstName.empty() && !lastName.empty() && !phone.empty();
    }
    bool renewMembership(MembershipType type, CustomerCategory category = CustomerCategory::REGULAR, int days = 0); // Renew membership with category
    bool editDetails(); // Method to update customer details interactively
    
    // Database operations
    bool saveToDatabase();
    bool loadFromDatabase(int customerId);
    bool updateInDatabase();
    static std::vector<Customer> getAllCustomers();
    static Customer findByPhone(const std::string& phone);
    static std::vector<Customer> findByName(const std::string& name);
    bool deleteFromDatabase();
    static bool deleteCustomer(int customerId);
    static bool phoneExists(const std::string& phone, int excludeCustomerId = -1);
    static bool customerExists(const std::string& firstName, const std::string& lastName, const std::string& phone, int excludeCustomerId = -1);
    static std::vector<Customer> getCustomersByCategory(CustomerCategory category);
    static std::vector<Customer> getCustomersByMembershipAndCategory(MembershipType type, CustomerCategory category);
    static bool isValidMembershipCombination(MembershipType type, CustomerCategory category); // Validate membership-category combination

    // Display methods
    void displayInfo() const;
    void displaySummary() const;
};

#endif // CUSTOMER_H