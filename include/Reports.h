// File: include/Reports.h
#ifndef REPORTS_H
#define REPORTS_H

#include <string>
#include <vector>
#include <ctime>
#include "Customer.h"
#include "Transaction.h"

struct RevenueBreakdown {
    double totalRevenue;
    double cashRevenue;
    double mpesaRevenue;
    int totalTransactions;
    int cashTransactions;
    int mpesaTransactions;
    double averageTransaction;
};

struct MembershipBreakdown {
    int dailyMembers;
    int weeklyMembers;
    int monthlyMembers;
    int trainerMembers;
    int totalMembers;
    int activeMembers;
    int expiredMembers;
    int expiringInWeek;
};

struct ActivityBreakdown {
    int totalCheckIns;
    int totalCheckOuts;
    int currentlyCheckedIn;
    int uniqueVisitors;
    std::vector<std::pair<int, int>> hourlyActivity; // hour, count pairs
};

struct DailyReportData {
    std::time_t reportDate;
    RevenueBreakdown revenue;
    MembershipBreakdown membership;
    ActivityBreakdown activity;
    int newRegistrations;
    int renewalsProcessed;
};

// NEW STRUCTURES FOR WEEKLY/MONTHLY REPORTS
struct WeeklyReportData {
    std::time_t weekStartDate;
    std::time_t weekEndDate;
    std::vector<DailyReportData> dailyReports;
    RevenueBreakdown totalRevenue;
    int totalNewMembers;
    int totalRenewals;
    int totalUniqueVisitors;
    int averageDailyVisitors;
    std::pair<int, double> bestDay; // day of week (0-6), revenue
    std::pair<int, double> worstDay;
};

struct MonthlyReportData {
    std::time_t monthStartDate;
    std::time_t monthEndDate;
    std::vector<WeeklyReportData> weeklyReports;
    RevenueBreakdown totalRevenue;
    MembershipBreakdown membershipGrowth;
    int totalNewMembers;
    int totalRenewals;
    int totalUniqueVisitors;
    double averageDailyRevenue;
    double averageDailyVisitors;
    std::pair<int, double> bestWeek; // week number, revenue
    std::pair<int, double> worstWeek;
};

struct LiveDashboardData {
    std::time_t lastUpdated;
    int currentlyCheckedIn;
    double todaysRevenue;
    int todaysTransactions;
    int todaysCheckIns;
    int todaysNewMembers;
    int expiringSoon; // expiring in next 3 days
    double weeklyRevenueTarget;
    double monthlyRevenueTarget;
    double weeklyProgress;
    double monthlyProgress;
    std::vector<std::pair<std::string, int>> recentActivity; // activity type, count
};

class Reports {
private:
    static std::time_t getStartOfDay(std::time_t date);
    static std::time_t getEndOfDay(std::time_t date);
    static std::time_t getStartOfWeek(std::time_t date);
    static std::time_t getEndOfWeek(std::time_t date);
    static std::time_t getStartOfMonth(std::time_t date);
    static std::time_t getEndOfMonth(std::time_t date);
    static std::string formatDate(std::time_t date);
    static std::string formatCurrency(double amount);
    static std::string formatPercentage(double value);
    static std::string getDayName(int dayOfWeek);
    static int getDayOfWeek(std::time_t date);
    static int getWeekOfMonth(std::time_t date);
    
    // Helper functions for data collection
    static RevenueBreakdown getRevenueData(std::time_t startDate, std::time_t endDate);
    static MembershipBreakdown getMembershipData(std::time_t date);
    static ActivityBreakdown getActivityData(std::time_t startDate, std::time_t endDate);
    static int getNewRegistrations(std::time_t startDate, std::time_t endDate);
    static int getRenewalsProcessed(std::time_t startDate, std::time_t endDate);
    
    // NEW: Advanced data collection functions
    static std::vector<int> getUniqueVisitorsByDateRange(std::time_t startDate, std::time_t endDate);
    static double calculateGrowthPercentage(double current, double previous);
    static void displayProgressBar(double percentage, int width = 30);
    static LiveDashboardData getLiveDashboardData();
    
public:
    // Daily Reports
    static void showDailyRevenueReport();
    static void showDailyActivityReport();
    static void showDailyMembershipReport();
    static void showCompleteDailyReport();
    static void showDailyReportForDate(std::time_t date);
    
    // NEW: Weekly Reports (Phase 2)
    static void showWeeklyReport();
    static void showWeeklyTrends();
    static void showWeeklyReportForDate(std::time_t date);
    static void showWeeklyComparison();
    
    // NEW: Monthly Reports (Phase 3)
    static void showMonthlyReport();
    static void showMonthlyComparison();
    static void showMonthlyReportForDate(std::time_t date);
    static void showMonthlyTrends();
    
    // Custom Reports (Phase 4)
    static void showCustomDateRangeReport();
    
    // NEW: Dashboard Metrics (Phase 5)
    static void showLiveDashboard();
    static void showQuickStats();
    static void refreshDashboard();
    static void showRealtimeMetrics();
    
    // Utility functions
    static void showReportsMenu();
    static DailyReportData generateDailyReportData(std::time_t date);
    static WeeklyReportData generateWeeklyReportData(std::time_t weekStartDate);
    static MonthlyReportData generateMonthlyReportData(std::time_t monthStartDate);
    static void displayReportHeader(const std::string& title, std::time_t date = 0);
    static void displaySeparator(int length = 60);
    static void clearScreen();
    static void waitForInput();
};

#endif // REPORTS_H