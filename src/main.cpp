// File: src/main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include "DatabaseManager.h"
#include "Customer.h"
#include "Transaction.h"
#include "Inventory.h"
#include "Reports.h"

// Forward declarations
void renewMembership(Customer& customer);
void addNewCustomer();
void deleteCustomer();
void searchCustomer();
void customerCheckIn();
void customerCheckOut();
void renewMembershipStandalone();
void listAllCustomers();
void editCustomer();
void showCustomerMenu();
void showCheckInOutMenu();
void testDatabaseConnection();
void showMainMenu();
void processPayment();
void viewTransactionHistory();
void showPaymentMenu();
PaymentType getPaymentType();
TransactionType getTransactionType();
void deleteTransactions();
void showDeleteTransactionsMenu();
// Forward declarations for inventory functions (add these at the top with other forward declarations)
void addInventoryItem();
void viewInventoryList();
void searchInventoryItem();
void updateInventoryStock();
void updateInventoryCondition();
void deleteInventoryItem();
void showInventoryByCategory();
void showInventoryAlerts();
void showInventoryMenu();
InventoryCategory getInventoryCategory();
EquipmentCondition getEquipmentCondition();
Inventory selectInventoryItem(const std::vector<Inventory>& items);



// Phone utility functions
std::string formatPhoneNumber(const std::string& phone);
bool isValidPhone(const std::string& phone);
std::string getPhoneInput(const std::string& prompt);

// Function to clear input buffer
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Function to get integer input with validation
int getIntInput(const std::string& prompt, int min = 0, int max = 100) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            clearInputBuffer();
            return value;
        }
        std::cout << "❌ Invalid input. Please enter a number between " << min << " and " << max << "." << std::endl;
        clearInputBuffer();
    }
}

// Function to get string input with validation
std::string getStringInput(const std::string& prompt, bool required = true) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        
        // Trim whitespace
        input.erase(0, input.find_first_not_of(" \t"));
        input.erase(input.find_last_not_of(" \t") + 1);
        
        if (!required || !input.empty()) {
            return input;
        }
        std::cout << "❌ This field is required. Please try again." << std::endl;
    }
}

// Function to format phone number to international format
std::string formatPhoneNumber(const std::string& phone) {
    std::string cleanPhone = phone;
    
    // Remove any spaces, dashes, or other non-digit characters except +
    std::string temp;
    for (char c : cleanPhone) {
        if (std::isdigit(c) || c == '+') {
            temp += c;
        }
    }
    cleanPhone = temp;
    
    // If it already starts with +254, return as is
    if (cleanPhone.substr(0, 4) == "+254") {
        return cleanPhone;
    }
    
    // If it starts with 254 (without +), add the +
    if (cleanPhone.substr(0, 3) == "254" && cleanPhone.length() >= 12) {
        return "+" + cleanPhone;
    }
    
    // If it starts with 07 or 01 and has 10 digits, convert to +254 format
    if (cleanPhone.length() == 10 && 
        (cleanPhone.substr(0, 2) == "07" || cleanPhone.substr(0, 2) == "01")) {
        return "+254" + cleanPhone.substr(1); // Remove the 0 and add +254
    }
    
    // If it's 9 digits starting with 7 or 1 (assuming the 0 was already removed)
    if (cleanPhone.length() == 9 && (cleanPhone[0] == '7' || cleanPhone[0] == '1')) {
        return "+254" + cleanPhone;
    }
    
    // Return original if we can't determine the format
    return phone;
}

// Function to get phone input with automatic formatting
std::string getPhoneInput(const std::string& prompt) {
    std::string phone;
    while (true) {
        std::cout << prompt << " (e.g., 0712345678 or 0112345678): ";
        std::getline(std::cin, phone);
        
        // Trim whitespace
        phone.erase(0, phone.find_first_not_of(" \t"));
        phone.erase(phone.find_last_not_of(" \t") + 1);
        
        if (phone.empty()) {
            std::cout << "❌ Phone number is required. Please try again." << std::endl;
            continue;
        }
        
        // Format the phone number
        std::string formattedPhone = formatPhoneNumber(phone);
        
        // Validate the formatted number
        if (isValidPhone(formattedPhone)) {
            if (phone != formattedPhone) {
                std::cout << "✅ Formatted to: " << formattedPhone << std::endl;
            }
            return formattedPhone;
        }
        
        std::cout << "❌ Invalid phone number format. Please use Kenyan format:" << std::endl;
        std::cout << "   - Mobile: 0712345678 (will become +254712345678)" << std::endl;
        std::cout << "   - Landline: 0112345678 (will become +254112345678)" << std::endl;
        std::cout << "   - Already formatted: +254712345678" << std::endl;
    }
}

// Function to validate phone number format
bool isValidPhone(const std::string& phone) {
    std::string formatted = formatPhoneNumber(phone);
    
    // Check if formatted number starts with +254 and has correct length
    if (formatted.substr(0, 4) != "+254") {
        return false;
    }
    
    // Should be +254 followed by 9 digits (total 13 characters)
    if (formatted.length() != 13) {
        return false;
    }
    
    // Check if all characters after +254 are digits
    for (size_t i = 4; i < formatted.length(); ++i) {
        if (!std::isdigit(formatted[i])) {
            return false;
        }
    }
    
    // Check if it's a valid Kenyan mobile/landline number
    // Mobile: starts with 7 (07xx becomes +2547xx)
    // Landline: starts with 1 (01xx becomes +2541xx) 
    char firstDigit = formatted[4];
    if (firstDigit != '7' && firstDigit != '1') {
        return false;
    }
    
    // Additional validation for common Kenyan prefixes
    if (firstDigit == '7') {
        // Mobile numbers: 070x, 071x, 072x, 073x, 074x, 075x, 076x, 077x, 078x, 079x
        char secondDigit = formatted[5];
        if (secondDigit < '0' || secondDigit > '9') {
            return false;
        }
    } else if (firstDigit == '1') {
        // Landline numbers: 010x, 011x, 020 (Nairobi), 040x, 041x, 042x, etc.
        char secondDigit = formatted[5];
        if (secondDigit < '0' || secondDigit > '9') {
            return false;
        }
    }
    
    return true;
}

// Function to get membership type from user
MembershipType getMembershipType() {
    std::cout << "\nMembership Types:" << std::endl;
    std::cout << "[1] Daily" << std::endl;
    std::cout << "[2] Weekly" << std::endl;
    std::cout << "[3] Monthly" << std::endl;
    std::cout << "[4] Trainer" << std::endl;
    
    int choice = getIntInput("Select membership type (1-4): ", 1, 4);
    
    switch (choice) {
        case 1: return MembershipType::DAILY;
        case 2: return MembershipType::WEEKLY;
        case 3: return MembershipType::MONTHLY;
        case 4: return MembershipType::TRAINER;
        default: return MembershipType::DAILY;
    }
}

