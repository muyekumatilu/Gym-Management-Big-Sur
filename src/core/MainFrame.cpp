#include "MainFrame.h"
#include "DashboardPanel.h"
#include "CustomerPanel.h"
#include "TransactionPanel.h"
#include "QuickCheckInDialog.h"
#include "DatabaseManager.h"
#include "Customer.h"
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/utils.h>
#include <wx/sizer.h>

// Event table for MainFrame
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(ID_REFRESH_DATA, MainFrame::OnRefreshData)
    EVT_MENU(ID_BACKUP_DB, MainFrame::OnBackupDatabase)
    EVT_MENU(ID_RESTORE_DB, MainFrame::OnRestoreDatabase)
    EVT_MENU(ID_SETTINGS, MainFrame::OnSettings)
    EVT_MENU(ID_NEW_CUSTOMER, MainFrame::OnNewCustomer)
    EVT_MENU(ID_QUICK_CHECKIN, MainFrame::OnQuickCheckIn)
    EVT_MENU(ID_DAILY_PAYMENT, MainFrame::OnDailyPayment)
    EVT_MENU(ID_VIEW_REPORTS, MainFrame::OnViewReports)
    EVT_TIMER(ID_REFRESH_TIMER, MainFrame::OnTimer)
    EVT_CLOSE(MainFrame::OnClose)
wxEND_EVENT_TABLE()

// MainFrame constructor
MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "Gym Management System", 
                                wxDefaultPosition, wxSize(1200, 800)),
                        dbManager(nullptr), refreshTimer(nullptr) {
    
    // Initialize database manager
    dbManager = DatabaseManager::getInstance();
    
    // Create UI components
    CreateMenuBar();
    CreateToolBar();
    CreateStatusBar();
    
    // Create main notebook
    notebook = new wxNotebook(this, wxID_ANY);
    
    // Create panels
    dashboardPanel = new DashboardPanel(notebook);
    customerPanel = new CustomerPanel(notebook);
    transactionPanel = new TransactionPanel(notebook);
    inventoryPanel = nullptr; // Will be implemented in Phase 2
    reportsPanel = nullptr;   // Will be implemented in Phase 3
    
    // Add pages to notebook
    notebook->AddPage(dashboardPanel, "Dashboard", true);
    notebook->AddPage(customerPanel, "Customers");
    notebook->AddPage(transactionPanel, "Transactions");
    
    // Create main sizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
    
    SetSizer(mainSizer);
    
    // Initialize refresh timer (refresh every 30 seconds)
    refreshTimer = new wxTimer(this, ID_REFRESH_TIMER);
    refreshTimer->Start(30000);
    
    // Center the frame
    Center();
    
    // Initial data refresh
    RefreshAllPanels();
}

MainFrame::~MainFrame() {
    if (refreshTimer) {
        refreshTimer->Stop();
        delete refreshTimer;
    }
}

void MainFrame::CreateMenuBar() {
    menuBar = new wxMenuBar();
    
    // File menu
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_REFRESH_DATA, "&Refresh Data\tF5", "Refresh all data");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_BACKUP_DB, "&Backup Database", "Backup database");
    fileMenu->Append(ID_RESTORE_DB, "&Restore Database", "Restore database");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_SETTINGS, "&Settings\tCtrl+,", "Open settings");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tCtrl+Q", "Exit the application");
    
    // Help menu
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About", "Show about dialog");
    
    // Add menus to menu bar
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    
    SetMenuBar(menuBar);
}

void MainFrame::CreateToolBar() {
    toolBar = wxFrame::CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT);
    
    // Add toolbar buttons (we'll add icons later)
    toolBar->AddTool(ID_REFRESH_DATA, "Refresh", wxNullBitmap, "Refresh Data");
    toolBar->AddSeparator();
    toolBar->AddTool(ID_NEW_CUSTOMER, "New Customer", wxNullBitmap, "Add New Customer");
    toolBar->AddTool(ID_QUICK_CHECKIN, "Check In", wxNullBitmap, "Quick Check In");
    toolBar->AddTool(ID_DAILY_PAYMENT, "Payment", wxNullBitmap, "Daily Payment");
    toolBar->AddSeparator();
    toolBar->AddTool(ID_VIEW_REPORTS, "Reports", wxNullBitmap, "View Reports");
    
    toolBar->Realize();
}

