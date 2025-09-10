#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

#include <wx/defs.h>
#include <string>

/**
 * Application-wide constants and enumerations for the Gym Management System
 * Contains UI control IDs, business logic enums, and helper classes for type conversion
 */

// ============================================================================
// UI CONTROL IDs - Unique identifiers for wxWidgets controls
// ============================================================================
enum {
    // Menu and toolbar IDs
    ID_REFRESH_DATA = wxID_HIGHEST + 1,
    ID_BACKUP_DB,
    ID_RESTORE_DB,
    ID_SETTINGS,
    ID_VIEW_REPORTS,
    ID_REFRESH_TIMER,

    // Customer management IDs
    ID_NEW_CUSTOMER,
    ID_EDIT_CUSTOMER,
    ID_DELETE_CUSTOMER,
    ID_RENEW_MEMBERSHIP,

    // Transaction and payment IDs
    ID_QUICK_CHECKIN,
    ID_DAILY_PAYMENT,
    ID_PROCESS_PAYMENT,
    ID_REFUND_PAYMENT,

    // Filter and search IDs
    ID_MEMBERSHIP_FILTER,
    ID_STATUS_FILTER,
    ID_MEMBERSHIP_CHOICE,
    ID_SEARCH_CUSTOMER,
    ID_CLEAR_SEARCH,

    // Report IDs
    ID_DAILY_REPORT,
    ID_MONTHLY_REPORT,
    ID_REVENUE_REPORT,
    ID_EXPORT_REPORT
};

// ============================================================================
// BUSINESS LOGIC ENUMERATIONS
// ============================================================================

/**
 * MembershipType - Types of gym memberships available
 * Used for pricing, duration calculation, and customer categorization
 */
enum class MembershipType {
    DAILY_GYM = 0,           // Regular gym access - KES 400/200 (student)
    DAILY_PERSONAL_TRAINING = 1,  // Gym + personal training - KES 500
    DAILY_MARTIAL_ARTS = 2,       // Martial arts training - KES 500/300 (student)
    WEEKLY = 3,              // 7-day membership - KES 800
    MONTHLY = 4,             // 30-day membership - KES 4500/3000 (student)
    TRAINER = 5              // Special trainer membership (no expiry)
};

/**
 * CustomerCategory - Special pricing categories
 * Used to apply discounts for students and minors
 */
enum class CustomerCategory {
    REGULAR = 0,    // Standard pricing
    STUDENT = 1,    // Student discount pricing
    MINOR = 2       // Minor discount pricing (same as student)
};

/**
 * CustomerStatus - Current status of customer membership
 * Automatically calculated based on membership type and expiry date
 */
enum class CustomerStatus {
    ACTIVE = 0,   // Membership is currently valid
    EXPIRED = 1   // Membership has expired
};

/**
 * PaymentType - Methods of payment accepted by the gym
 * Used for transaction processing and reporting
 */
enum class PaymentType {
    CASH = 0,     // Cash payment
    MPESA = 1     // M-Pesa mobile money payment
};

/**
 * TransactionType - Categories of financial transactions
 * Used for accounting, reporting, and business analytics
 */
enum class TransactionType {
    MEMBERSHIP_PAYMENT = 0,  // Initial membership purchase
    DAILY_FEE = 1,          // Daily gym access fee
    RENEWAL = 2,            // Membership renewal
    REFUND = 3,             // Payment refund
    OTHER = 4               // Miscellaneous transactions
};

/**
 * TransactionStatus - Current state of a transaction
 * Used for payment processing and financial tracking
 */
enum class TransactionStatus {
    COMPLETED = 0,  // Transaction successfully completed
    PENDING = 1,    // Transaction awaiting completion
    FAILED = 2,     // Transaction failed to process
    REFUNDED = 3    // Transaction has been refunded
};

// ============================================================================
// HELPER CLASSES FOR ENUM CONVERSION
// ============================================================================

/**
 * MembershipTypeHelper - Utility class for MembershipType enum operations
 * Provides string conversion and validation for membership types
 */