// Function to select customer from search results
Customer selectCustomerFromList(const std::vector<Customer>& customers) {
    if (customers.empty()) {
        return Customer(); // Return empty customer
    }
    
    if (customers.size() == 1) {
        std::cout << "Customer found:" << std::endl;
        customers[0].displaySummary();
        return customers[0];
    }
    
    std::cout << "\nMultiple customers found:" << std::endl;
    std::cout << std::left << std::setw(4) << "No."
              << std::setw(20) << "Name"
              << std::setw(15) << "Phone"
              << std::setw(10) << "Type"
              << std::setw(10) << "Status" << std::endl;
    std::cout << std::string(59, '-') << std::endl;
    
    for (size_t i = 0; i < customers.size(); ++i) {
        std::cout << std::left << std::setw(4) << (i + 1);
        customers[i].displaySummary();
    }
    
    int choice = getIntInput("Select customer (1-" + std::to_string(customers.size()) + ") or 0 to cancel: ", 0, static_cast<int>(customers.size()));
    
    if (choice == 0) {
        return Customer(); // Return empty customer
    }
    
    return customers[choice - 1];
}

// Function to find customer by phone or name
Customer findCustomer(const std::string& prompt) {
    std::cout << "\n" << prompt << std::endl;
    std::cout << "[1] Search by Phone Number" << std::endl;
    std::cout << "[2] Search by Name" << std::endl;
    
    int searchType = getIntInput("Choose search method: ", 1, 2);
    
    Customer customer;
    
    if (searchType == 1) {
        std::string phone = getPhoneInput("Enter phone number: ");
        customer = Customer::findByPhone(phone);
        
        if (customer.getCustomerId() == 0) {
            std::cout << "❌ No customer found with phone number: " << phone << std::endl;
        }
    } else {
        std::string name = getStringInput("Enter customer name (first, last, or full name): ");
        std::vector<Customer> customers = Customer::findByName(name);
        
        if (customers.empty()) {
            std::cout << "❌ No customers found with name: " << name << std::endl;
            return Customer();
        }
        
        customer = selectCustomerFromList(customers);
    }
    
    return customer;
}

// Function to add a new customer
void addNewCustomer() {
    std::cout << "\n=== Add New Customer ===" << std::endl;
    
    // Get customer details
    std::string firstName = getStringInput("First Name: ");
    std::string lastName = getStringInput("Last Name: ");
    std::string phone = getPhoneInput("Phone Number");
    
    // Check if customer already exists
    Customer existingCustomer = Customer::findByPhone(phone);
    if (existingCustomer.getCustomerId() != 0) {
        std::cout << "❌ Customer with phone number " << phone << " already exists!" << std::endl;
        existingCustomer.displayInfo();
        return;
    }
    
    std::string email = getStringInput("Email (optional, press Enter to skip): ", false);
    MembershipType membershipType = getMembershipType();
    
    // Create new customer
    Customer newCustomer(firstName, lastName, phone, membershipType);
    if (!email.empty()) {
        newCustomer.setEmail(email);
    }
    
    // Save to database
    if (newCustomer.saveToDatabase()) {
        std::cout << "\n✅ Customer added successfully!" << std::endl;
        newCustomer.displayInfo();
    } else {
        std::cout << "\n❌ Failed to add customer to database." << std::endl;
    }
}

// Function to search and display customer
void searchCustomer() {
    Customer customer = findCustomer("=== Search Customer ===");
    
    if (customer.getCustomerId() != 0) {
        customer.displayInfo();
    }
}

// Function to handle customer check-in
void customerCheckIn() {
    Customer customer = findCustomer("=== Customer Check-In ===");
    
    if (customer.getCustomerId() == 0) {
        char addNew;
        std::cout << "Would you like to add a new customer? (y/n): ";
        std::cin >> addNew;
        clearInputBuffer();
        
        if (addNew == 'y' || addNew == 'Y') {
            addNewCustomer();
        }
        return;
    }
    
    // Display customer info
    customer.displayInfo();
    
    // Attempt check-in
    if (customer.checkIn()) {
        std::cout << "\n🎉 Check-in successful!" << std::endl;
    } else {
        std::cout << "\n❌ Check-in failed!" << std::endl;
        
        if (customer.isExpired()) {
            std::cout << "💡 Membership has expired. Please renew membership." << std::endl;
            
            char renewChoice;
            std::cout << "Would you like to renew membership now? (y/n): ";
            std::cin >> renewChoice;
            clearInputBuffer();
            
            if (renewChoice == 'y' || renewChoice == 'Y') {
                renewMembership(customer);
            }
        }
    }
}

// Function to handle customer check-out
void customerCheckOut() {
    Customer customer = findCustomer("=== Customer Check-Out ===");
    
    if (customer.getCustomerId() == 0) {
        return;
    }
    
    if (customer.checkOut()) {
        std::cout << "\n👋 Check-out successful!" << std::endl;
    } else {
        std::cout << "\n❌ Check-out failed or no active check-in found." << std::endl;
    }
}

// Function to renew membership
void renewMembership(Customer& customer) {
    std::cout << "\n=== Renew Membership ===" << std::endl;
    
    customer.displayInfo();
    
    std::cout << "\nCurrent membership: " << customer.getMembershipTypeString() << std::endl;
    
    MembershipType newType = getMembershipType();
    
    int customDays = 0;
    if (newType == MembershipType::WEEKLY || newType == MembershipType::MONTHLY) {
        char useCustom;
        std::cout << "Use custom duration? (y/n): ";
        std::cin >> useCustom;
        clearInputBuffer();
        
        if (useCustom == 'y' || useCustom == 'Y') {
            customDays = getIntInput("Enter number of days: ", 1, 365);
        }
    }
    
    if (customer.renewMembership(newType, customDays)) {
        std::cout << "\n✅ Membership renewed successfully!" << std::endl;
        customer.displayInfo();
    } else {
        std::cout << "\n❌ Failed to renew membership." << std::endl;
    }
}

// Function to standalone renew membership
void renewMembershipStandalone() {
    Customer customer = findCustomer("=== Renew Membership ===");
    
    if (customer.getCustomerId() == 0) {
        return;
    }
    
    renewMembership(customer);
}

