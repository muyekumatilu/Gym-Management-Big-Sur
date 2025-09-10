#ifndef DASHBOARD_PANEL_H
#define DASHBOARD_PANEL_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include "AppConstants.h"

class DashboardPanel : public wxPanel {
public:
    DashboardPanel(wxWindow* parent);
    
    void RefreshData();

private:
    void CreateLayout();
    void UpdateQuickStats();
    void UpdateRecentActivity();

    // Event handlers
    void OnQuickCheckIn(wxCommandEvent& event);
    void OnNewCustomer(wxCommandEvent& event);
    void OnDailyPayment(wxCommandEvent& event);
    void OnViewReports(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);

    // UI components
    wxStaticText* totalCustomersText;
    wxStaticText* activeCustomersText;
    wxStaticText* expiredCustomersText;
    wxStaticText* currentCheckedInText;
    wxStaticText* todayRevenueText;
    wxStaticText* weekRevenueText;
    wxStaticText* monthRevenueText;
    wxButton* quickCheckInBtn;
    wxButton* newCustomerBtn;
    wxButton* dailyPaymentBtn;
    wxButton* viewReportsBtn;
    wxListCtrl* recentActivityList;

    wxDECLARE_EVENT_TABLE();
};

#endif // DASHBOARD_PANEL_H