class MembershipTypeHelper {
public:
    /**
     * Convert MembershipType enum to display string
     * @param type The membership type to convert
     * @return String representation suitable for UI display
     */
    static std::string toString(MembershipType type) {
        switch (type) {
            case MembershipType::DAILY_GYM:              return "Daily Gym";
            case MembershipType::DAILY_PERSONAL_TRAINING: return "Daily Personal Training";
            case MembershipType::DAILY_MARTIAL_ARTS:     return "Daily Martial Arts";
            case MembershipType::WEEKLY:                 return "Weekly";
            case MembershipType::MONTHLY:                return "Monthly";
            case MembershipType::TRAINER:                return "Trainer";
            default: return "Daily Gym"; // Default fallback
        }
    }
    
    /**
     * Convert string to MembershipType enum (case-insensitive)
     * @param str String representation of membership type
     * @return Corresponding MembershipType enum value
     */
    static MembershipType fromString(const std::string& str) {
        if (str == "daily_gym" || str == "Daily Gym" || str == "DAILY_GYM") {
            return MembershipType::DAILY_GYM;
        }
        if (str == "daily_personal_training" || str == "Daily Personal Training" || str == "DAILY_PERSONAL_TRAINING") {
            return MembershipType::DAILY_PERSONAL_TRAINING;
        }
        if (str == "daily_martial_arts" || str == "Daily Martial Arts" || str == "DAILY_MARTIAL_ARTS") {
            return MembershipType::DAILY_MARTIAL_ARTS;
        }
        if (str == "weekly" || str == "Weekly" || str == "WEEKLY") {
            return MembershipType::WEEKLY;
        }
        if (str == "monthly" || str == "Monthly" || str == "MONTHLY") {
            return MembershipType::MONTHLY;
        }
        if (str == "trainer" || str == "Trainer" || str == "TRAINER") {
            return MembershipType::TRAINER;
        }
        return MembershipType::DAILY_GYM; // Default fallback
    }
    
    /**
     * Get membership duration in days
     * @param type The membership type
     * @return Duration in days (-1 for unlimited/trainer)
     */
    static int getDurationInDays(MembershipType type) {
        switch (type) {
            case MembershipType::DAILY_GYM:
            case MembershipType::DAILY_PERSONAL_TRAINING:
            case MembershipType::DAILY_MARTIAL_ARTS:     return 1;
            case MembershipType::WEEKLY:                 return 7;
            case MembershipType::MONTHLY:                return 30;
            case MembershipType::TRAINER:                return -1; // Unlimited
            default: return 1;
        }
    }
    
    /**
     * Check if membership type has expiry date
     * @param type The membership type to check
     * @return True if membership expires, false otherwise
     */
    static bool hasExpiry(MembershipType type) {
        return type == MembershipType::WEEKLY || type == MembershipType::MONTHLY;
    }
    
    /**
     * Check if membership type is daily-based
     * @param type The membership type to check
     * @return True if membership is daily-based, false otherwise
     */
    static bool isDailyType(MembershipType type) {
        return type == MembershipType::DAILY_GYM || 
               type == MembershipType::DAILY_PERSONAL_TRAINING || 
               type == MembershipType::DAILY_MARTIAL_ARTS;
    }
    
    /**
     * Get standard pricing for membership type
     * @param type The membership type
     * @return Standard price in KES
     */
    static double getStandardPrice(MembershipType type) {
        switch (type) {
            case MembershipType::DAILY_GYM:              return 400.0;
            case MembershipType::DAILY_PERSONAL_TRAINING: return 500.0;
            case MembershipType::DAILY_MARTIAL_ARTS:     return 500.0;
            case MembershipType::WEEKLY:                 return 1500.0;
            case MembershipType::MONTHLY:                return 4500.0;
            case MembershipType::TRAINER:                return 100.0;
            default: return 400.0;
        }
    }
    
    /**
     * Get discounted pricing for students/minors
     * @param type The membership type
     * @return Discounted price in KES
     */
    static double getDiscountedPrice(MembershipType type) {
        switch (type) {
            case MembershipType::DAILY_GYM:              return 200.0;
            case MembershipType::DAILY_PERSONAL_TRAINING: return 500.0;
            case MembershipType::DAILY_MARTIAL_ARTS:     return 300.0;
            case MembershipType::WEEKLY:                 return 1500.0;  // No weekly discount
            case MembershipType::MONTHLY:                return 3000.0;
            case MembershipType::TRAINER:                return 100.0;  // No trainer discount
            default: return 200.0;
        }
    }
};