// Function to list all customers
void listAllCustomers() {
    std::cout << "\n=== All Customers ===" << std::endl;
    
    std::vector<Customer> customers = Customer::getAllCustomers();
    
    if (customers.empty()) {
        std::cout << "No customers found in database." << std::endl;
        return;
    }
    
    // Display header
    std::cout << std::left << std::setw(4) << "ID"
              << std::setw(20) << "Name"
              << std::setw(15) << "Phone"
              << std::setw(10) << "Type"
              << std::setw(10) << "Status"
              << std::setw(8) << "Expires" << std::endl;
    
    std::cout << std::string(67, '-') << std::endl;
    
    // Display customers
    for (const auto& customer : customers) {
        customer.displaySummary();
    }
    
    std::cout << "\nTotal customers: " << customers.size() << std::endl;
}

// Function to filter customers by membership type
void filterCustomersByType() {
    std::cout << "\n=== Filter Customers by Type ===" << std::endl;
    
    // Get membership type to filter by
    MembershipType filterType = getMembershipType();
    
    // Get all customers
    std::vector<Customer> allCustomers = Customer::getAllCustomers();
    
    if (allCustomers.empty()) {
        std::cout << "No customers found in database." << std::endl;
        return;
    }
    
    // Filter customers by type
    std::vector<Customer> filteredCustomers;
    for (const auto& customer : allCustomers) {
        if (customer.getMembershipType() == filterType) {
            filteredCustomers.push_back(customer);
        }
    }
    
    // Display results
    std::string typeString;
    switch (filterType) {
        case MembershipType::DAILY: typeString = "Daily"; break;
        case MembershipType::WEEKLY: typeString = "Weekly"; break;
        case MembershipType::MONTHLY: typeString = "Monthly"; break;
        case MembershipType::TRAINER: typeString = "Trainer"; break;
    }
    
    std::cout << "\n=== " << typeString << " Members ===" << std::endl;
    
    if (filteredCustomers.empty()) {
        std::cout << "No " << typeString << " members found." << std::endl;
        return;
    }
    
    // Display header
    std::cout << std::left << std::setw(4) << "ID"
              << std::setw(20) << "Name"
              << std::setw(15) << "Phone"
              << std::setw(10) << "Type"
              << std::setw(10) << "Status"
              << std::setw(8) << "Expires" << std::endl;
    
    std::cout << std::string(67, '-') << std::endl;
    
    // Display filtered customers
    for (const auto& customer : filteredCustomers) {
        customer.displaySummary();
    }
    
    std::cout << "\nTotal " << typeString << " members: " << filteredCustomers.size() << std::endl;
}

// Function to edit customer details
void editCustomer() {
    Customer customer = findCustomer("=== Edit Customer ===");
    
    if (customer.getCustomerId() == 0) {
        return;
    }
    
    customer.editDetails();
}

// Function to delete a customer
void deleteCustomer() {
    Customer customer = findCustomer("=== Delete Customer ===");
    
    if (customer.getCustomerId() == 0) {
        return;
    }
    
    // Display customer info for confirmation
    customer.displayInfo();
    
    // Confirmation prompt
    std::cout << "\n⚠️  WARNING: This will permanently delete the customer and ALL related data including:" << std::endl;
    std::cout << "   • Customer information" << std::endl;
    std::cout << "   • Check-in/check-out history" << std::endl;
    std::cout << "   • Transaction history" << std::endl;
    std::cout << "   • This action CANNOT be undone!" << std::endl;
    
    char confirm1;
    std::cout << "\nAre you sure you want to delete " << customer.getFullName() << "? (y/n): ";
    std::cin >> confirm1;
    clearInputBuffer();
    
    if (confirm1 != 'y' && confirm1 != 'Y') {
        std::cout << "❌ Customer deletion cancelled." << std::endl;
        return;
    }
    
    // Double confirmation for safety
    char confirm2;
    std::cout << "⚠️  FINAL CONFIRMATION: Type 'Y' to permanently delete this customer: ";
    std::cin >> confirm2;
    clearInputBuffer();
    
    if (confirm2 != 'Y') {
        std::cout << "❌ Customer deletion cancelled." << std::endl;
        return;
    }
    
    // Proceed with deletion
    std::cout << "\n🗑️  Deleting customer..." << std::endl;
    
    if (customer.deleteFromDatabase()) {
        std::cout << "✅ Customer '" << customer.getFullName() << "' has been permanently deleted." << std::endl;
    } else {
        std::cout << "❌ Failed to delete customer. Please try again." << std::endl;
    }
}

// Function to show customer management menu
void showCustomerMenu() {
    int choice;
    do {
        std::cout << "\n=== Customer Management ===" << std::endl;
        std::cout << "[1] Add New Customer" << std::endl;
        std::cout << "[2] Search Customer" << std::endl;
        std::cout << "[3] List All Customers" << std::endl;
        std::cout << "[4] Filter by Membership Type" << std::endl;
        std::cout << "[5] Edit Customer" << std::endl;
        std::cout << "[6] Delete Customer" << std::endl;
        std::cout << "[7] Renew Membership" << std::endl;
        std::cout << "[0] Back to Main Menu" << std::endl;

choice = getIntInput("Enter choice: ", 0, 7);
        
    switch (choice) {
        case 1:
            addNewCustomer();
            break;
        case 2:
            searchCustomer();
            break;
        case 3:
            listAllCustomers();
            break;
        case 4:
            filterCustomersByType();
            break;
        case 5:
            editCustomer();
            break;
        case 6:
            deleteCustomer();
            break;
        case 7:
            renewMembershipStandalone();
            break;
        case 0:
            break;
        default:
            std::cout << "❌ Invalid choice!" << std::endl;
            break;
    }
        
        if (choice != 0) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
        
    } while (choice != 0);
}

// Function to show check-in/out menu
void showCheckInOutMenu() {
    int choice;
    do {
        std::cout << "\n=== Check In/Out ===" << std::endl;
        std::cout << "[1] Customer Check-In" << std::endl;
        std::cout << "[2] Customer Check-Out" << std::endl;
        std::cout << "[0] Back to Main Menu" << std::endl;
        
        choice = getIntInput("Enter choice: ", 0, 2);
        
        switch (choice) {
            case 1:
                customerCheckIn();
                break;
            case 2:
                customerCheckOut();
                break;
            case 0:
                break;
            default:
                std::cout << "❌ Invalid choice!" << std::endl;
                break;
        }
        
        if (choice != 0) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
        
    } while (choice != 0);
}

