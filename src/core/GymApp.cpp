// File: src/core/GymApp.cpp
#include "GymApp.h"
#include "MainFrame.h"
#include "DatabaseManager.h"
#include <wx/msgdlg.h>

// Application initialization
bool GymApp::OnInit() {
    // Initialize database
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db->connect()) {
        wxMessageBox("Failed to connect to database!", "Database Error", 
                     wxOK | wxICON_ERROR);
        return false;
    }
    
    // Create main frame
    MainFrame* frame = new MainFrame();
    frame->Show(true);
    
    return true;
}

int GymApp::OnExit() {
    // Cleanup database connection
    DatabaseManager* db = DatabaseManager::getInstance();
    if (db) {
        db->disconnect();
    }
    return 0;
}