
// File: src/Reports.cpp
#include "Reports.h"
#include "DatabaseManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <algorithm>
#include <set>
#include <limits>
#ifdef _WIN32
    #include <windows.h>  // Add this for Sleep() function
    #define strptime(s, f, tm) sscanf(s, "%d-%d-%d", &(tm)->tm_year, &(tm)->tm_mon, &(tm)->tm_mday); \
        (tm)->tm_year -= 1900; (tm)->tm_mon -= 1
#else
    #include <unistd.h>   // Add this for sleep() function on Unix/Linux/Mac
#endif

// Utility Functions Implementation
std::time_t Reports::getStartOfDay(std::time_t date) {
    std::tm* tm = std::localtime(&date);
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    return std::mktime(tm);
}

std::time_t Reports::getEndOfDay(std::time_t date) {
    std::tm* tm = std::localtime(&date);
    tm->tm_hour = 23;
    tm->tm_min = 59;
    tm->tm_sec = 59;
    return std::mktime(tm);
}

std::string Reports::formatDate(std::time_t date) {
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&date), "%Y-%m-%d");
    return oss.str();
}

std::string Reports::formatCurrency(double amount) {
    std::ostringstream oss;
    oss << "KES " << std::fixed << std::setprecision(2) << amount;
    return oss.str();
}

std::string Reports::formatPercentage(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value << "%";
    return oss.str();
}

void Reports::displaySeparator(int length) {
    std::cout << std::string(length, '=') << std::endl;
}

void Reports::displayReportHeader(const std::string& title, std::time_t date) {
    displaySeparator(60);
    std::cout << "ðŸ“Š " << title;
    if (date != 0) {
        std::cout << " - " << formatDate(date);
    }
    std::cout << std::endl;
    displaySeparator(60);
}

// Data Collection Functions
RevenueBreakdown Reports::getRevenueData(std::time_t startDate, std::time_t endDate) {
    RevenueBreakdown breakdown = {0.0, 0.0, 0.0, 0, 0, 0, 0.0};
    
    std::vector<Transaction> transactions = Transaction::getAllTransactions();
    
    for (const auto& transaction : transactions) {
        if (transaction.getTransactionDate() >= startDate && 
            transaction.getTransactionDate() <= endDate &&
            transaction.getStatus() == TransactionStatus::COMPLETED) {
            
            double amount = transaction.getAmount();
            breakdown.totalRevenue += amount;
            breakdown.totalTransactions++;
            
            if (transaction.getPaymentType() == PaymentType::CASH) {
                breakdown.cashRevenue += amount;
                breakdown.cashTransactions++;
            } else {
                breakdown.mpesaRevenue += amount;
                breakdown.mpesaTransactions++;
            }
        }
    }
    
    if (breakdown.totalTransactions > 0) {
        breakdown.averageTransaction = breakdown.totalRevenue / breakdown.totalTransactions;
    }
    
    return breakdown;
}

MembershipBreakdown Reports::getMembershipData(std::time_t date) {
    MembershipBreakdown breakdown = {0, 0, 0, 0, 0, 0, 0, 0};
    
    std::vector<Customer> customers = Customer::getAllCustomers();
    std::time_t weekFromNow = date + (7 * 24 * 60 * 60); // 7 days from date
    
    for (const auto& customer : customers) {
        breakdown.totalMembers++;
        
        // Count by membership type
        switch (customer.getMembershipType()) {
            case MembershipType::DAILY:
                breakdown.dailyMembers++;
                break;
            case MembershipType::WEEKLY:
                breakdown.weeklyMembers++;
                break;
            case MembershipType::MONTHLY:
                breakdown.monthlyMembers++;
                break;
            case MembershipType::TRAINER:
                breakdown.trainerMembers++;
                break;
        }
        
        // Check if membership is expired or expiring
        if (customer.getMembershipType() != MembershipType::DAILY) {
            std::time_t expiryDate = customer.getExpiryDate();
            
            if (expiryDate < date) {
                breakdown.expiredMembers++;
            } else if (expiryDate <= weekFromNow) {
                breakdown.expiringInWeek++;
                breakdown.activeMembers++;
            } else {
                breakdown.activeMembers++;
            }
        } else {
            // Daily members are always considered active
            breakdown.activeMembers++;
        }
    }
    
    return breakdown;
}

ActivityBreakdown Reports::getActivityData(std::time_t startDate, std::time_t endDate) {
    ActivityBreakdown breakdown = {0, 0, 0, 0, {}};
    
    // Initialize hourly activity (24 hours)
    breakdown.hourlyActivity.resize(24);
    for (int i = 0; i < 24; i++) {
        breakdown.hourlyActivity[i] = {i, 0};
    }
    
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db->getDatabase()) {
        return breakdown;
    }
    
    // Query check-ins for the date range
    std::string query = R"(
        SELECT customer_id, check_in_time, check_out_time 
        FROM check_ins 
        WHERE check_in_time >= ? AND check_in_time <= ?
        ORDER BY check_in_time
    )";
    
    sqlite3_stmt* stmt = db->prepareStatement(query);
    if (!stmt) {
        return breakdown;
    }
    
    sqlite3_bind_int64(stmt, 1, startDate);
    sqlite3_bind_int64(stmt, 2, endDate);
    
    std::set<int> uniqueCustomers;
    std::set<int> currentlyInGym;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int customerId = sqlite3_column_int(stmt, 0);
        std::time_t checkInTime = sqlite3_column_int64(stmt, 1);
        std::time_t checkOutTime = sqlite3_column_int64(stmt, 2);
        
        breakdown.totalCheckIns++;
        uniqueCustomers.insert(customerId);
        
        // Add to hourly activity
        std::tm* tm = std::localtime(&checkInTime);
        if (tm->tm_hour >= 0 && tm->tm_hour < 24) {
            breakdown.hourlyActivity[tm->tm_hour].second++;
        }
        
        if (checkOutTime > 0) {
            breakdown.totalCheckOuts++;
        } else {
            // Still checked in
            currentlyInGym.insert(customerId);
        }
    }
    
    sqlite3_finalize(stmt);
    
    breakdown.uniqueVisitors = static_cast<int>(uniqueCustomers.size());
    breakdown.currentlyCheckedIn = static_cast<int>(currentlyInGym.size());
    
    return breakdown;
}

int Reports::getNewRegistrations(std::time_t startDate, std::time_t endDate) {
    std::vector<Customer> customers = Customer::getAllCustomers();
    int count = 0;
    
    for (const auto& customer : customers) {
        std::time_t joinDate = customer.getJoinDate();
        if (joinDate >= startDate && joinDate <= endDate) {
            count++;
        }
    }
    
    return count;
}

int Reports::getRenewalsProcessed(std::time_t startDate, std::time_t endDate) {
    std::vector<Transaction> transactions = Transaction::getAllTransactions();
    int count = 0;
    
    for (const auto& transaction : transactions) {
        if (transaction.getTransactionDate() >= startDate && 
            transaction.getTransactionDate() <= endDate &&
            transaction.getTransactionType() == TransactionType::RENEWAL &&
            transaction.getStatus() == TransactionStatus::COMPLETED) {
            count++;
        }
    }
    
    return count;
}