void testDatabaseConnection() {
    std::cout << "\n=== Testing Database Connection ===" << std::endl;
    
    // Get database instance
    DatabaseManager* db = DatabaseManager::getInstance("data/gym_management.db");
    
    // Test connection
    if (db->connect()) {
        std::cout << "✅ Database connected successfully!" << std::endl;
        
        // Test if tables exist
        if (db->tableExists("customers")) {
            std::cout << "✅ Customers table created" << std::endl;
        }
        if (db->tableExists("check_ins")) {
            std::cout << "✅ Check-ins table created" << std::endl;
        }
        if (db->tableExists("transactions")) {
            std::cout << "✅ Transactions table created" << std::endl;
        }
        if (db->tableExists("inventory")) {
            std::cout << "✅ Inventory table created" << std::endl;
        }
        
        // Test a simple query
        if (db->executeQuery("SELECT COUNT(*) FROM customers;")) {
            std::cout << "✅ Database queries working" << std::endl;
        }
        
        // Keep connection open for the session
    } else {
        std::cout << "❌ Database connection failed!" << std::endl;
        std::cout << "Make sure the 'data' directory exists and is writable." << std::endl;
    }
}

void showMainMenu() {
    std::cout << "==================================" << std::endl;
    std::cout << "   Gym Management System v1.0     " << std::endl;
    std::cout << "   Cross-Platform Compatible      " << std::endl;
    std::cout << "==================================" << std::endl;
    
    std::cout << "\n[1] Customer Management" << std::endl;
    std::cout << "[2] Check In/Out" << std::endl;
    std::cout << "[3] Process Payment" << std::endl;
    std::cout << "[4] Reports" << std::endl;
    std::cout << "[5] Inventory Management" << std::endl;
    std::cout << "[0] Exit" << std::endl;
}

// Add these functions to main.cpp (before main() function)

// Function to get payment type from user
PaymentType getPaymentType() {
    std::cout << "\nPayment Types:" << std::endl;
    std::cout << "[1] Cash" << std::endl;
    std::cout << "[2] M-Pesa" << std::endl;
    
    int choice = getIntInput("Select payment type (1-2): ", 1, 2);
    
    return (choice == 1) ? PaymentType::CASH : PaymentType::MPESA;
}

// Function to get transaction type from user
TransactionType getTransactionType() {
    std::cout << "\nTransaction Types:" << std::endl;
    std::cout << "[1] Membership Payment" << std::endl;
    std::cout << "[2] Daily Fee" << std::endl;
    std::cout << "[3] Renewal" << std::endl;
    std::cout << "[4] Other" << std::endl;
    
    int choice = getIntInput("Select transaction type (1-4): ", 1, 4);
    
    switch (choice) {
        case 1: return TransactionType::MEMBERSHIP_PAYMENT;
        case 2: return TransactionType::DAILY_FEE;
        case 3: return TransactionType::RENEWAL;
        case 4: return TransactionType::OTHER;
        default: return TransactionType::OTHER;
    }
}

// Function to process payments
void processPayment() {
    Customer customer = findCustomer("=== Process Payment ===");
    
    if (customer.getCustomerId() == 0) {
        return;
    }
    
    // Display customer info
    customer.displayInfo();
    
    // Get payment details
    double amount;
    std::cout << "\nEnter payment amount (KES): ";
    while (!(std::cin >> amount) || amount <= 0) {
        std::cout << "❌ Please enter a valid amount: ";
        clearInputBuffer();
    }
    clearInputBuffer();
    
    PaymentType paymentType = getPaymentType();
    TransactionType transactionType = getTransactionType();
    
    std::string description = getStringInput("Description (optional): ", false);
    
    // Create transaction
    Transaction transaction(customer.getCustomerId(), amount, paymentType, transactionType, description);
    
    // Handle M-Pesa specific details
    if (paymentType == PaymentType::MPESA) {
        std::string mpesaCode = getStringInput("M-Pesa confirmation code: ");
        transaction.setMpesaCode(mpesaCode);
    }
    
    // Process the payment
    if (transaction.processPayment()) {
        std::cout << "\n🎉 Payment processed successfully!" << std::endl;
        transaction.displayInfo();
        
        // Update customer balance if needed
        if (transactionType == TransactionType::MEMBERSHIP_PAYMENT || 
            transactionType == TransactionType::RENEWAL) {
            // Reset balance for membership payments
            customer.setBalance(0.0);
            customer.updateInDatabase();
        }
    } else {
        std::cout << "\n❌ Payment processing failed!" << std::endl;
    }
}

