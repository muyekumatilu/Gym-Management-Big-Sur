#include "DashboardPanel.h"
#include "MainFrame.h"
#include "QuickCheckInDialog.h"
#include "Customer.h"
#include "Transaction.h"
#include <wx/sizer.h>
#include <wx/statbox.h>

// Dashboard Panel Implementation
wxBEGIN_EVENT_TABLE(DashboardPanel, wxPanel)
    EVT_BUTTON(ID_QUICK_CHECKIN, DashboardPanel::OnQuickCheckIn)
    EVT_BUTTON(ID_NEW_CUSTOMER, DashboardPanel::OnNewCustomer)
    EVT_BUTTON(ID_DAILY_PAYMENT, DashboardPanel::OnDailyPayment)
    EVT_BUTTON(ID_VIEW_REPORTS, DashboardPanel::OnViewReports)
    EVT_BUTTON(ID_REFRESH_DATA, DashboardPanel::OnRefresh)
wxEND_EVENT_TABLE()

DashboardPanel::DashboardPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    CreateLayout();
    RefreshData();
}

void DashboardPanel::CreateLayout() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    wxStaticText* title = new wxStaticText(this, wxID_ANY, "Dashboard", 
                                          wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(16);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    mainSizer->Add(title, 0, wxEXPAND | wxALL, 10);
    
    // Quick stats section
    wxStaticBox* statsBox = new wxStaticBox(this, wxID_ANY, "Quick Statistics");
    wxStaticBoxSizer* statsSizer = new wxStaticBoxSizer(statsBox, wxVERTICAL);
    
    wxFlexGridSizer* statsGrid = new wxFlexGridSizer(4, 2, 10, 20);
    statsGrid->AddGrowableCol(1);
    
    // Customer stats
    statsGrid->Add(new wxStaticText(this, wxID_ANY, "Total Customers:"));
    totalCustomersText = new wxStaticText(this, wxID_ANY, "0");
    statsGrid->Add(totalCustomersText);
    
    statsGrid->Add(new wxStaticText(this, wxID_ANY, "Active Customers:"));
    activeCustomersText = new wxStaticText(this, wxID_ANY, "0");
    statsGrid->Add(activeCustomersText);
    
    statsGrid->Add(new wxStaticText(this, wxID_ANY, "Expired Customers:"));
    expiredCustomersText = new wxStaticText(this, wxID_ANY, "0");
    statsGrid->Add(expiredCustomersText);
    
    statsGrid->Add(new wxStaticText(this, wxID_ANY, "Currently Checked In:"));
    currentCheckedInText = new wxStaticText(this, wxID_ANY, "0");
    statsGrid->Add(currentCheckedInText);
    
    statsSizer->Add(statsGrid, 0, wxEXPAND | wxALL, 10);
    
    // Revenue section
    wxStaticBox* revenueBox = new wxStaticBox(this, wxID_ANY, "Revenue Summary");
    wxStaticBoxSizer* revenueSizer = new wxStaticBoxSizer(revenueBox, wxVERTICAL);
    
    wxFlexGridSizer* revenueGrid = new wxFlexGridSizer(3, 2, 10, 20);
    revenueGrid->AddGrowableCol(1);
    
    revenueGrid->Add(new wxStaticText(this, wxID_ANY, "Today's Revenue:"));
    todayRevenueText = new wxStaticText(this, wxID_ANY, "KSh 0.00");
    revenueGrid->Add(todayRevenueText);
    
    revenueGrid->Add(new wxStaticText(this, wxID_ANY, "This Week:"));
    weekRevenueText = new wxStaticText(this, wxID_ANY, "KSh 0.00");
    revenueGrid->Add(weekRevenueText);
    
    revenueGrid->Add(new wxStaticText(this, wxID_ANY, "This Month:"));
    monthRevenueText = new wxStaticText(this, wxID_ANY, "KSh 0.00");
    revenueGrid->Add(monthRevenueText);
    
    revenueSizer->Add(revenueGrid, 0, wxEXPAND | wxALL, 10);
    
    // Quick actions
    wxStaticBox* actionsBox = new wxStaticBox(this, wxID_ANY, "Quick Actions");
    wxStaticBoxSizer* actionsSizer = new wxStaticBoxSizer(actionsBox, wxHORIZONTAL);
    
    quickCheckInBtn = new wxButton(this, ID_QUICK_CHECKIN, "Quick Check-In");
    newCustomerBtn = new wxButton(this, ID_NEW_CUSTOMER, "New Customer");
    dailyPaymentBtn = new wxButton(this, ID_DAILY_PAYMENT, "Daily Payment");
    viewReportsBtn = new wxButton(this, ID_VIEW_REPORTS, "View Reports");
    
    actionsSizer->Add(quickCheckInBtn, 0, wxALL, 5);
    actionsSizer->Add(newCustomerBtn, 0, wxALL, 5);
    actionsSizer->Add(dailyPaymentBtn, 0, wxALL, 5);
    actionsSizer->Add(viewReportsBtn, 0, wxALL, 5);
    
    // Recent activity
    wxStaticBox* activityBox = new wxStaticBox(this, wxID_ANY, "Recent Activity");
    wxStaticBoxSizer* activitySizer = new wxStaticBoxSizer(activityBox, wxVERTICAL);
    
    recentActivityList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, 
                                       wxSize(-1, 200), wxLC_REPORT);
    recentActivityList->AppendColumn("Time", wxLIST_FORMAT_LEFT, 100);
    recentActivityList->AppendColumn("Activity", wxLIST_FORMAT_LEFT, 200);
    recentActivityList->AppendColumn("Customer", wxLIST_FORMAT_LEFT, 150);
    recentActivityList->AppendColumn("Amount", wxLIST_FORMAT_RIGHT, 100);
    
    activitySizer->Add(recentActivityList, 1, wxEXPAND | wxALL, 10);
    
    // Arrange main layout
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    topSizer->Add(statsSizer, 1, wxEXPAND | wxALL, 5);
    topSizer->Add(revenueSizer, 1, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(topSizer, 0, wxEXPAND);
    mainSizer->Add(actionsSizer, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(activitySizer, 1, wxEXPAND | wxALL, 5);
    
    SetSizer(mainSizer);
}

void DashboardPanel::RefreshData() {
    UpdateQuickStats();
    UpdateRecentActivity();
}

void DashboardPanel::UpdateQuickStats() {
    // Get customer statistics
    std::vector<Customer> allCustomers = Customer::getAllCustomers();
    int totalCustomers = allCustomers.size();
    int activeCustomers = 0;
    int expiredCustomers = 0;
    int checkedIn = 0; // TODO: Implement check-in tracking
    
    for (const Customer& customer : allCustomers) {
        if (customer.getStatus() == CustomerStatus::ACTIVE) {
            activeCustomers++;
        } else {
            expiredCustomers++;
        }
    }
    
    // Update customer stats
    totalCustomersText->SetLabel(wxString::Format("%d", totalCustomers));
    activeCustomersText->SetLabel(wxString::Format("%d", activeCustomers));
    expiredCustomersText->SetLabel(wxString::Format("%d", expiredCustomers));
    currentCheckedInText->SetLabel(wxString::Format("%d", checkedIn));
    
    // Calculate revenue (simplified for now)
    std::time_t today = std::time(nullptr);
    std::time_t weekStart = today - (7 * 24 * 60 * 60); // 7 days ago
    std::time_t monthStart = today - (30 * 24 * 60 * 60); // 30 days ago
    
    double todayRevenue = Transaction::getTotalRevenueByDate(today, today);
    double weekRevenue = Transaction::getTotalRevenueByDate(weekStart, today);
    double monthRevenue = Transaction::getTotalRevenueByDate(monthStart, today);
    
    todayRevenueText->SetLabel(wxString::Format("KSh %.2f", todayRevenue));
    weekRevenueText->SetLabel(wxString::Format("KSh %.2f", weekRevenue));
    monthRevenueText->SetLabel(wxString::Format("KSh %.2f", monthRevenue));
}

void DashboardPanel::UpdateRecentActivity() {
    recentActivityList->DeleteAllItems();
    
    // Get recent transactions
    std::vector<Transaction> recentTransactions = Transaction::getAllTransactions();
    
    // Show only the most recent 20 transactions
    int count = 0;
    for (auto it = recentTransactions.rbegin(); 
         it != recentTransactions.rend() && count < 20; 
         ++it, ++count) {
        
        const Transaction& transaction = *it;
        
        long index = recentActivityList->InsertItem(count, transaction.getFormattedDate());
        recentActivityList->SetItem(index, 1, transaction.getTransactionTypeString());
        
        // Get customer name
        Customer customer;
        if (customer.loadFromDatabase(transaction.getCustomerId())) {
            recentActivityList->SetItem(index, 2, customer.getFullName());
        } else {
            recentActivityList->SetItem(index, 2, "Unknown");
        }
        
        recentActivityList->SetItem(index, 3, transaction.getFormattedAmount());
    }
}

// Dashboard event handlers
void DashboardPanel::OnQuickCheckIn(wxCommandEvent& event) {
    QuickCheckInDialog dialog(this);
    
    if (dialog.ShowModal() == wxID_OK) {
        // Refresh all panels after successful check-in
        MainFrame* mainFrame = dynamic_cast<MainFrame*>(GetParent()->GetParent());
        if (mainFrame) {
            mainFrame->RefreshDashboardPanel();
            mainFrame->RefreshCustomerPanel();
            mainFrame->RefreshTransactionPanel();
        }
    }
}
void DashboardPanel::OnNewCustomer(wxCommandEvent& event) {
    // Switch to customer tab and trigger new customer
    MainFrame* parent = dynamic_cast<MainFrame*>(GetParent()->GetParent());
    if (parent) {
        // Switch to customer tab (index 1)
        wxNotebook* notebook = dynamic_cast<wxNotebook*>(GetParent());
        if (notebook) {
            notebook->SetSelection(1);
        }
    }
}

void DashboardPanel::OnDailyPayment(wxCommandEvent& event) {
    // Switch to transaction tab
    MainFrame* parent = dynamic_cast<MainFrame*>(GetParent()->GetParent());
    if (parent) {
        wxNotebook* notebook = dynamic_cast<wxNotebook*>(GetParent());
        if (notebook) {
            notebook->SetSelection(2);
        }
    }
}

void DashboardPanel::OnViewReports(wxCommandEvent& event) {
    wxMessageBox("Reports functionality will be implemented in Phase 3", 
                 "Reports", wxOK | wxICON_INFORMATION);
}

void DashboardPanel::OnRefresh(wxCommandEvent& event) {
    RefreshData();
}