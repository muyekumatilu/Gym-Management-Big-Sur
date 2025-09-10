#include "QuickCheckInDialog.h"
#include "Customer.h"
#include "Transaction.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

// Quick Check-in Dialog Implementation
wxBEGIN_EVENT_TABLE(QuickCheckInDialog, wxDialog)
    EVT_TEXT(wxID_ANY, QuickCheckInDialog::OnSearch)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, QuickCheckInDialog::OnCustomerSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, QuickCheckInDialog::OnCustomerDoubleClick)
    EVT_BUTTON(wxID_OK, QuickCheckInDialog::OnCheckIn)
    EVT_BUTTON(wxID_CANCEL, QuickCheckInDialog::OnCancel)
    EVT_CHOICE(wxID_ANY, QuickCheckInDialog::OnPaymentTypeChange)
wxEND_EVENT_TABLE()

QuickCheckInDialog::QuickCheckInDialog(wxWindow* parent) 
    : wxDialog(parent, wxID_ANY, "Quick Check-In", wxDefaultPosition, wxSize(600, 500)), 
      selectedCustomerId(-1) {
    CreateLayout();
    PopulateCustomerList();
    UpdateButtonStates();
}

void QuickCheckInDialog::CreateLayout() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    wxStaticText* title = new wxStaticText(this, wxID_ANY, "Quick Customer Check-In");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(14);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    mainSizer->Add(title, 0, wxALIGN_CENTER | wxALL, 10);
    
    // Search section
    wxStaticBox* searchBox = new wxStaticBox(this, wxID_ANY, "Search Customer");
    wxStaticBoxSizer* searchSizer = new wxStaticBoxSizer(searchBox, wxHORIZONTAL);
    
    searchSizer->Add(new wxStaticText(this, wxID_ANY, "Name or Phone:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    searchText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    searchSizer->Add(searchText, 1, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(searchSizer, 0, wxEXPAND | wxALL, 5);
    
    // Customer list
    wxStaticBox* listBox = new wxStaticBox(this, wxID_ANY, "Select Customer");
    wxStaticBoxSizer* listSizer = new wxStaticBoxSizer(listBox, wxVERTICAL);
    
    customerList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, 
                                 wxSize(-1, 250), wxLC_REPORT | wxLC_SINGLE_SEL);
    customerList->AppendColumn("Name", wxLIST_FORMAT_LEFT, 180);
    customerList->AppendColumn("Phone", wxLIST_FORMAT_LEFT, 120);
    customerList->AppendColumn("Membership", wxLIST_FORMAT_CENTER, 100);
    customerList->AppendColumn("Status", wxLIST_FORMAT_CENTER, 80);
    customerList->AppendColumn("Expiry", wxLIST_FORMAT_CENTER, 100);
    
    listSizer->Add(customerList, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(listSizer, 1, wxEXPAND | wxALL, 5);
    
    // Payment section (for daily customers or expired customers)
    wxStaticBox* paymentBox = new wxStaticBox(this, wxID_ANY, "Payment (if required)");
    wxStaticBoxSizer* paymentSizer = new wxStaticBoxSizer(paymentBox, wxHORIZONTAL);
    
    paymentSizer->Add(new wxStaticText(this, wxID_ANY, "Payment Type:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    paymentTypeChoice = new wxChoice(this, wxID_ANY);
    paymentTypeChoice->Append("Cash");
    paymentTypeChoice->Append("M-Pesa");
    paymentTypeChoice->SetSelection(0);
    paymentSizer->Add(paymentTypeChoice, 0, wxALL, 5);
    
    paymentSizer->Add(new wxStaticText(this, wxID_ANY, "M-Pesa Code:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    mpesaCodeText = new wxTextCtrl(this, wxID_ANY);
    mpesaCodeText->Enable(false);
    paymentSizer->Add(mpesaCodeText, 1, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(paymentSizer, 0, wxEXPAND | wxALL, 5);
    
    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    checkInBtn = new wxButton(this, wxID_OK, "Check In");
    cancelBtn = new wxButton(this, wxID_CANCEL, "Cancel");
    
    buttonSizer->Add(checkInBtn, 0, wxALL, 5);
    buttonSizer->Add(cancelBtn, 0, wxALL, 5);
    
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 10);
    
    SetSizer(mainSizer);
}

void QuickCheckInDialog::PopulateCustomerList() {
    std::vector<Customer> customers = Customer::getAllCustomers();
    PopulateCustomerList(customers);
}

void QuickCheckInDialog::PopulateCustomerList(const std::vector<Customer>& customers) {
    customerList->DeleteAllItems();
    
    for (size_t i = 0; i < customers.size(); ++i) {
        const Customer& customer = customers[i];
        
        long index = customerList->InsertItem(i, customer.getFullName());
        customerList->SetItem(index, 1, customer.getPhone());
        customerList->SetItem(index, 2, customer.getMembershipTypeString());
        customerList->SetItem(index, 3, customer.getStatusString());
        
        // Format expiry date
        if (customer.getExpiryDate() > 0) {
            std::time_t expiryTime = customer.getExpiryDate();
            struct tm* timeinfo = localtime(&expiryTime);
            char buffer[11];
            strftime(buffer, 11, "%Y-%m-%d", timeinfo);
            customerList->SetItem(index, 4, buffer);
        } else {
            customerList->SetItem(index, 4, "N/A");
        }
        
        // Store customer ID in item data
        customerList->SetItemData(index, customer.getCustomerId());
        
        // Highlight expired customers in red
        if (customer.getStatus() == CustomerStatus::EXPIRED) {
            customerList->SetItemTextColour(index, *wxRED);
        }
    }
}

void QuickCheckInDialog::UpdateButtonStates() {
    bool hasSelection = (selectedCustomerId != -1);
    checkInBtn->Enable(hasSelection);
}

bool QuickCheckInDialog::ProcessCheckIn() {
    if (selectedCustomerId == -1) return false;
    
    Customer customer;
    if (!customer.loadFromDatabase(selectedCustomerId)) {
        wxMessageBox("Failed to load customer data", "Error", wxOK | wxICON_ERROR);
        return false;
    }
    
    // Check if customer needs to pay (daily membership or expired)
    bool needsPayment = (customer.getMembershipType() == MembershipType::DAILY) || 
                        (customer.getStatus() == CustomerStatus::EXPIRED);
    
    if (needsPayment) {
        // Validate payment information
        if (paymentTypeChoice->GetSelection() == 1) { // M-Pesa
            if (mpesaCodeText->GetValue().IsEmpty()) {
                wxMessageBox("M-Pesa code is required for M-Pesa payments", 
                           "Payment Required", wxOK | wxICON_ERROR);
                return false;
            }
        }
        
        // Determine amount based on membership type
        double amount = 0.0;
        TransactionType transType = TransactionType::DAILY_FEE;
        std::string description = "Daily gym fee";
        
        if (customer.getMembershipType() == MembershipType::DAILY) {
            amount = 300.0; // Daily fee - adjust as needed
        } else if (customer.getStatus() == CustomerStatus::EXPIRED) {
            // Renewal payment
            switch (customer.getMembershipType()) {
                case MembershipType::WEEKLY:
                    amount = 1500.0; // Weekly fee - adjust as needed
                    break;
                case MembershipType::MONTHLY:
                    amount = 3500.0; // Monthly fee - adjust as needed
                    break;
                case MembershipType::TRAINER:
                    amount = 100.0; // Trainer fee - adjust as needed
                    break;
                default:
                    amount = 200.0;
            }
            transType = TransactionType::RENEWAL;
            description = customer.getMembershipTypeString() + " membership renewal";
        }
        
        // Create and save transaction
        Transaction transaction(selectedCustomerId, amount, 
                              (paymentTypeChoice->GetSelection() == 0) ? PaymentType::CASH : PaymentType::MPESA,
                              transType, description);
        
        if (paymentTypeChoice->GetSelection() == 1) { // M-Pesa
            transaction.setMpesaCode(mpesaCodeText->GetValue().ToStdString());
        }
        
        transaction.setStatus(TransactionStatus::COMPLETED);
        
        if (!transaction.saveToDatabase()) {
            wxMessageBox("Failed to save payment transaction", "Error", wxOK | wxICON_ERROR);
            return false;
        }
        
        // Update customer expiry if it was a renewal
        if (transType == TransactionType::RENEWAL) {
            customer.calculateExpiryDate();
            customer.updateInDatabase();
        }
    }
    
    // Perform check-in
    if (!customer.checkIn()) {
        wxMessageBox("Check-in failed", "Error", wxOK | wxICON_ERROR);
        return false;
    }
    
    wxString message = wxString::Format("Customer %s checked in successfully", customer.getFullName());
    if (needsPayment) {
        message += wxString::Format("\nPayment processed: KSh %.2f", 
                                  (customer.getMembershipType() == MembershipType::DAILY) ? 100.0 : 
                                  (customer.getMembershipType() == MembershipType::WEEKLY) ? 500.0 :
                                  (customer.getMembershipType() == MembershipType::MONTHLY) ? 1500.0 : 2500.0);
    }
    
    wxMessageBox(message, "Check-In Successful", wxOK | wxICON_INFORMATION);
    return true;
}

// Event handlers
void QuickCheckInDialog::OnSearch(wxCommandEvent& event) {
    wxString searchTerm = searchText->GetValue();
    std::vector<Customer> customers;
    
    if (!searchTerm.IsEmpty()) {
        customers = Customer::findByName(searchTerm.ToStdString());
        
        // Also search by phone if no name matches found
        if (customers.empty()) {
            std::vector<Customer> allCustomers = Customer::getAllCustomers();
            for (const Customer& customer : allCustomers) {
                if (customer.getPhone().find(searchTerm.ToStdString()) != std::string::npos) {
                    customers.push_back(customer);
                }
            }
        }
    } else {
        customers = Customer::getAllCustomers();
    }
    
    PopulateCustomerList(customers);
    selectedCustomerId = -1;
    UpdateButtonStates();
}

void QuickCheckInDialog::OnCustomerSelected(wxListEvent& event) {
    long item = event.GetIndex();
    if (item != -1) {
        selectedCustomerId = customerList->GetItemData(item);
    }
    UpdateButtonStates();
}

void QuickCheckInDialog::OnCustomerDoubleClick(wxListEvent& event) {
    wxCommandEvent dummyEvent;
    OnCheckIn(dummyEvent);
}

void QuickCheckInDialog::OnCheckIn(wxCommandEvent& event) {
    if (ProcessCheckIn()) {
        EndModal(wxID_OK);
    }
}

void QuickCheckInDialog::OnCancel(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

void QuickCheckInDialog::OnPaymentTypeChange(wxCommandEvent& event) {
    bool isMpesa = (paymentTypeChoice->GetSelection() == 1);
    mpesaCodeText->Enable(isMpesa);
    
    if (!isMpesa) {
        mpesaCodeText->Clear();
    }
}