// Function to view transaction history
void viewTransactionHistory() {
    std::cout << "\n=== Transaction History ===" << std::endl;
    std::cout << "[1] All Transactions" << std::endl;
    std::cout << "[2] Customer Transactions" << std::endl;
    std::cout << "[3] Today's Revenue" << std::endl;
    
    int choice = getIntInput("Choose option: ", 1, 3);
    
    switch (choice) {
        case 1: {
            // All transactions
            std::vector<Transaction> transactions = Transaction::getAllTransactions();
            
            if (transactions.empty()) {
                std::cout << "No transactions found." << std::endl;
                return;
            }
            
            // Display header
            std::cout << "\n" << std::left 
                      << std::setw(6) << "ID"
                      << std::setw(8) << "Cust ID"
                      << std::setw(12) << "Amount"
                      << std::setw(10) << "Payment"
                      << std::setw(18) << "Type"
                      << std::setw(12) << "Status"
                      << "Date" << std::endl;
            
            std::cout << std::string(80, '-') << std::endl;
            
            for (const auto& transaction : transactions) {
                transaction.displaySummary();
            }
            
            std::cout << "\nTotal transactions: " << transactions.size() << std::endl;
            break;
        }
        
        case 2: {
            // Customer transactions
            Customer customer = findCustomer("=== Customer Transaction History ===");
            
            if (customer.getCustomerId() == 0) {
                return;
            }
            
            std::vector<Transaction> transactions = Transaction::getTransactionsByCustomer(customer.getCustomerId());
            
            if (transactions.empty()) {
                std::cout << "No transactions found for " << customer.getFullName() << std::endl;
                return;
            }
            
            std::cout << "\nTransaction history for: " << customer.getFullName() << std::endl;
            
            // Display header
            std::cout << "\n" << std::left 
                      << std::setw(6) << "ID"
                      << std::setw(12) << "Amount"
                      << std::setw(10) << "Payment"
                      << std::setw(18) << "Type"
                      << std::setw(12) << "Status"
                      << "Date" << std::endl;
            
            std::cout << std::string(70, '-') << std::endl;
            
            double totalPaid = 0.0;
            for (const auto& transaction : transactions) {
                std::cout << std::left 
                          << std::setw(6) << transaction.getTransactionId()
                          << std::setw(12) << transaction.getFormattedAmount()
                          << std::setw(10) << transaction.getPaymentTypeString()
                          << std::setw(18) << transaction.getTransactionTypeString()
                          << std::setw(12) << transaction.getStatusString()
                          << transaction.getFormattedDate().substr(0, 16) << std::endl;
                
                if (transaction.getStatus() == TransactionStatus::COMPLETED) {
                    totalPaid += transaction.getAmount();
                }
            }
            
            std::cout << "\nTotal transactions: " << transactions.size() << std::endl;
            std::cout << "Total paid: KES " << std::fixed << std::setprecision(2) << totalPaid << std::endl;
            break;
        }
        
        case 3: {
            // Today's revenue
            std::time_t now = std::time(nullptr);
            std::tm* today = std::localtime(&now);
            
            // Set to start of day
            today->tm_hour = 0;
            today->tm_min = 0;
            today->tm_sec = 0;
            std::time_t startOfDay = std::mktime(today);
            
            // Set to end of day
            today->tm_hour = 23;
            today->tm_min = 59;
            today->tm_sec = 59;
            std::time_t endOfDay = std::mktime(today);
            
            double todayRevenue = Transaction::getTotalRevenueByDate(startOfDay, endOfDay);
            
            std::cout << "\n📊 Today's Revenue Report" << std::endl;
            std::cout << "Date: " << std::put_time(std::localtime(&now), "%Y-%m-%d") << std::endl;
            std::cout << "Total Revenue: KES " << std::fixed << std::setprecision(2) << todayRevenue << std::endl;
            
            // Get today's transactions for details
            std::vector<Transaction> todayTransactions;
            std::vector<Transaction> allTransactions = Transaction::getAllTransactions();
            
            for (const auto& transaction : allTransactions) {
                if (transaction.getTransactionDate() >= startOfDay && 
                    transaction.getTransactionDate() <= endOfDay &&
                    transaction.getStatus() == TransactionStatus::COMPLETED) {
                    todayTransactions.push_back(transaction);
                }
            }
            
            if (!todayTransactions.empty()) {
                std::cout << "\nToday's completed transactions: " << todayTransactions.size() << std::endl;
                
                // Count by payment type
                int cashCount = 0, mpesaCount = 0;
                double cashAmount = 0.0, mpesaAmount = 0.0;
                
                for (const auto& transaction : todayTransactions) {
                    if (transaction.getPaymentType() == PaymentType::CASH) {
                        cashCount++;
                        cashAmount += transaction.getAmount();
                    } else {
                        mpesaCount++;
                        mpesaAmount += transaction.getAmount();
                    }
                }
                
                std::cout << "Cash: " << cashCount << " transactions, KES " << std::fixed << std::setprecision(2) << cashAmount << std::endl;
                std::cout << "M-Pesa: " << mpesaCount << " transactions, KES " << std::fixed << std::setprecision(2) << mpesaAmount << std::endl;
            } else {
                std::cout << "No completed transactions today." << std::endl;
            }
            
            break;
        }
    }
}

// Function to show payment menu
void showPaymentMenu() {
    int choice;
    do {
        std::cout << "\n=== Payment Management ===" << std::endl;
        std::cout << "[1] Process Payment" << std::endl;
        std::cout << "[2] View Transaction History" << std::endl;
        std::cout << "[3] Daily Revenue Report" << std::endl;
        std::cout << "[4] Delete Transaction History" << std::endl;  // ADD THIS LINE
        std::cout << "[0] Back to Main Menu" << std::endl;
        
        choice = getIntInput("Enter choice: ", 0, 4);  // CHANGE FROM 3 TO 4
        
        switch (choice) {
            case 1:
                processPayment();
                break;
            case 2:
                viewTransactionHistory();
                break;
            case 3: {
                // Quick daily revenue
                std::time_t now = std::time(nullptr);
                std::tm* today = std::localtime(&now);
                
                today->tm_hour = 0;
                today->tm_min = 0;
                today->tm_sec = 0;
                std::time_t startOfDay = std::mktime(today);
                
                today->tm_hour = 23;
                today->tm_min = 59;
                today->tm_sec = 59;
                std::time_t endOfDay = std::mktime(today);
                
                double revenue = Transaction::getTotalRevenueByDate(startOfDay, endOfDay);
                
                std::cout << "\n💰 Today's Revenue: KES " << std::fixed << std::setprecision(2) << revenue << std::endl;
                break;
            }
            case 4:  // ADD THIS CASE
                deleteTransactions();
                break;
            case 0:
                break;
            default:
                std::cout << "❌ Invalid choice!" << std::endl;
                break;
        }
        
        if (choice != 0) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
        
    } while (choice != 0);
}
// Function to show delete transactions menu
void showDeleteTransactionsMenu() {
    int choice;
    do {
        std::cout << "\n=== Delete Transaction History ===" << std::endl;
        std::cout << "⚠️  WARNING: This will permanently delete transaction data!" << std::endl;
        std::cout << "[1] Delete All Transactions" << std::endl;
        std::cout << "[2] Delete Customer's Transactions" << std::endl;
        std::cout << "[3] Delete Single Transaction" << std::endl;
        std::cout << "[4] Delete Today's Transactions" << std::endl;
        std::cout << "[0] Back" << std::endl;
        
        choice = getIntInput("Enter choice: ", 0, 4);
        
        switch (choice) {
            case 1: {
                // Delete all transactions
                std::vector<Transaction> allTransactions = Transaction::getAllTransactions();
                if (allTransactions.empty()) {
                    std::cout << "No transactions to delete." << std::endl;
                    break;
                }
                
                std::cout << "\n⚠️  You are about to delete ALL " << allTransactions.size() << " transactions!" << std::endl;
                std::cout << "This action CANNOT be undone!" << std::endl;
                
                char confirm;
                std::cout << "Type 'Y' to confirm: ";
                std::cin >> confirm;
                clearInputBuffer();
                
                if (confirm == 'Y') {
                    Transaction::deleteAllTransactions();
                } else {
                    std::cout << "❌ Deletion cancelled." << std::endl;
                }
                break;
            }
            
            case 2: {
                // Delete customer transactions
                Customer customer = findCustomer("=== Delete Customer Transactions ===");
                if (customer.getCustomerId() == 0) {
                    break;
                }
                
                std::vector<Transaction> customerTransactions = Transaction::getTransactionsByCustomer(customer.getCustomerId());
                if (customerTransactions.empty()) {
                    std::cout << "No transactions found for " << customer.getFullName() << std::endl;
                    break;
                }
                
                std::cout << "\n⚠️  Delete " << customerTransactions.size() << " transactions for " << customer.getFullName() << "?" << std::endl;
                
                char confirm;
                std::cout << "Type 'Y' to confirm: ";
                std::cin >> confirm;
                clearInputBuffer();
                
                if (confirm == 'Y') {
                    Transaction::deleteTransactionsByCustomer(customer.getCustomerId());
                } else {
                    std::cout << "❌ Deletion cancelled." << std::endl;
                }
                break;
            }
            
            case 3: {
                // Delete single transaction
                int transactionId = getIntInput("Enter Transaction ID to delete: ", 1, 999999);
                
                std::cout << "\n⚠️  Delete transaction ID " << transactionId << "?" << std::endl;
                
                char confirm;
                std::cout << "Type 'Y' to confirm: ";
                std::cin >> confirm;
                clearInputBuffer();
                
                if (confirm == 'Y') {
                    Transaction::deleteTransaction(transactionId);
                } else {
                    std::cout << "❌ Deletion cancelled." << std::endl;
                }
                break;
            }
            
            case 4: {
                // Delete today's transactions
                std::time_t now = std::time(nullptr);
                std::tm* today = std::localtime(&now);
                
                today->tm_hour = 0;
                today->tm_min = 0;
                today->tm_sec = 0;
                std::time_t startOfDay = std::mktime(today);
                
                today->tm_hour = 23;
                today->tm_min = 59;
                today->tm_sec = 59;
                std::time_t endOfDay = std::mktime(today);
                
                std::cout << "\n⚠️  Delete all transactions from today?" << std::endl;
                
                char confirm;
                std::cout << "Type 'Y' to confirm: ";
                std::cin >> confirm;
                clearInputBuffer();
                
                if (confirm == 'Y') {
                    Transaction::deleteTransactionsByDateRange(startOfDay, endOfDay);
                } else {
                    std::cout << "❌ Deletion cancelled." << std::endl;
                }
                break;
            }
            
            case 0:
                break;
                
            default:
                std::cout << "❌ Invalid choice!" << std::endl;
                break;
        }
        
        if (choice != 0) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
        
    } while (choice != 0);
}