// Report Generation Functions
DailyReportData Reports::generateDailyReportData(std::time_t date) {
    DailyReportData data;
    data.reportDate = date;
    
    std::time_t startOfDay = getStartOfDay(date);
    std::time_t endOfDay = getEndOfDay(date);
    
    data.revenue = getRevenueData(startOfDay, endOfDay);
    data.membership = getMembershipData(date);
    data.activity = getActivityData(startOfDay, endOfDay);
    data.newRegistrations = getNewRegistrations(startOfDay, endOfDay);
    data.renewalsProcessed = getRenewalsProcessed(startOfDay, endOfDay);
    
    return data;
}

// Daily Report Functions
void Reports::showDailyRevenueReport() {
    std::time_t today = std::time(nullptr);
    displayReportHeader("Daily Revenue Report", today);
    
    DailyReportData data = generateDailyReportData(today);
    
    std::cout << "ðŸ’° Revenue Summary:" << std::endl;
    std::cout << "   Total Revenue: " << formatCurrency(data.revenue.totalRevenue) << std::endl;
    std::cout << "   Cash Revenue:  " << formatCurrency(data.revenue.cashRevenue) 
              << " (" << data.revenue.cashTransactions << " transactions)" << std::endl;
    std::cout << "   M-Pesa Revenue: " << formatCurrency(data.revenue.mpesaRevenue) 
              << " (" << data.revenue.mpesaTransactions << " transactions)" << std::endl;
    
    if (data.revenue.totalTransactions > 0) {
        double cashPercentage = (data.revenue.cashRevenue / data.revenue.totalRevenue) * 100;
        double mpesaPercentage = (data.revenue.mpesaRevenue / data.revenue.totalRevenue) * 100;
        
        std::cout << "\nðŸ“Š Payment Method Distribution:" << std::endl;
        std::cout << "   Cash:   " << formatPercentage(cashPercentage) << std::endl;
        std::cout << "   M-Pesa: " << formatPercentage(mpesaPercentage) << std::endl;
        std::cout << "\nðŸ“ˆ Transaction Metrics:" << std::endl;
        std::cout << "   Total Transactions: " << data.revenue.totalTransactions << std::endl;
        std::cout << "   Average per Transaction: " << formatCurrency(data.revenue.averageTransaction) << std::endl;
    } else {
        std::cout << "\nâŒ No transactions recorded for today." << std::endl;
    }
}

void Reports::showDailyActivityReport() {
    std::time_t today = std::time(nullptr);
    displayReportHeader("Daily Activity Report", today);
    
    DailyReportData data = generateDailyReportData(today);
    
    std::cout << "ðŸƒâ€â™‚ï¸ Activity Summary:" << std::endl;
    std::cout << "   Total Check-ins: " << data.activity.totalCheckIns << std::endl;
    std::cout << "   Total Check-outs: " << data.activity.totalCheckOuts << std::endl;
    std::cout << "   Unique Visitors: " << data.activity.uniqueVisitors << std::endl;
    std::cout << "   Currently in Gym: " << data.activity.currentlyCheckedIn << std::endl;
    
    // Show peak hours
    std::cout << "\nâ° Hourly Activity Distribution:" << std::endl;
    
    // Find peak hour
    int peakHour = 0;
    int peakCount = 0;
    for (const auto& hour : data.activity.hourlyActivity) {
        if (hour.second > peakCount) {
            peakCount = hour.second;
            peakHour = hour.first;
        }
    }
    
    if (peakCount > 0) {
        std::cout << "   Peak Hour: " << std::setfill('0') << std::setw(2) << peakHour 
                  << ":00 (" << peakCount << " check-ins)" << std::endl;
    }
    
    // Show hours with activity
    std::cout << "\n   Active Hours:" << std::endl;
    for (const auto& hour : data.activity.hourlyActivity) {
        if (hour.second > 0) {
            std::cout << "   " << std::setfill('0') << std::setw(2) << hour.first 
                      << ":00 - " << hour.second << " check-ins" << std::endl;
        }
    }
}

void Reports::showDailyMembershipReport() {
    std::time_t today = std::time(nullptr);
    displayReportHeader("Daily Membership Report", today);
    
    DailyReportData data = generateDailyReportData(today);
    
    std::cout << "ðŸ‘¥ Membership Overview:" << std::endl;
    std::cout << "   Total Members: " << data.membership.totalMembers << std::endl;
    std::cout << "   Active Members: " << data.membership.activeMembers << std::endl;
    std::cout << "   Expired Members: " << data.membership.expiredMembers << std::endl;
    std::cout << "   Expiring in 7 Days: " << data.membership.expiringInWeek << std::endl;
    
    std::cout << "\nðŸ“ Membership Distribution:" << std::endl;
    std::cout << "   Daily: " << data.membership.dailyMembers << std::endl;
    std::cout << "   Weekly: " << data.membership.weeklyMembers << std::endl;
    std::cout << "   Monthly: " << data.membership.monthlyMembers << std::endl;
    std::cout << "   Trainer: " << data.membership.trainerMembers << std::endl;
    
    std::cout << "\nðŸ†• Today's Activity:" << std::endl;
    std::cout << "   New Registrations: " << data.newRegistrations << std::endl;
    std::cout << "   Renewals Processed: " << data.renewalsProcessed << std::endl;
    
    // Calculate percentages
    if (data.membership.totalMembers > 0) {
        double activePercentage = (static_cast<double>(data.membership.activeMembers) / data.membership.totalMembers) * 100;
        std::cout << "\nðŸ“Š Member Status:" << std::endl;
        std::cout << "   Active Rate: " << formatPercentage(activePercentage) << std::endl;
    }
}

void Reports::showCompleteDailyReport() {
    std::time_t today = std::time(nullptr);
    displayReportHeader("Complete Daily Report", today);
    
    // Revenue Section
    std::cout << "\nðŸ’° REVENUE" << std::endl;
    displaySeparator(30);
    showDailyRevenueReport();
    
    std::cout << "\n\nðŸƒâ€â™‚ï¸ ACTIVITY" << std::endl;
    displaySeparator(30);
    showDailyActivityReport();
    
    std::cout << "\n\nðŸ‘¥ MEMBERSHIP" << std::endl;
    displaySeparator(30);
    showDailyMembershipReport();
}