/**
 * Check if customer category qualifies for discount
 * Note: Discounts apply to gym, martial arts, and monthly memberships only
 * @param category The customer category to check
 * @return True if discount applies, false otherwise
 */
class CustomerCategoryHelper {
public:
    /**
     * Convert CustomerCategory enum to display string
     * @param category The customer category to convert
     * @return String representation suitable for UI display
     */
    static std::string toString(CustomerCategory category) {
        switch (category) {
            case CustomerCategory::REGULAR: return "Regular";
            case CustomerCategory::STUDENT: return "Student";
            case CustomerCategory::MINOR:   return "Minor";
            default: return "Regular";
        }
    }
    
    /**
     * Convert string to CustomerCategory enum (case-insensitive)
     * @param str String representation of customer category
     * @return Corresponding CustomerCategory enum value
     */
    static CustomerCategory fromString(const std::string& str) {
        if (str == "student" || str == "Student" || str == "STUDENT") {
            return CustomerCategory::STUDENT;
        }
        if (str == "minor" || str == "Minor" || str == "MINOR") {
            return CustomerCategory::MINOR;
        }
        return CustomerCategory::REGULAR;
    }
    
    /**
     * Check if customer category qualifies for discount
     * @param category The customer category to check
     * @return True if discount applies, false otherwise
     */
    static bool hasDiscount(CustomerCategory category) {
        return category == CustomerCategory::STUDENT || category == CustomerCategory::MINOR;
    }
};

/**
 * CustomerStatusHelper - Utility class for CustomerStatus enum operations
 */
class CustomerStatusHelper {
public:
    /**
     * Convert CustomerStatus enum to display string
     * @param status The customer status to convert
     * @return String representation suitable for UI display
     */
    static std::string toString(CustomerStatus status) {
        switch (status) {
            case CustomerStatus::ACTIVE:  return "Active";
            case CustomerStatus::EXPIRED: return "Expired";
            default: return "Active"; // Default fallback
        }
    }
    
    /**
     * Convert string to CustomerStatus enum (case-insensitive)
     * @param str String representation of customer status
     * @return Corresponding CustomerStatus enum value
     */
    static CustomerStatus fromString(const std::string& str) {
        if (str == "expired" || str == "Expired" || str == "EXPIRED") {
            return CustomerStatus::EXPIRED;
        }
        return CustomerStatus::ACTIVE; // Default fallback
    }
};

/**
 * PaymentTypeHelper - Utility class for PaymentType enum operations
 */
class PaymentTypeHelper {
public:
    /**
     * Convert PaymentType enum to display string
     * @param type The payment type to convert
     * @return String representation suitable for UI display
     */
    static std::string toString(PaymentType type) {
        switch (type) {
            case PaymentType::CASH:  return "Cash";
            case PaymentType::MPESA: return "M-Pesa";
            default: return "Cash"; // Default fallback
        }
    }
    
    /**
     * Convert string to PaymentType enum (case-insensitive)
     * @param str String representation of payment type
     * @return Corresponding PaymentType enum value
     */
    static PaymentType fromString(const std::string& str) {
        if (str == "M-Pesa" || str == "MPESA" || str == "mpesa" || 
            str == "m-pesa" || str == "Mpesa") {
            return PaymentType::MPESA;
        }
        return PaymentType::CASH; // Default fallback
    }
    
    /**
     * Check if payment type requires additional verification
     * @param type The payment type to check
     * @return True if verification required (M-Pesa code), false otherwise
     */
    static bool requiresVerification(PaymentType type) {
        return type == PaymentType::MPESA;
    }
};

/**
 * TransactionTypeHelper - Utility class for TransactionType enum operations
 */
class TransactionTypeHelper {
public:
    /**
     * Convert TransactionType enum to display string
     * @param type The transaction type to convert
     * @return String representation suitable for UI display
     */
    static std::string toString(TransactionType type) {
        switch (type) {
            case TransactionType::MEMBERSHIP_PAYMENT: return "Membership Payment";
            case TransactionType::DAILY_FEE:         return "Daily Fee";
            case TransactionType::RENEWAL:           return "Renewal";
            case TransactionType::REFUND:            return "Refund";
            case TransactionType::OTHER:             return "Other";
            default: return "Other";
        }
    }
    