// Function to handle transaction deletion
void deleteTransactions() {
    showDeleteTransactionsMenu();
}

// Inventory utility functions
InventoryCategory getInventoryCategory() {
    std::cout << "\nInventory Categories:" << std::endl;
    std::cout << "[1] Equipment (Gym machines, weights, etc.)" << std::endl;
    std::cout << "[2] Cleaning Supplies" << std::endl;
    std::cout << "[3] Maintenance Equipment" << std::endl;
    std::cout << "[4] Drinks & Beverages" << std::endl;
    std::cout << "[5] Supplements" << std::endl;
    std::cout << "[6] Other" << std::endl;
    
    int choice = getIntInput("Select category (1-6): ", 1, 6);
    
    switch (choice) {
        case 1: return InventoryCategory::EQUIPMENT;
        case 2: return InventoryCategory::CLEANING;
        case 3: return InventoryCategory::MAINTENANCE;
        case 4: return InventoryCategory::DRINKS;
        case 5: return InventoryCategory::SUPPLEMENTS;
        case 6: return InventoryCategory::OTHER;
        default: return InventoryCategory::OTHER;
    }
}

EquipmentCondition getEquipmentCondition() {
    std::cout << "\nEquipment Conditions:" << std::endl;
    std::cout << "[1] Excellent (Like new)" << std::endl;
    std::cout << "[2] Good (Minor wear, fully functional)" << std::endl;
    std::cout << "[3] Fair (Moderate wear, needs attention)" << std::endl;
    std::cout << "[4] Poor (Heavy wear, needs repair)" << std::endl;
    std::cout << "[5] Out of Order (Not functional)" << std::endl;
    
    int choice = getIntInput("Select condition (1-5): ", 1, 5);
    
    switch (choice) {
        case 1: return EquipmentCondition::EXCELLENT;
        case 2: return EquipmentCondition::GOOD;
        case 3: return EquipmentCondition::FAIR;
        case 4: return EquipmentCondition::POOR;
        case 5: return EquipmentCondition::OUT_OF_ORDER;
        default: return EquipmentCondition::EXCELLENT;
    }
}

Inventory selectInventoryItem(const std::vector<Inventory>& items) {
    if (items.empty()) {
        return Inventory(); // Return empty item
    }
    
    if (items.size() == 1) {
        std::cout << "Item found:" << std::endl;
        items[0].displaySummary();
        return items[0];
    }
    
    std::cout << "\nMultiple items found:" << std::endl;
    std::cout << std::left << std::setw(4) << "No."
              << std::setw(25) << "Name"
              << std::setw(12) << "Category"
              << std::setw(8) << "Qty"
              << "Status" << std::endl;
    std::cout << std::string(55, '-') << std::endl;
    
    for (size_t i = 0; i < items.size(); ++i) {
        std::cout << std::left << std::setw(4) << (i + 1);
        items[i].displaySummary();
    }
    
    int choice = getIntInput("Select item (1-" + std::to_string(items.size()) + ") or 0 to cancel: ", 0, static_cast<int>(items.size()));
    
    if (choice == 0) {
        return Inventory(); // Return empty item
    }
    
    return items[choice - 1];
}

void addInventoryItem() {
    std::cout << "\n=== Add New Inventory Item ===" << std::endl;
    
    std::string itemName = getStringInput("Item Name: ");
    InventoryCategory category = getInventoryCategory();
    
    int quantity = getIntInput("Initial Quantity: ", 0, 10000);
    
    double costPrice = 0.0;
    std::cout << "Cost Price (KES, press Enter for 0): ";
    std::string priceStr;
    std::getline(std::cin, priceStr);
    if (!priceStr.empty()) {
        try {
            costPrice = std::stod(priceStr);
        } catch (...) {
            costPrice = 0.0;
        }
    }
    
    std::string supplier = getStringInput("Supplier (optional): ", false);
    
    // Create new inventory item
    Inventory newItem(itemName, category, quantity, costPrice);
    if (!supplier.empty()) {
        newItem.setSupplier(supplier);
    }
    
    // Set category-specific details
    if (category == InventoryCategory::EQUIPMENT) {
        std::cout << "\nFor equipment, set initial condition:" << std::endl;
        EquipmentCondition condition = getEquipmentCondition();
        newItem.setCondition(condition);
    } else {
        // For consumables, set original quantity and optional expiry
        newItem.setOriginalQuantity(quantity);
        
        char hasExpiry;
        std::cout << "Does this item have an expiry date? (y/n): ";
        std::cin >> hasExpiry;
        clearInputBuffer();
        
        if (hasExpiry == 'y' || hasExpiry == 'Y') {
            std::cout << "Enter expiry date (YYYY-MM-DD): ";
            std::string dateStr;
            std::getline(std::cin, dateStr);
            
            // Basic date parsing
            std::tm tm = {};
            if (strptime(dateStr.c_str(), "%Y-%m-%d", &tm)) {
                std::time_t expiryTime = mktime(&tm);
                newItem.setExpiryDate(expiryTime);
            } else {
                std::cout << "⚠️ Invalid date format, skipping expiry date" << std::endl;
            }
        }
    }
    
    std::string notes = getStringInput("Notes (optional): ", false);
    if (!notes.empty()) {
        newItem.setNotes(notes);
    }
    
    // Save to database
    if (newItem.saveToDatabase()) {
        std::cout << "\n✅ Inventory item added successfully!" << std::endl;
        newItem.displayInfo();
    } else {
        std::cout << "\n❌ Failed to add inventory item." << std::endl;
    }
}

