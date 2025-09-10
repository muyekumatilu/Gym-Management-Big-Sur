#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/timer.h>
#include "AppConstants.h"

// Forward declarations
class DatabaseManager;
class DashboardPanel;
class CustomerPanel;
class TransactionPanel;

class MainFrame : public wxFrame {
public:
    MainFrame();
    ~MainFrame();

    // Public methods for panel communication
    void RefreshCustomerPanel();
    void RefreshTransactionPanel();
    void RefreshInventoryPanel();
    void RefreshReportsPanel();
    void RefreshDashboardPanel();

private:
    void CreateMenuBar();
    void CreateToolBar();
    void CreateStatusBar();
    void UpdateStatusBar();
    void RefreshAllPanels();
    bool InitializeDatabase();

    // Event handlers
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnRefreshData(wxCommandEvent& event);
    void OnBackupDatabase(wxCommandEvent& event);
    void OnRestoreDatabase(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnNewCustomer(wxCommandEvent& event);
    void OnQuickCheckIn(wxCommandEvent& event);
    void OnDailyPayment(wxCommandEvent& event);
    void OnViewReports(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnClose(wxCloseEvent& event);

    // UI components
    wxMenuBar* menuBar;
    wxToolBar* toolBar;
    wxStatusBar* statusBar;
    wxNotebook* notebook;
    
    // Panels
    DashboardPanel* dashboardPanel;
    CustomerPanel* customerPanel;
    TransactionPanel* transactionPanel;
    void* inventoryPanel;  // Will be implemented in Phase 2
    void* reportsPanel;    // Will be implemented in Phase 3
    
    // Other members
    DatabaseManager* dbManager;
    wxTimer* refreshTimer;

    wxDECLARE_EVENT_TABLE();
};

#endif // MAIN_FRAME_H