void Reports::showDailyReportForDate(std::time_t date) {
    displayReportHeader("Daily Report", date);
    
    DailyReportData data = generateDailyReportData(date);
    
    // Show all sections for the specific date
    std::cout << "\nðŸ’° Revenue: " << formatCurrency(data.revenue.totalRevenue) 
              << " (" << data.revenue.totalTransactions << " transactions)" << std::endl;
    
    std::cout << "ðŸƒâ€â™‚ï¸ Activity: " << data.activity.totalCheckIns << " check-ins, "
              << data.activity.uniqueVisitors << " unique visitors" << std::endl;
    
    std::cout << "ðŸ‘¥ Members: " << data.membership.totalMembers << " total, "
              << data.membership.activeMembers << " active" << std::endl;
    
    std::cout << "ðŸ†• New: " << data.newRegistrations << " registrations, "
              << data.renewalsProcessed << " renewals" << std::endl;
}

// Additional implementations to ADD to your existing Reports.cpp file

// NEW UTILITY FUNCTIONS - Add these after your existing utility functions

std::time_t Reports::getStartOfWeek(std::time_t date) {
    std::tm* tm = std::localtime(&date);
    int dayOfWeek = tm->tm_wday; // Sunday = 0
    if (dayOfWeek == 0) dayOfWeek = 7; // Make Monday = 1, Sunday = 7
    tm->tm_mday -= (dayOfWeek - 1); // Go back to Monday
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    return std::mktime(tm);
}

std::time_t Reports::getEndOfWeek(std::time_t date) {
    std::time_t startOfWeek = getStartOfWeek(date);
    return startOfWeek + (7 * 24 * 60 * 60) - 1; // Add 6 days, 23:59:59
}

std::time_t Reports::getStartOfMonth(std::time_t date) {
    std::tm* tm = std::localtime(&date);
    tm->tm_mday = 1;
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    return std::mktime(tm);
}

std::time_t Reports::getEndOfMonth(std::time_t date) {
    std::tm* tm = std::localtime(&date);
    tm->tm_mday = 1;
    tm->tm_mon++;
    if (tm->tm_mon > 11) {
        tm->tm_mon = 0;
        tm->tm_year++;
    }
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    std::time_t nextMonth = std::mktime(tm);
    return nextMonth - 1; // Last second of current month
}

std::string Reports::getDayName(int dayOfWeek) {
    const std::string days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
                               "Thursday", "Friday", "Saturday"};
    return (dayOfWeek >= 0 && dayOfWeek <= 6) ? days[dayOfWeek] : "Unknown";
}

int Reports::getDayOfWeek(std::time_t date) {
    std::tm* tm = std::localtime(&date);
    return tm->tm_wday;
}

int Reports::getWeekOfMonth(std::time_t date) {
    std::time_t startOfMonth = getStartOfMonth(date);
    return ((date - startOfMonth) / (7 * 24 * 60 * 60)) + 1;
}

std::vector<int> Reports::getUniqueVisitorsByDateRange(std::time_t startDate, std::time_t endDate) {
    std::vector<int> uniqueVisitors;
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db->getDatabase()) {
        return uniqueVisitors;
    }

    std::string query = R"(
        SELECT DATE(check_in_time, 'unixepoch') as check_date, 
               COUNT(DISTINCT customer_id) as unique_count
        FROM check_ins 
        WHERE check_in_time >= ? AND check_in_time <= ?
        GROUP BY DATE(check_in_time, 'unixepoch')
        ORDER BY check_date
    )";

    sqlite3_stmt* stmt = db->prepareStatement(query);
    if (!stmt) return uniqueVisitors;

    sqlite3_bind_int64(stmt, 1, startDate);
    sqlite3_bind_int64(stmt, 2, endDate);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        uniqueVisitors.push_back(sqlite3_column_int(stmt, 1));
    }

    sqlite3_finalize(stmt);
    return uniqueVisitors;
}

double Reports::calculateGrowthPercentage(double current, double previous) {
    if (previous == 0) return (current > 0) ? 100.0 : 0.0;
    return ((current - previous) / previous) * 100.0;
}

void Reports::displayProgressBar(double percentage, int width) {
    int filled = static_cast<int>((percentage / 100.0) * width);
    std::cout << "[";
    for (int i = 0; i < width; i++) {
        if (i < filled) std::cout << "█";
        else std::cout << "░";
    }
    std::cout << "] " << formatPercentage(percentage);
}