void viewInventoryList() {
    std::cout << "\n=== Inventory List ===" << std::endl;
    
    std::vector<Inventory> items = Inventory::getAllItems();
    
    if (items.empty()) {
        std::cout << "No inventory items found." << std::endl;
        return;
    }
    
    // Display header
    std::cout << std::left << std::setw(4) << "ID"
              << std::setw(25) << "Name"
              << std::setw(12) << "Category"
              << std::setw(8) << "Qty"
              << "Status/Usage" << std::endl;
    
    std::cout << std::string(60, '-') << std::endl;
    
    // Display items
    for (const auto& item : items) {
        item.displaySummary();
    }
    
    std::cout << "\nTotal items: " << items.size() << std::endl;
}

void searchInventoryItem() {
    std::string searchTerm = getStringInput("\n=== Search Inventory ===\nEnter search term (name, supplier, or notes): ");
    
    std::vector<Inventory> items = Inventory::searchItems(searchTerm);
    
    if (items.empty()) {
        std::cout << "❌ No items found matching: " << searchTerm << std::endl;
        return;
    }
    
    Inventory selectedItem = selectInventoryItem(items);
    
    if (selectedItem.getItemId() != 0) {
        selectedItem.displayInfo();
    }
}

void updateInventoryStock() {
    std::cout << "\n=== Update Stock ===" << std::endl;
    std::cout << "[1] Search by Name" << std::endl;
    std::cout << "[2] Browse by Category" << std::endl;
    
    int searchType = getIntInput("Choose method: ", 1, 2);
    
    std::vector<Inventory> items;
    
    if (searchType == 1) {
        std::string searchTerm = getStringInput("Enter item name: ");
        items = Inventory::searchItems(searchTerm);
    } else {
        InventoryCategory category = getInventoryCategory();
        items = Inventory::getItemsByCategory(category);
    }
    
    if (items.empty()) {
        std::cout << "❌ No items found." << std::endl;
        return;
    }
    
    Inventory selectedItem = selectInventoryItem(items);
    
    if (selectedItem.getItemId() == 0) {
        return;
    }
    
    selectedItem.displayInfo();
    
    std::cout << "\nStock Operations:" << std::endl;
    std::cout << "[1] Add Stock" << std::endl;
    std::cout << "[2] Reduce Stock (Usage)" << std::endl;
    std::cout << "[0] Cancel" << std::endl;
    
    int operation = getIntInput("Select operation: ", 0, 2);
    
    if (operation == 0) return;
    
    int amount = getIntInput("Enter quantity: ", 1, 10000);
    std::string note = getStringInput("Enter note (optional): ", false);
    
    bool success = false;
    if (operation == 1) {
        success = selectedItem.addStock(amount, note);
        if (success) {
            std::cout << "✅ Stock added successfully!" << std::endl;
        }
    } else {
        success = selectedItem.reduceStock(amount, note);
        if (success) {
            std::cout << "✅ Stock reduced successfully!" << std::endl;
        }
    }
    
    if (success) {
        selectedItem.displayInfo();
    }
}

void updateInventoryCondition() {
    std::cout << "\n=== Update Equipment Condition ===" << std::endl;
    
    std::vector<Inventory> equipment = Inventory::getItemsByCategory(InventoryCategory::EQUIPMENT);
    
    if (equipment.empty()) {
        std::cout << "❌ No equipment found." << std::endl;
        return;
    }
    
    Inventory selectedItem = selectInventoryItem(equipment);
    
    if (selectedItem.getItemId() == 0) {
        return;
    }
    
    selectedItem.displayInfo();
    
    std::cout << "\nCondition Operations:" << std::endl;
    std::cout << "[1] Update Condition" << std::endl;
    std::cout << "[2] Record Maintenance" << std::endl;
    std::cout << "[0] Cancel" << std::endl;
    
    int operation = getIntInput("Select operation: ", 0, 2);
    
    if (operation == 0) return;
    
    if (operation == 1) {
        std::cout << "\nCurrent condition: " << selectedItem.getConditionString() << std::endl;
        EquipmentCondition newCondition = getEquipmentCondition();
        std::string note = getStringInput("Enter note about condition change: ", false);
        
        if (selectedItem.updateCondition(newCondition, note)) {
            std::cout << "✅ Condition updated successfully!" << std::endl;
        }
    } else {
        std::string note = getStringInput("Enter maintenance details: ");
        
        if (selectedItem.recordMaintenance(note)) {
            std::cout << "✅ Maintenance recorded successfully!" << std::endl;
        }
    }
}

void deleteInventoryItem() {
    std::string searchTerm = getStringInput("\n=== Delete Inventory Item ===\nEnter item name to search: ");
    
    std::vector<Inventory> items = Inventory::searchItems(searchTerm);
    
    if (items.empty()) {
        std::cout << "❌ No items found matching: " << searchTerm << std::endl;
        return;
    }
    
    Inventory selectedItem = selectInventoryItem(items);
    
    if (selectedItem.getItemId() == 0) {
        return;
    }
    
    // Display item info for confirmation
    selectedItem.displayInfo();
    
    // Confirmation prompt
    std::cout << "\n⚠️ WARNING: This will permanently delete this inventory item!" << std::endl;
    
    char confirm1;
    std::cout << "\nAre you sure you want to delete '" << selectedItem.getItemName() << "'? (y/n): ";
    std::cin >> confirm1;
    clearInputBuffer();
    
    if (confirm1 != 'y' && confirm1 != 'Y') {
        std::cout << "❌ Item deletion cancelled." << std::endl;
        return;
    }
    
    // Final confirmation
    char confirm2;
    std::cout << "⚠️ FINAL CONFIRMATION: Type 'Y' to permanently delete this item: ";
    std::cin >> confirm2;
    clearInputBuffer();
    
    if (confirm2 != 'Y') {
        std::cout << "❌ Item deletion cancelled." << std::endl;
        return;
    }
    
    // Proceed with deletion
    if (selectedItem.deleteFromDatabase()) {
        std::cout << "✅ Item '" << selectedItem.getItemName() << "' has been permanently deleted." << std::endl;
    } else {
        std::cout << "❌ Failed to delete item. Please try again." << std::endl;
    }
}