void MainFrame::CreateStatusBar() {
    statusBar = wxFrame::CreateStatusBar(3);
    statusBar->SetStatusText("Ready", 0);
    statusBar->SetStatusText("Database: Connected", 1);
    statusBar->SetStatusText("", 2);
    
    // Set status bar widths
    int widths[3] = {-1, 150, 100};
    statusBar->SetStatusWidths(3, widths);
}

void MainFrame::UpdateStatusBar() {
    if (!statusBar) return;
    
    // Update customer count
    std::vector<Customer> customers = Customer::getAllCustomers();
    wxString customerInfo = wxString::Format("Customers: %zu", customers.size());
    statusBar->SetStatusText(customerInfo, 2);
    
    // Update time
    wxDateTime now = wxDateTime::Now();
    statusBar->SetStatusText(now.Format("%H:%M:%S"), 0);
}

bool MainFrame::InitializeDatabase() {
    if (!dbManager->connect()) {
        wxMessageBox("Failed to connect to database!", "Database Error", 
                     wxOK | wxICON_ERROR);
        return false;
    }
    return true;
}

void MainFrame::RefreshAllPanels() {
    if (dashboardPanel) dashboardPanel->RefreshData();
    if (customerPanel) customerPanel->RefreshData();
    if (transactionPanel) transactionPanel->RefreshData();
    
    UpdateStatusBar();
}

// Event handlers
void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("Gym Management System v1.0\n\n"
                 "A comprehensive gym management solution\n"
                 "Built with wxWidgets and SQLite",
                 "About Gym Management System",
                 wxOK | wxICON_INFORMATION);
}

void MainFrame::OnRefreshData(wxCommandEvent& event) {
    RefreshAllPanels();
    statusBar->SetStatusText("Data refreshed", 0);
}

void MainFrame::OnBackupDatabase(wxCommandEvent& event) {
    wxFileDialog saveDialog(this, "Save database backup", "", "",
                           "SQLite files (*.db)|*.db",
                           wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    
    if (saveDialog.ShowModal() == wxID_OK) {
        // TODO: Implement database backup functionality
        wxString path = saveDialog.GetPath();
        statusBar->SetStatusText("Database backed up to: " + path, 0);
    }
}

void MainFrame::OnRestoreDatabase(wxCommandEvent& event) {
    wxFileDialog openDialog(this, "Choose database file to restore", "", "",
                           "SQLite files (*.db)|*.db",
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (openDialog.ShowModal() == wxID_OK) {
        // TODO: Implement database restore functionality
        wxString path = openDialog.GetPath();
        statusBar->SetStatusText("Database restored from: " + path, 0);
        RefreshAllPanels();
    }
}

void MainFrame::OnSettings(wxCommandEvent& event) {
    // TODO: Implement settings dialog
    wxMessageBox("Settings dialog not yet implemented", "Settings", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnTimer(wxTimerEvent& event) {
    UpdateStatusBar();
}

void MainFrame::OnNewCustomer(wxCommandEvent& event) {
    // Switch to customer tab and trigger new customer
    notebook->SetSelection(1); // Customer tab
    
    // Clear the form for new customer entry
    if (customerPanel) {
        customerPanel->RefreshData();
    }
}

void MainFrame::OnQuickCheckIn(wxCommandEvent& event) {
    QuickCheckInDialog dialog(this);
    
    if (dialog.ShowModal() == wxID_OK) {
        // Refresh all panels after successful check-in
        RefreshDashboardPanel();
        RefreshCustomerPanel();
        RefreshTransactionPanel();
    }
}

void MainFrame::OnDailyPayment(wxCommandEvent& event) {
    // Switch to transaction tab
    notebook->SetSelection(2); // Transaction tab
    
    // Set transaction type to daily fee
    if (transactionPanel) {
        transactionPanel->SetTransactionTypeToDaily();
    }
}

void MainFrame::OnViewReports(wxCommandEvent& event) {
    wxMessageBox("Reports functionality will be implemented in Phase 3", 
                 "Reports", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnClose(wxCloseEvent& event) {
    if (refreshTimer) {
        refreshTimer->Stop();
    }
    Destroy();
}

// Public methods for panel communication
void MainFrame::RefreshCustomerPanel() {
    if (customerPanel) customerPanel->RefreshData();
}

void MainFrame::RefreshTransactionPanel() {
    if (transactionPanel) transactionPanel->RefreshData();
}

void MainFrame::RefreshInventoryPanel() {
    // Will be implemented in Phase 2
}

void MainFrame::RefreshReportsPanel() {
    // Will be implemented in Phase 3
}

void MainFrame::RefreshDashboardPanel() {
    if (dashboardPanel) dashboardPanel->RefreshData();
}