    /**
     * Convert string to TransactionType enum
     * @param str String representation of transaction type
     * @return Corresponding TransactionType enum value
     */
    static TransactionType fromString(const std::string& str) {
        if (str == "Membership Payment" || str == "membership_payment") {
            return TransactionType::MEMBERSHIP_PAYMENT;
        }
        if (str == "Daily Fee" || str == "daily_fee") {
            return TransactionType::DAILY_FEE;
        }
        if (str == "Renewal" || str == "renewal") {
            return TransactionType::RENEWAL;
        }
        if (str == "Refund" || str == "refund") {
            return TransactionType::REFUND;
        }
        return TransactionType::OTHER;
    }
};

/**
 * TransactionStatusHelper - Utility class for TransactionStatus enum operations
 */
class TransactionStatusHelper {
public:
    /**
     * Convert TransactionStatus enum to display string
     * @param status The transaction status to convert
     * @return String representation suitable for UI display
     */
    static std::string toString(TransactionStatus status) {
        switch (status) {
            case TransactionStatus::COMPLETED: return "Completed";
            case TransactionStatus::PENDING:   return "Pending";
            case TransactionStatus::FAILED:    return "Failed";
            case TransactionStatus::REFUNDED:  return "Refunded";
            default: return "Pending";
        }
    }
    
    /**
     * Convert string to TransactionStatus enum
     * @param str String representation of transaction status
     * @return Corresponding TransactionStatus enum value
     */
    static TransactionStatus fromString(const std::string& str) {
        if (str == "Completed" || str == "completed") {
            return TransactionStatus::COMPLETED;
        }
        if (str == "Failed" || str == "failed") {
            return TransactionStatus::FAILED;
        }
        if (str == "Refunded" || str == "refunded") {
            return TransactionStatus::REFUNDED;
        }
        return TransactionStatus::PENDING;
    }
    
    /**
     * Check if transaction status allows modification
     * @param status The transaction status to check
     * @return True if transaction can be modified, false otherwise
     */
    static bool canModify(TransactionStatus status) {
        return status == TransactionStatus::PENDING;
    }
};

// ============================================================================
// APPLICATION CONSTANTS
// ============================================================================

namespace AppConfig {
    // Database configuration
    const std::string DATABASE_NAME = "gym_management.db";
    const std::string DATABASE_VERSION = "1.0";
    
    // UI configuration
    const int DEFAULT_WINDOW_WIDTH = 1200;
    const int DEFAULT_WINDOW_HEIGHT = 800;
    const int MIN_WINDOW_WIDTH = 800;
    const int MIN_WINDOW_HEIGHT = 600;
    
    // Business logic constants - Standard pricing
    const double DEFAULT_DAILY_GYM_FEE = 400.0;                    // KES - Regular gym access
    const double DEFAULT_DAILY_PERSONAL_TRAINING_FEE = 500.0;      // KES - Gym + personal training
    const double DEFAULT_DAILY_MARTIAL_ARTS_FEE = 500.0;           // KES - Martial arts training
    const double DEFAULT_WEEKLY_FEE = 1500.0;                       // KES
    const double DEFAULT_MONTHLY_FEE = 4500.0;                     // KES
    const double DEFAULT_TRAINER_FEE = 100.0;                      // KES

    // Student/Minor discount pricing
    const double STUDENT_DAILY_GYM_FEE = 200.0;                    // KES - 50% discount
    const double STUDENT_DAILY_PERSONAL_TRAINING_FEE = 500.0;      // No discount
    const double STUDENT_DAILY_MARTIAL_ARTS_FEE = 300.0;           // KES - 40% discount
    const double STUDENT_MONTHLY_FEE = 3000.0;                     // KES - ~33% discount
    // No student discounts for weekly and trainer memberships
    
    // Search and pagination
    const int MAX_SEARCH_RESULTS = 1000;
    const int DEFAULT_PAGE_SIZE = 50;
    
    // Phone number validation (Kenyan format)
    const std::string PHONE_REGEX = "^(\\+254|254|0)[17]\\d{8}$";
    const int PHONE_MIN_LENGTH = 9;
    const int PHONE_MAX_LENGTH = 13;
}

#endif // APP_CONSTANTS_H