void Reports::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Reports::waitForInput() {
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

// WEEKLY REPORTS IMPLEMENTATION

WeeklyReportData Reports::generateWeeklyReportData(std::time_t weekStartDate) {
    WeeklyReportData data;
    data.weekStartDate = weekStartDate;
    data.weekEndDate = getEndOfWeek(weekStartDate);
    
    // Initialize totals
    data.totalRevenue = {0.0, 0.0, 0.0, 0, 0, 0, 0.0};
    data.totalNewMembers = 0;
    data.totalRenewals = 0;
    data.totalUniqueVisitors = 0;
    
    double bestDayRevenue = 0.0;
    double worstDayRevenue = std::numeric_limits<double>::max();
    int bestDay = 0, worstDay = 0;
    
    // Generate daily reports for each day of the week
    for (int i = 0; i < 7; i++) {
        std::time_t dayDate = weekStartDate + (i * 24 * 60 * 60);
        DailyReportData dailyData = generateDailyReportData(dayDate);
        data.dailyReports.push_back(dailyData);
        
        // Accumulate weekly totals
        data.totalRevenue.totalRevenue += dailyData.revenue.totalRevenue;
        data.totalRevenue.cashRevenue += dailyData.revenue.cashRevenue;
        data.totalRevenue.mpesaRevenue += dailyData.revenue.mpesaRevenue;
        data.totalRevenue.totalTransactions += dailyData.revenue.totalTransactions;
        data.totalRevenue.cashTransactions += dailyData.revenue.cashTransactions;
        data.totalRevenue.mpesaTransactions += dailyData.revenue.mpesaTransactions;
        
        data.totalNewMembers += dailyData.newRegistrations;
        data.totalRenewals += dailyData.renewalsProcessed;
        data.totalUniqueVisitors += dailyData.activity.uniqueVisitors;
        
        // Track best and worst days
        if (dailyData.revenue.totalRevenue > bestDayRevenue) {
            bestDayRevenue = dailyData.revenue.totalRevenue;
            bestDay = i;
        }
        if (dailyData.revenue.totalRevenue < worstDayRevenue && dailyData.revenue.totalRevenue > 0) {
            worstDayRevenue = dailyData.revenue.totalRevenue;
            worstDay = i;
        }
    }
    
    // Calculate averages
    if (data.totalRevenue.totalTransactions > 0) {
        data.totalRevenue.averageTransaction = data.totalRevenue.totalRevenue / data.totalRevenue.totalTransactions;
    }
    
    data.averageDailyVisitors = data.totalUniqueVisitors / 7;
    data.bestDay = {bestDay, bestDayRevenue};
    data.worstDay = {worstDay, worstDayRevenue};
    
    return data;
}

void Reports::showWeeklyReport() {
    std::time_t today = std::time(nullptr);
    std::time_t weekStart = getStartOfWeek(today);
    
    displayReportHeader("Weekly Report", weekStart);
    
    WeeklyReportData data = generateWeeklyReportData(weekStart);
    
    std::cout << "📅 Week Period: " << formatDate(data.weekStartDate) 
              << " to " << formatDate(data.weekEndDate) << std::endl;
    
    // Revenue Summary
    std::cout << "\n💰 Weekly Revenue Summary:" << std::endl;
    std::cout << "   Total Revenue: " << formatCurrency(data.totalRevenue.totalRevenue) << std::endl;
    std::cout << "   Average per Day: " << formatCurrency(data.totalRevenue.totalRevenue / 7) << std::endl;
    std::cout << "   Cash Revenue: " << formatCurrency(data.totalRevenue.cashRevenue) 
              << " (" << formatPercentage(data.totalRevenue.totalRevenue > 0 ? 
                 (data.totalRevenue.cashRevenue / data.totalRevenue.totalRevenue) * 100 : 0) << ")" << std::endl;
    std::cout << "   M-Pesa Revenue: " << formatCurrency(data.totalRevenue.mpesaRevenue) 
              << " (" << formatPercentage(data.totalRevenue.totalRevenue > 0 ? 
                 (data.totalRevenue.mpesaRevenue / data.totalRevenue.totalRevenue) * 100 : 0) << ")" << std::endl;
    
    // Daily Breakdown
    std::cout << "\n📊 Daily Breakdown:" << std::endl;
    for (size_t i = 0; i < data.dailyReports.size(); i++) {
        const auto& daily = data.dailyReports[i];
        std::cout << "   " << getDayName(static_cast<int>(i + 1) % 7) << ": " 
                  << formatCurrency(daily.revenue.totalRevenue) 
                  << " (" << daily.activity.uniqueVisitors << " visitors)" << std::endl;
    }
    
    // Best and Worst Days
    std::cout << "\n🏆 Performance Highlights:" << std::endl;
    std::cout << "   Best Day: " << getDayName((data.bestDay.first + 1) % 7) 
              << " - " << formatCurrency(data.bestDay.second) << std::endl;
    if (data.worstDay.second < std::numeric_limits<double>::max()) {
        std::cout << "   Lowest Day: " << getDayName((data.worstDay.first + 1) % 7) 
                  << " - " << formatCurrency(data.worstDay.second) << std::endl;
    }
    
    // Member Activity
    std::cout << "\n👥 Member Activity:" << std::endl;
    std::cout << "   Total Unique Visitors: " << data.totalUniqueVisitors << std::endl;
    std::cout << "   Average Daily Visitors: " << data.averageDailyVisitors << std::endl;
    std::cout << "   New Registrations: " << data.totalNewMembers << std::endl;
    std::cout << "   Renewals Processed: " << data.totalRenewals << std::endl;
}

void Reports::showWeeklyTrends() {
    std::time_t today = std::time(nullptr);
    std::time_t currentWeekStart = getStartOfWeek(today);
    std::time_t lastWeekStart = currentWeekStart - (7 * 24 * 60 * 60);
    
    displayReportHeader("Weekly Trends Analysis");
    
    WeeklyReportData currentWeek = generateWeeklyReportData(currentWeekStart);
    WeeklyReportData lastWeek = generateWeeklyReportData(lastWeekStart);
    
    // Revenue Growth
    double revenueGrowth = calculateGrowthPercentage(
        currentWeek.totalRevenue.totalRevenue, 
        lastWeek.totalRevenue.totalRevenue
    );
    
    // Visitor Growth
    double visitorGrowth = calculateGrowthPercentage(
        static_cast<double>(currentWeek.totalUniqueVisitors),
        static_cast<double>(lastWeek.totalUniqueVisitors)
    );
    
    std::cout << "📈 Week-over-Week Growth:" << std::endl;
    std::cout << "   Revenue Growth: " << formatPercentage(revenueGrowth) 
              << (revenueGrowth >= 0 ? " ⬆️" : " ⬇️") << std::endl;
    std::cout << "   Visitor Growth: " << formatPercentage(visitorGrowth) 
              << (visitorGrowth >= 0 ? " ⬆️" : " ⬇️") << std::endl;
    
    std::cout << "\n📊 Comparison Details:" << std::endl;
    std::cout << "   Current Week Revenue: " << formatCurrency(currentWeek.totalRevenue.totalRevenue) << std::endl;
    std::cout << "   Last Week Revenue: " << formatCurrency(lastWeek.totalRevenue.totalRevenue) << std::endl;
    std::cout << "   Current Week Visitors: " << currentWeek.totalUniqueVisitors << std::endl;
    std::cout << "   Last Week Visitors: " << lastWeek.totalUniqueVisitors << std::endl;
}

// MONTHLY REPORTS IMPLEMENTATION

MonthlyReportData Reports::generateMonthlyReportData(std::time_t monthStartDate) {
    MonthlyReportData data;
    data.monthStartDate = monthStartDate;
    data.monthEndDate = getEndOfMonth(monthStartDate);
    
    // Initialize totals
    data.totalRevenue = {0.0, 0.0, 0.0, 0, 0, 0, 0.0};
    data.totalNewMembers = 0;
    data.totalRenewals = 0;
    data.totalUniqueVisitors = 0;
    
    double bestWeekRevenue = 0.0;
    double worstWeekRevenue = std::numeric_limits<double>::max();
    int bestWeek = 0, worstWeek = 0;
    
    // Generate weekly reports for the month
    std::time_t currentWeekStart = getStartOfWeek(monthStartDate);
    int weekCount = 0;
    
    while (currentWeekStart < data.monthEndDate) {
        WeeklyReportData weeklyData = generateWeeklyReportData(currentWeekStart);
        data.weeklyReports.push_back(weeklyData);
        
        // Accumulate monthly totals
        data.totalRevenue.totalRevenue += weeklyData.totalRevenue.totalRevenue;
        data.totalRevenue.cashRevenue += weeklyData.totalRevenue.cashRevenue;
        data.totalRevenue.mpesaRevenue += weeklyData.totalRevenue.mpesaRevenue;
        data.totalRevenue.totalTransactions += weeklyData.totalRevenue.totalTransactions;
        data.totalRevenue.cashTransactions += weeklyData.totalRevenue.cashTransactions;
        data.totalRevenue.mpesaTransactions += weeklyData.totalRevenue.mpesaTransactions;
        
        data.totalNewMembers += weeklyData.totalNewMembers;
        data.totalRenewals += weeklyData.totalRenewals;
        data.totalUniqueVisitors += weeklyData.totalUniqueVisitors;
        
        // Track best and worst weeks
        if (weeklyData.totalRevenue.totalRevenue > bestWeekRevenue) {
            bestWeekRevenue = weeklyData.totalRevenue.totalRevenue;
            bestWeek = weekCount;
        }
        if (weeklyData.totalRevenue.totalRevenue < worstWeekRevenue && weeklyData.totalRevenue.totalRevenue > 0) {
            worstWeekRevenue = weeklyData.totalRevenue.totalRevenue;
            worstWeek = weekCount;
        }
        
        currentWeekStart += (7 * 24 * 60 * 60);
        weekCount++;
    }
    
    // Calculate averages
    if (data.totalRevenue.totalTransactions > 0) {
        data.totalRevenue.averageTransaction = data.totalRevenue.totalRevenue / data.totalRevenue.totalTransactions;
    }
    
    // Calculate days in month for accurate averages
    int daysInMonth = static_cast<int>((data.monthEndDate - data.monthStartDate) / (24 * 60 * 60)) + 1;
    data.averageDailyRevenue = data.totalRevenue.totalRevenue / daysInMonth;
    data.averageDailyVisitors = static_cast<double>(data.totalUniqueVisitors) / daysInMonth;
    
    data.bestWeek = {bestWeek, bestWeekRevenue};
    data.worstWeek = {worstWeek, worstWeekRevenue};
    
    // Get current membership data
    data.membershipGrowth = getMembershipData(data.monthEndDate);
    
    return data;
}

void Reports::showMonthlyReport() {
    std::time_t today = std::time(nullptr);
    std::time_t monthStart = getStartOfMonth(today);
    
    displayReportHeader("Monthly Report", monthStart);
    
    MonthlyReportData data = generateMonthlyReportData(monthStart);
    
    std::cout << "📅 Month Period: " << formatDate(data.monthStartDate) 
              << " to " << formatDate(data.monthEndDate) << std::endl;
    
    // Revenue Summary
    std::cout << "\n💰 Monthly Revenue Summary:" << std::endl;
    std::cout << "   Total Revenue: " << formatCurrency(data.totalRevenue.totalRevenue) << std::endl;
    std::cout << "   Average per Day: " << formatCurrency(data.averageDailyRevenue) << std::endl;
    std::cout << "   Average per Week: " << formatCurrency(data.totalRevenue.totalRevenue / data.weeklyReports.size()) << std::endl;
    std::cout << "   Cash Revenue: " << formatCurrency(data.totalRevenue.cashRevenue) 
              << " (" << formatPercentage(data.totalRevenue.totalRevenue > 0 ? 
                 (data.totalRevenue.cashRevenue / data.totalRevenue.totalRevenue) * 100 : 0) << ")" << std::endl;
    std::cout << "   M-Pesa Revenue: " << formatCurrency(data.totalRevenue.mpesaRevenue) 
              << " (" << formatPercentage(data.totalRevenue.totalRevenue > 0 ? 
                 (data.totalRevenue.mpesaRevenue / data.totalRevenue.totalRevenue) * 100 : 0) << ")" << std::endl;
    
    // Weekly Breakdown
    std::cout << "\n📊 Weekly Breakdown:" << std::endl;
    for (size_t i = 0; i < data.weeklyReports.size(); i++) {
        const auto& weekly = data.weeklyReports[i];
        std::cout << "   Week " << (i + 1) << ": " 
                  << formatCurrency(weekly.totalRevenue.totalRevenue)
                  << " (" << weekly.totalUniqueVisitors << " visitors)" << std::endl;
    }
    
    // Performance Highlights
    std::cout << "\n🏆 Performance Highlights:" << std::endl;
    std::cout << "   Best Week: Week " << (data.bestWeek.first + 1) 
              << " - " << formatCurrency(data.bestWeek.second) << std::endl;
    if (data.worstWeek.second < std::numeric_limits<double>::max()) {
        std::cout << "   Lowest Week: Week " << (data.worstWeek.first + 1) 
                  << " - " << formatCurrency(data.worstWeek.second) << std::endl;
    }
    
    // Member Growth
    std::cout << "\n👥 Member Growth:" << std::endl;
    std::cout << "   New Registrations: " << data.totalNewMembers << std::endl;
    std::cout << "   Renewals Processed: " << data.totalRenewals << std::endl;
    std::cout << "   Total Members: " << data.membershipGrowth.totalMembers << std::endl;
    std::cout << "   Active Members: " << data.membershipGrowth.activeMembers << std::endl;
    std::cout << "   Average Daily Visitors: " << static_cast<int>(data.averageDailyVisitors) << std::endl;
}

// LIVE DASHBOARD IMPLEMENTATION

LiveDashboardData Reports::getLiveDashboardData() {
    LiveDashboardData data;
    data.lastUpdated = std::time(nullptr);
    
    std::time_t today = std::time(nullptr);
    std::time_t startOfDay = getStartOfDay(today);
    std::time_t endOfDay = getEndOfDay(today);
    
    // Get today's data
    DailyReportData todayData = generateDailyReportData(today);
    data.todaysRevenue = todayData.revenue.totalRevenue;
    data.todaysTransactions = todayData.revenue.totalTransactions;
    data.todaysCheckIns = todayData.activity.totalCheckIns;
    data.todaysNewMembers = todayData.newRegistrations;
    data.currentlyCheckedIn = todayData.activity.currentlyCheckedIn;
    
    // Calculate members expiring soon (next 3 days)
    std::vector<Customer> customers = Customer::getAllCustomers();
    std::time_t threeDaysFromNow = today + (3 * 24 * 60 * 60);
    data.expiringSoon = 0;
    
    for (const auto& customer : customers) {
        if (customer.getMembershipType() != MembershipType::DAILY) {
            std::time_t expiryDate = customer.getExpiryDate();
            if (expiryDate > today && expiryDate <= threeDaysFromNow) {
                data.expiringSoon++;
            }
        }
    }
    
    // Calculate weekly and monthly progress (simplified targets)
    std::time_t weekStart = getStartOfWeek(today);
    std::time_t monthStart = getStartOfMonth(today);
    
    WeeklyReportData weekData = generateWeeklyReportData(weekStart);
    MonthlyReportData monthData = generateMonthlyReportData(monthStart);
    
    // Set simple targets (these should be configurable in a real system)
    data.weeklyRevenueTarget = 50000.0; // KES 50,000 per week
    data.monthlyRevenueTarget = 200000.0; // KES 200,000 per month
    
    data.weeklyProgress = (weekData.totalRevenue.totalRevenue / data.weeklyRevenueTarget) * 100;
    data.monthlyProgress = (monthData.totalRevenue.totalRevenue / data.monthlyRevenueTarget) * 100;
    
    // Recent activity (simplified)
    data.recentActivity.push_back({"Check-ins today", data.todaysCheckIns});
    data.recentActivity.push_back({"New members today", data.todaysNewMembers});
    data.recentActivity.push_back({"Transactions today", data.todaysTransactions});
    data.recentActivity.push_back({"Currently in gym", data.currentlyCheckedIn});
    
    return data;
}

void Reports::showLiveDashboard() {
    clearScreen();
    LiveDashboardData data = getLiveDashboardData();
    
    displayReportHeader("Live Dashboard");
    std::cout << "🕒 Last Updated: " << formatDate(data.lastUpdated) 
              << " " << std::put_time(std::localtime(&data.lastUpdated), "%H:%M:%S") << std::endl;
    
    displaySeparator(60);
    
    // Today's Quick Stats
    std::cout << "📊 TODAY'S OVERVIEW" << std::endl;
    std::cout << "💰 Revenue: " << formatCurrency(data.todaysRevenue) << std::endl;
    std::cout << "🏃‍♂️ Check-ins: " << data.todaysCheckIns << std::endl;
    std::cout << "👥 Currently in Gym: " << data.currentlyCheckedIn << std::endl;
    std::cout << "🆕 New Members: " << data.todaysNewMembers << std::endl;
    std::cout << "⚠️ Expiring Soon: " << data.expiringSoon << " members" << std::endl;
    
    displaySeparator(40);
    
    // Progress Towards Targets
    //std::cout << "🎯 PROGRESS TOWARDS TARGETS" << std::endl;
    //std::cout << "Weekly Target: " << formatCurrency(data.weeklyRevenueTarget) << std::endl;
    //std::cout << "Weekly Progress: ";
   //displayProgressBar(data.weeklyProgress, 25);
    //std::cout << std::endl;

    //std::cout << "Monthly Target: " << formatCurrency(data.monthlyRevenueTarget) << std::endl;
    //std::cout << "Monthly Progress: ";
    //displayProgressBar(data.monthlyProgress, 25);
    //std::cout << std::endl;

    //displaySeparator(40);
    
    // Recent Activity
    std::cout << "📈 QUICK METRICS" << std::endl;
    for (const auto& activity : data.recentActivity) {
        std::cout << activity.first << ": " << activity.second << std::endl;
    }
    
    displaySeparator(60);
    std::cout << "Press 'r' to refresh, or any other key to continue...";
}

void Reports::showQuickStats() {
    std::time_t today = std::time(nullptr);
    DailyReportData todayData = generateDailyReportData(today);
    
    std::cout << "\n⚡ QUICK STATS" << std::endl;
    displaySeparator(30);
    std::cout << "Today's Revenue: " << formatCurrency(todayData.revenue.totalRevenue) << std::endl;
    std::cout << "Check-ins: " << todayData.activity.totalCheckIns << std::endl;
    std::cout << "Currently in Gym: " << todayData.activity.currentlyCheckedIn << std::endl;
    std::cout << "New Members: " << todayData.newRegistrations << std::endl;
    
    // Week comparison
    std::time_t lastWeek = today - (7 * 24 * 60 * 60);
    DailyReportData lastWeekData = generateDailyReportData(lastWeek);
    
    double revenueGrowth = calculateGrowthPercentage(
        todayData.revenue.totalRevenue, 
        lastWeekData.revenue.totalRevenue
    );
    
    std::cout << "Week-over-Week: " << formatPercentage(revenueGrowth) 
              << (revenueGrowth >= 0 ? " ⬆️" : " ⬇️") << std::endl;
}

void Reports::refreshDashboard() {
    char choice;
    do {
        showLiveDashboard();
        std::cin >> choice;
        
        if (choice == 'r' || choice == 'R') {
            std::cout << "Refreshing dashboard..." << std::endl;
            #ifdef _WIN32
            Sleep(1000); // Windows
            #else
            sleep(1); // Unix/Linux
            #endif
        }
    } while (choice == 'r' || choice == 'R');
}

void Reports::showRealtimeMetrics() {
    displayReportHeader("Real-time Metrics");
    
    LiveDashboardData data = getLiveDashboardData();
    
    std::cout << "🔄 REAL-TIME STATUS" << std::endl;
    std::cout << "Active Sessions: " << data.currentlyCheckedIn << std::endl;
    std::cout << "Today's Check-ins: " << data.todaysCheckIns << std::endl;
    
    // Show hourly activity for today
    std::time_t today = std::time(nullptr);
    ActivityBreakdown activity = getActivityData(getStartOfDay(today), getEndOfDay(today));
    
    std::cout << "\n⏰ HOURLY ACTIVITY (Today)" << std::endl;
    std::time_t now = std::time(nullptr);
    std::tm* currentTm = std::localtime(&now);
    int currentHour = currentTm->tm_hour;
    
    // Show last 6 hours of activity
    for (int i = std::max(0, currentHour - 5); i <= currentHour; i++) {
        if (i < static_cast<int>(activity.hourlyActivity.size())) {
            std::cout << std::setfill('0') << std::setw(2) << i << ":00 - " 
                      << activity.hourlyActivity[i].second << " check-ins";
            if (i == currentHour) std::cout << " (current hour)";
            std::cout << std::endl;
        }
    }
    
    // Predictions/Alerts
    std::cout << "\n⚡ ALERTS & NOTIFICATIONS" << std::endl;
    if (data.expiringSoon > 0) {
        std::cout << "⚠️ " << data.expiringSoon << " memberships expiring in 3 days" << std::endl;
    }
    
    if (data.currentlyCheckedIn > 50) { // Assuming 50 is high capacity
        std::cout << "🏃‍♂️ High gym traffic: " << data.currentlyCheckedIn << " people currently inside" << std::endl;
    }
    
    if (data.todaysRevenue < (data.weeklyRevenueTarget / 7) * 0.5) { // Below 50% of daily target
        std::cout << "📉 Revenue below daily target - consider promotional activities" << std::endl;
    }
}

// ADDITIONAL WEEKLY/MONTHLY FUNCTIONS

void Reports::showWeeklyReportForDate(std::time_t date) {
    std::time_t weekStart = getStartOfWeek(date);
    displayReportHeader("Weekly Report", weekStart);
    
    WeeklyReportData data = generateWeeklyReportData(weekStart);
    
    std::cout << "📅 Week Period: " << formatDate(data.weekStartDate) 
              << " to " << formatDate(data.weekEndDate) << std::endl;
    
    std::cout << "\n💰 Revenue: " << formatCurrency(data.totalRevenue.totalRevenue) << std::endl;
    std::cout << "👥 Visitors: " << data.totalUniqueVisitors << std::endl;
    std::cout << "🆕 New Members: " << data.totalNewMembers << std::endl;
    std::cout << "🔄 Renewals: " << data.totalRenewals << std::endl;
    
    std::cout << "\n📊 Daily Performance:" << std::endl;
    for (size_t i = 0; i < data.dailyReports.size(); i++) {
        const auto& daily = data.dailyReports[i];
        std::cout << "   " << getDayName(static_cast<int>(i + 1) % 7) << ": " 
                  << formatCurrency(daily.revenue.totalRevenue) << std::endl;
    }
}

void Reports::showWeeklyComparison() {
    std::time_t today = std::time(nullptr);
    std::time_t currentWeekStart = getStartOfWeek(today);
    std::time_t lastWeekStart = currentWeekStart - (7 * 24 * 60 * 60);
    std::time_t twoWeeksAgoStart = lastWeekStart - (7 * 24 * 60 * 60);
    
    displayReportHeader("Weekly Comparison Analysis");
    
    WeeklyReportData currentWeek = generateWeeklyReportData(currentWeekStart);
    WeeklyReportData lastWeek = generateWeeklyReportData(lastWeekStart);
    WeeklyReportData twoWeeksAgo = generateWeeklyReportData(twoWeeksAgoStart);
    
    std::cout << "📊 THREE-WEEK COMPARISON" << std::endl;
    std::cout << std::left << std::setw(15) << "Week" 
              << std::setw(15) << "Revenue" 
              << std::setw(15) << "Visitors" 
              << std::setw(15) << "New Members" << std::endl;
    displaySeparator(60);
    
    std::cout << std::left << std::setw(15) << "Current" 
              << std::setw(15) << formatCurrency(currentWeek.totalRevenue.totalRevenue)
              << std::setw(15) << currentWeek.totalUniqueVisitors
              << std::setw(15) << currentWeek.totalNewMembers << std::endl;
              
    std::cout << std::left << std::setw(15) << "Last Week" 
              << std::setw(15) << formatCurrency(lastWeek.totalRevenue.totalRevenue)
              << std::setw(15) << lastWeek.totalUniqueVisitors
              << std::setw(15) << lastWeek.totalNewMembers << std::endl;
              
    std::cout << std::left << std::setw(15) << "2 Weeks Ago" 
              << std::setw(15) << formatCurrency(twoWeeksAgo.totalRevenue.totalRevenue)
              << std::setw(15) << twoWeeksAgo.totalUniqueVisitors
              << std::setw(15) << twoWeeksAgo.totalNewMembers << std::endl;
    
    // Growth Analysis
    std::cout << "\n📈 GROWTH ANALYSIS" << std::endl;
    double revenueGrowth = calculateGrowthPercentage(
        currentWeek.totalRevenue.totalRevenue, 
        lastWeek.totalRevenue.totalRevenue
    );
    double visitorGrowth = calculateGrowthPercentage(
        static_cast<double>(currentWeek.totalUniqueVisitors),
        static_cast<double>(lastWeek.totalUniqueVisitors)
    );
    
    std::cout << "Revenue Growth (vs last week): " << formatPercentage(revenueGrowth) 
              << (revenueGrowth >= 0 ? " ⬆️" : " ⬇️") << std::endl;
    std::cout << "Visitor Growth (vs last week): " << formatPercentage(visitorGrowth) 
              << (visitorGrowth >= 0 ? " ⬆️" : " ⬇️") << std::endl;
}

void Reports::showMonthlyReportForDate(std::time_t date) {
    std::time_t monthStart = getStartOfMonth(date);
    displayReportHeader("Monthly Report", monthStart);
    
    MonthlyReportData data = generateMonthlyReportData(monthStart);
    
    std::cout << "📅 Month Period: " << formatDate(data.monthStartDate) 
              << " to " << formatDate(data.monthEndDate) << std::endl;
    
    std::cout << "\n💰 Total Revenue: " << formatCurrency(data.totalRevenue.totalRevenue) << std::endl;
    std::cout << "📈 Average Daily: " << formatCurrency(data.averageDailyRevenue) << std::endl;
    std::cout << "👥 Total Visitors: " << data.totalUniqueVisitors << std::endl;
    std::cout << "🆕 New Members: " << data.totalNewMembers << std::endl;
    std::cout << "🔄 Renewals: " << data.totalRenewals << std::endl;
    
    std::cout << "\n📊 Weekly Breakdown:" << std::endl;
    for (size_t i = 0; i < data.weeklyReports.size(); i++) {
        const auto& weekly = data.weeklyReports[i];
        std::cout << "   Week " << (i + 1) << ": " 
                  << formatCurrency(weekly.totalRevenue.totalRevenue) << std::endl;
    }
}

void Reports::showMonthlyComparison() {
    std::time_t today = std::time(nullptr);
    std::time_t currentMonthStart = getStartOfMonth(today);
    
    // Get last month
    std::tm* tm = std::localtime(&currentMonthStart);
    tm->tm_mon--;
    if (tm->tm_mon < 0) {
        tm->tm_mon = 11;
        tm->tm_year--;
    }
    std::time_t lastMonthStart = std::mktime(tm);
    
    displayReportHeader("Monthly Comparison Analysis");
    
    MonthlyReportData currentMonth = generateMonthlyReportData(currentMonthStart);
    MonthlyReportData lastMonth = generateMonthlyReportData(lastMonthStart);
    
    std::cout << "📊 MONTH-OVER-MONTH COMPARISON" << std::endl;
    std::cout << std::left << std::setw(20) << "Metric" 
              << std::setw(20) << "Current Month" 
              << std::setw(20) << "Last Month"
              << std::setw(15) << "Growth" << std::endl;
    displaySeparator(75);
    
    double revenueGrowth = calculateGrowthPercentage(
        currentMonth.totalRevenue.totalRevenue, 
        lastMonth.totalRevenue.totalRevenue
    );
    double visitorGrowth = calculateGrowthPercentage(
        static_cast<double>(currentMonth.totalUniqueVisitors),
        static_cast<double>(lastMonth.totalUniqueVisitors)
    );
    double memberGrowth = calculateGrowthPercentage(
        static_cast<double>(currentMonth.totalNewMembers),
        static_cast<double>(lastMonth.totalNewMembers)
    );
    
    std::cout << std::left << std::setw(20) << "Revenue" 
              << std::setw(20) << formatCurrency(currentMonth.totalRevenue.totalRevenue)
              << std::setw(20) << formatCurrency(lastMonth.totalRevenue.totalRevenue)
              << std::setw(15) << formatPercentage(revenueGrowth) << std::endl;
              
    std::cout << std::left << std::setw(20) << "Visitors" 
              << std::setw(20) << currentMonth.totalUniqueVisitors
              << std::setw(20) << lastMonth.totalUniqueVisitors
              << std::setw(15) << formatPercentage(visitorGrowth) << std::endl;
              
    std::cout << std::left << std::setw(20) << "New Members" 
              << std::setw(20) << currentMonth.totalNewMembers
              << std::setw(20) << lastMonth.totalNewMembers
              << std::setw(15) << formatPercentage(memberGrowth) << std::endl;
    
    // Performance Summary
    std::cout << "\n🏆 PERFORMANCE SUMMARY" << std::endl;
    if (revenueGrowth > 0) {
        std::cout << "✅ Revenue is growing month-over-month" << std::endl;
    } else {
        std::cout << "⚠️ Revenue declined compared to last month" << std::endl;
    }
    
    if (visitorGrowth > 0) {
        std::cout << "✅ Visitor count is increasing" << std::endl;
    } else {
        std::cout << "⚠️ Visitor count decreased" << std::endl;
    }
    
    if (memberGrowth > 0) {
        std::cout << "✅ New member acquisition is growing" << std::endl;
    } else {
        std::cout << "⚠️ New member acquisition slowed down" << std::endl;
    }
}

void Reports::showMonthlyTrends() {
    std::time_t today = std::time(nullptr);
    std::time_t currentMonthStart = getStartOfMonth(today);
    
    displayReportHeader("Monthly Trends Analysis");
    
    MonthlyReportData monthData = generateMonthlyReportData(currentMonthStart);
    
    std::cout << "📈 REVENUE TRENDS" << std::endl;
    
    // Show weekly trend within the month
    if (monthData.weeklyReports.size() > 1) {
        std::cout << "Weekly Revenue Progression:" << std::endl;
        for (size_t i = 0; i < monthData.weeklyReports.size(); i++) {
            const auto& week = monthData.weeklyReports[i];
            std::cout << "   Week " << (i + 1) << ": " 
                      << formatCurrency(week.totalRevenue.totalRevenue);
            
            if (i > 0) {
                double weeklyGrowth = calculateGrowthPercentage(
                    week.totalRevenue.totalRevenue,
                    monthData.weeklyReports[i-1].totalRevenue.totalRevenue
                );
                std::cout << " (" << formatPercentage(weeklyGrowth) 
                          << (weeklyGrowth >= 0 ? " ⬆️" : " ⬇️") << ")";
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << "\n👥 VISITOR TRENDS" << std::endl;
    std::cout << "Average Daily Visitors: " << static_cast<int>(monthData.averageDailyVisitors) << std::endl;
    std::cout << "Total Unique Visitors: " << monthData.totalUniqueVisitors << std::endl;
    
    // Show best and worst performing days of week (aggregated)
    std::map<int, int> dayOfWeekVisitors;
    std::map<int, double> dayOfWeekRevenue;
    
    for (const auto& week : monthData.weeklyReports) {
        for (size_t i = 0; i < week.dailyReports.size(); i++) {
            int dayOfWeek = static_cast<int>(i + 1) % 7; // Monday = 1, Sunday = 0
            dayOfWeekVisitors[dayOfWeek] += week.dailyReports[i].activity.uniqueVisitors;
            dayOfWeekRevenue[dayOfWeek] += week.dailyReports[i].revenue.totalRevenue;
        }
    }
    
    // Find best day
    int bestDay = 0;
    double bestDayRevenue = 0;
    for (const auto& day : dayOfWeekRevenue) {
        if (day.second > bestDayRevenue) {
            bestDayRevenue = day.second;
            bestDay = day.first;
        }
    }
    
    std::cout << "\n📊 BEST PERFORMING DAY" << std::endl;
    std::cout << "Day: " << getDayName(bestDay) << std::endl;
    std::cout << "Total Revenue: " << formatCurrency(bestDayRevenue) << std::endl;
    std::cout << "Total Visitors: " << dayOfWeekVisitors[bestDay] << std::endl;
}

// REPLACE the existing showReportsMenu() function in your Reports.cpp with this updated version:

void Reports::showReportsMenu() {
    int choice;
    do {
        std::cout << "\n📊 === Reports & Analytics ===" << std::endl;
        std::cout << "[1] Daily Revenue Report" << std::endl;
        std::cout << "[2] Daily Activity Report" << std::endl;
        std::cout << "[3] Daily Membership Report" << std::endl;
        std::cout << "[4] Complete Daily Report" << std::endl;
        std::cout << "[5] Historical Daily Report" << std::endl;
        displaySeparator(40);
        std::cout << "[6] Weekly Report" << std::endl;
        std::cout << "[7] Weekly Trends" << std::endl;
        std::cout << "[8] Weekly Comparison" << std::endl;
        std::cout << "[9] Historical Weekly Report" << std::endl;
        displaySeparator(40);
        std::cout << "[10] Monthly Report" << std::endl;
        std::cout << "[11] Monthly Comparison" << std::endl;
        std::cout << "[12] Monthly Trends" << std::endl;
        std::cout << "[13] Historical Monthly Report" << std::endl;
        displaySeparator(40);
        std::cout << "[14] Live Dashboard" << std::endl;
        std::cout << "[15] Quick Stats" << std::endl;
        std::cout << "[16] Real-time Metrics" << std::endl;
        std::cout << "[17] Custom Date Range Report (Coming Soon)" << std::endl;
        displaySeparator(40);
        std::cout << "[0] Back to Main Menu" << std::endl;
        
        std::cout << "Enter choice: ";
        std::cin >> choice;
        
        // Clear input buffer
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (choice) {
            // Daily Reports
            case 1:
                showDailyRevenueReport();
                break;
            case 2:
                showDailyActivityReport();
                break;
            case 3:
                showDailyMembershipReport();
                break;
            case 4:
                showCompleteDailyReport();
                break;
            case 5: {
                std::cout << "Enter date (YYYY-MM-DD): ";
                std::string dateStr;
                std::getline(std::cin, dateStr);
                
                // Parse date string (simplified - assumes correct format)
                std::tm tm = {};
                if (strptime(dateStr.c_str(), "%Y-%m-%d", &tm)) {
                    std::time_t reportDate = std::mktime(&tm);
                    showDailyReportForDate(reportDate);
                } else {
                    std::cout << "❌ Invalid date format. Please use YYYY-MM-DD" << std::endl;
                }
                break;
            }
            
            // Weekly Reports
            case 6:
                showWeeklyReport();
                break;
            case 7:
                showWeeklyTrends();
                break;
            case 8:
                showWeeklyComparison();
                break;
            case 9: {
                std::cout << "Enter date (YYYY-MM-DD) for any day in the week: ";
                std::string dateStr;
                std::getline(std::cin, dateStr);
                
                std::tm tm = {};
                if (strptime(dateStr.c_str(), "%Y-%m-%d", &tm)) {
                    std::time_t reportDate = std::mktime(&tm);
                    showWeeklyReportForDate(reportDate);
                } else {
                    std::cout << "❌ Invalid date format. Please use YYYY-MM-DD" << std::endl;
                }
                break;
            }
            
            // Monthly Reports
            case 10:
                showMonthlyReport();
                break;
            case 11:
                showMonthlyComparison();
                break;
            case 12:
                showMonthlyTrends();
                break;
            case 13: {
                std::cout << "Enter date (YYYY-MM-DD) for any day in the month: ";
                std::string dateStr;
                std::getline(std::cin, dateStr);
                
                std::tm tm = {};
                if (strptime(dateStr.c_str(), "%Y-%m-%d", &tm)) {
                    std::time_t reportDate = std::mktime(&tm);
                    showMonthlyReportForDate(reportDate);
                } else {
                    std::cout << "❌ Invalid date format. Please use YYYY-MM-DD" << std::endl;
                }
                break;
            }
            
            // Dashboard & Live Reports
            case 14:
                refreshDashboard();
                break;
            case 15:
                showQuickStats();
                break;
            case 16:
                showRealtimeMetrics();
                break;
            case 17:
                std::cout << "\n🚧 Custom Date Range Reports - Coming in Phase 4!" << std::endl;
                break;
                
            case 0:
                break;
            default:
                std::cout << "❌ Invalid choice!" << std::endl;
                break;
        }
        
        if (choice != 0) {
            waitForInput();
        }
        
    } while (choice != 0);
}