void showInventoryByCategory() {
    std::cout << "\n=== View Inventory by Category ===" << std::endl;
    
    InventoryCategory category = getInventoryCategory();
    std::vector<Inventory> items = Inventory::getItemsByCategory(category);
    
    if (items.empty()) {
        std::cout << "No items found in this category." << std::endl;
        return;
    }
    
    std::cout << "\n=== " << items[0].getCategoryString() << " Items ===" << std::endl;
    
    // Display header
    std::cout << std::left << std::setw(4) << "ID"
              << std::setw(25) << "Name"
              << std::setw(8) << "Qty"
              << "Status/Usage" << std::endl;
    
    std::cout << std::string(45, '-') << std::endl;
    
    // Display items
    for (const auto& item : items) {
        item.displaySummary();
    }
    
    std::cout << "\nTotal " << items[0].getCategoryString() << " items: " << items.size() << std::endl;
}

void showInventoryAlerts() {
    std::cout << "\n=== Inventory Alerts ===" << std::endl;
    
    std::vector<Inventory> allItems = Inventory::getAllItems();
    
    if (allItems.empty()) {
        std::cout << "No inventory items found." << std::endl;
        return;
    }
    
    bool hasAlerts = false;
    
    // Check for low stock (quantity <= 5)
    std::cout << "🔻 LOW STOCK ALERTS:" << std::endl;
    for (const auto& item : allItems) {
        if (item.getQuantity() <= 5) {
            std::cout << "   • " << item.getItemName() 
                      << " (" << item.getCategoryString() << ") - Only " 
                      << item.getQuantity() << " left" << std::endl;
            hasAlerts = true;
        }
    }
    if (!hasAlerts) {
        std::cout << "   No low stock items." << std::endl;
    }
    hasAlerts = false;
    
    // Check for expired items
    std::cout << "\n📅 EXPIRY ALERTS:" << std::endl;
    for (const auto& item : allItems) {
        if (item.isExpired()) {
            std::cout << "   • " << item.getItemName() 
                      << " (" << item.getCategoryString() << ") - EXPIRED!" << std::endl;
            hasAlerts = true;
        }
    }
    if (!hasAlerts) {
        std::cout << "   No expired items." << std::endl;
   }
   hasAlerts = false;
   
   // Check for equipment needing maintenance
   std::cout << "\n🔧 MAINTENANCE ALERTS:" << std::endl;
   for (const auto& item : allItems) {
       if (item.getCategory() == InventoryCategory::EQUIPMENT) {
           if (item.needsMaintenance()) {
               std::cout << "   • " << item.getItemName() 
                         << " - Maintenance due" << std::endl;
               hasAlerts = true;
           }
           if (item.getCondition() == EquipmentCondition::POOR || 
               item.getCondition() == EquipmentCondition::OUT_OF_ORDER) {
               std::cout << "   • " << item.getItemName() 
                         << " - Condition: " << item.getConditionString() << std::endl;
               hasAlerts = true;
           }
       }
   }
   if (!hasAlerts) {
       std::cout << "   No maintenance alerts." << std::endl;
   }
}

void showInventoryMenu() {
   int choice;
   do {
       std::cout << "\n=== Inventory Management ===" << std::endl;
       std::cout << "[1] Add New Item" << std::endl;
       std::cout << "[2] View All Items" << std::endl;
       std::cout << "[3] Search Item" << std::endl;
       std::cout << "[4] Update Stock" << std::endl;
       std::cout << "[5] Update Equipment Condition" << std::endl;
       std::cout << "[6] View by Category" << std::endl;
       std::cout << "[7] View Alerts (Low Stock, Expiry, Maintenance)" << std::endl;
       std::cout << "[8] Edit Item Details" << std::endl;
       std::cout << "[9] Delete Item" << std::endl;
       std::cout << "[0] Back to Main Menu" << std::endl;
       
       choice = getIntInput("Enter choice: ", 0, 9);
       
       switch (choice) {
           case 1:
               addInventoryItem();
               break;
           case 2:
               viewInventoryList();
               break;
           case 3:
               searchInventoryItem();
               break;
           case 4:
               updateInventoryStock();
               break;
           case 5:
               updateInventoryCondition();
               break;
           case 6:
               showInventoryByCategory();
               break;
           case 7:
               showInventoryAlerts();
               break;
           case 8: {
               std::string searchTerm = getStringInput("Enter item name to edit: ");
               std::vector<Inventory> items = Inventory::searchItems(searchTerm);
               
               if (items.empty()) {
                   std::cout << "❌ No items found." << std::endl;
               } else {
                   Inventory selectedItem = selectInventoryItem(items);
                   if (selectedItem.getItemId() != 0) {
                       selectedItem.editDetails();
                   }
               }
               break;
           }
           case 9:
               deleteInventoryItem();
               break;
           case 0:
               break;
           default:
               std::cout << "❌ Invalid choice!" << std::endl;
               break;
       }
       
       if (choice != 0) {
           std::cout << "\nPress Enter to continue...";
           std::cin.get();
       }
       
   } while (choice != 0);
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "   Gym Management System v1.0     " << std::endl;
    std::cout << "   Cross-Platform Compatible      " << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;

    std::cout << "System initialized successfully!" << std::endl;
    
    #ifdef __aarch64__
        std::cout << "Architecture: ARM64 (M3 Optimized)" << std::endl;
    #endif
    
    // Test database connection and keep it open
    testDatabaseConnection();
    
    // Main application loop
    int choice;
    do {
        showMainMenu();
        choice = getIntInput("Enter choice: ", 0, 5);
        
        switch (choice) {
            case 1:
                showCustomerMenu();
                break;
            case 2:
                showCheckInOutMenu();
                break;
            case 3:
                showPaymentMenu();
                break;
            case 4:
                Reports::showReportsMenu();
                break;
            case 5:
                showInventoryMenu();
                break;
            case 0:
                std::cout << "\nThank you for using Gym Management System! 👋" << std::endl;
                break;
            default:
                std::cout << "\n❌ Invalid choice. Please try again." << std::endl;
                break;
        }
        
        if (choice != 0 && choice >= 3 && choice <= 5) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
        
    } while (choice != 0);
    
    // Cleanup - disconnect database
    DatabaseManager* db = DatabaseManager::getInstance();
    if (db) {
        db->disconnect();
    }
    
    return 0;
}