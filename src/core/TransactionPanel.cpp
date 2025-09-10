#include "TransactionPanel.h"
#include "MainFrame.h"
#include "Customer.h"
#include "Transaction.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

// Transaction Panel Implementation
wxBEGIN_EVENT_TABLE(TransactionPanel, wxPanel)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, TransactionPanel::OnTransactionSelected)
    EVT_BUTTON(wxID_ADD, TransactionPanel::OnAddTransaction)
    EVT_BUTTON(wxID_EDIT, TransactionPanel::OnEditTransaction)
    EVT_BUTTON(wxID_DELETE, TransactionPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_UNDO, TransactionPanel::OnRefundTransaction)
    EVT_BUTTON(wxID_PRINT, TransactionPanel::OnPrintReceipt)
    EVT_BUTTON(wxID_APPLY, TransactionPanel::OnFilterTransactions)
    EVT_BUTTON(wxID_CLEAR, TransactionPanel::OnClearFilter)
    EVT_CHOICE(wxID_ANY, TransactionPanel::OnPaymentTypeChange)
    EVT_TEXT_ENTER(wxID_ANY, TransactionPanel::OnTextEnter)
wxEND_EVENT_TABLE()

TransactionPanel::TransactionPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY), selectedTransactionId(-1) {
    CreateLayout();
    RefreshData();
}

void TransactionPanel::CreateLayout() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    wxStaticText* title = new wxStaticText(this, wxID_ANY, "Transaction Management", 
                                          wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(16);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    mainSizer->Add(title, 0, wxEXPAND | wxALL, 10);
    
    // Filter section
    wxStaticBox* filterBox = new wxStaticBox(this, wxID_ANY, "Filters");
    wxStaticBoxSizer* filterSizer = new wxStaticBoxSizer(filterBox, wxHORIZONTAL);
    
    filterSizer->Add(new wxStaticText(this, wxID_ANY, "From:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    fromDate = new wxDatePickerCtrl(this, wxID_ANY);
    filterSizer->Add(fromDate, 0, wxALL, 5);
    
    filterSizer->Add(new wxStaticText(this, wxID_ANY, "To:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    toDate = new wxDatePickerCtrl(this, wxID_ANY);
    filterSizer->Add(toDate, 0, wxALL, 5);
    
    filterSizer->Add(new wxStaticText(this, wxID_ANY, "Payment:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    paymentTypeFilter = new wxChoice(this, wxID_ANY);
    paymentTypeFilter->Append("All");
    paymentTypeFilter->Append("Cash");
    paymentTypeFilter->Append("M-Pesa");
    paymentTypeFilter->SetSelection(0);
    filterSizer->Add(paymentTypeFilter, 0, wxALL, 5);
    
    filterSizer->Add(new wxStaticText(this, wxID_ANY, "Type:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    transactionTypeFilter = new wxChoice(this, wxID_ANY);
    transactionTypeFilter->Append("All");
    transactionTypeFilter->Append("Membership Payment");
    transactionTypeFilter->Append("Daily Fee");
    transactionTypeFilter->Append("Renewal");
    transactionTypeFilter->Append("Refund");
    transactionTypeFilter->Append("Other");
    transactionTypeFilter->SetSelection(0);
    filterSizer->Add(transactionTypeFilter, 0, wxALL, 5);
    
    wxButton* filterBtn = new wxButton(this, wxID_APPLY, "Filter");
    wxButton* clearFilterBtn = new wxButton(this, wxID_CLEAR, "Clear");
    filterSizer->Add(filterBtn, 0, wxALL, 5);
    filterSizer->Add(clearFilterBtn, 0, wxALL, 5);
    
    mainSizer->Add(filterSizer, 0, wxEXPAND | wxALL, 5);
    
    // Main content area
    wxBoxSizer* contentSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Transaction list (left side)
    wxStaticBox* listBox = new wxStaticBox(this, wxID_ANY, "Transaction List");
    wxStaticBoxSizer* listSizer = new wxStaticBoxSizer(listBox, wxVERTICAL);
    
    transactionList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, 
                                   wxSize(700, 400), wxLC_REPORT | wxLC_SINGLE_SEL);
    transactionList->AppendColumn("ID", wxLIST_FORMAT_RIGHT, 50);
    transactionList->AppendColumn("Date", wxLIST_FORMAT_LEFT, 100);
    transactionList->AppendColumn("Customer", wxLIST_FORMAT_LEFT, 150);
    transactionList->AppendColumn("Amount", wxLIST_FORMAT_RIGHT, 100);
    transactionList->AppendColumn("Payment", wxLIST_FORMAT_CENTER, 80);
    transactionList->AppendColumn("Type", wxLIST_FORMAT_LEFT, 120);
    transactionList->AppendColumn("Status", wxLIST_FORMAT_CENTER, 80);
    
    listSizer->Add(transactionList, 1, wxEXPAND | wxALL, 5);
    
    // Action buttons for transaction list
    wxBoxSizer* listButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    addTransactionBtn = new wxButton(this, wxID_ADD, "Add Transaction");
    editTransactionBtn = new wxButton(this, wxID_EDIT, "Edit");
    deleteTransactionBtn = new wxButton(this, wxID_DELETE, "Delete");
    refundBtn = new wxButton(this, wxID_UNDO, "Refund");
    printReceiptBtn = new wxButton(this, wxID_PRINT, "Print Receipt");
    
    listButtonSizer->Add(addTransactionBtn, 0, wxALL, 5);
    listButtonSizer->Add(editTransactionBtn, 0, wxALL, 5);
    listButtonSizer->Add(deleteTransactionBtn, 0, wxALL, 5);
    listButtonSizer->Add(refundBtn, 0, wxALL, 5);
    listButtonSizer->Add(printReceiptBtn, 0, wxALL, 5);
    
    listSizer->Add(listButtonSizer, 0, wxALIGN_CENTER);
    
    // Summary section
    wxStaticBox* summaryBox = new wxStaticBox(this, wxID_ANY, "Summary");
    wxStaticBoxSizer* summarySizer = new wxStaticBoxSizer(summaryBox, wxVERTICAL);
    
    wxFlexGridSizer* summaryGrid = new wxFlexGridSizer(4, 2, 5, 10);
    summaryGrid->AddGrowableCol(1);
    
    summaryGrid->Add(new wxStaticText(this, wxID_ANY, "Total Amount:"));
    totalAmountText = new wxStaticText(this, wxID_ANY, "KSh 0.00");
    summaryGrid->Add(totalAmountText);
    
    summaryGrid->Add(new wxStaticText(this, wxID_ANY, "Cash:"));
    cashAmountText = new wxStaticText(this, wxID_ANY, "KSh 0.00");
    summaryGrid->Add(cashAmountText);
    
    summaryGrid->Add(new wxStaticText(this, wxID_ANY, "M-Pesa:"));
    mpesaAmountText = new wxStaticText(this, wxID_ANY, "KSh 0.00");
    summaryGrid->Add(mpesaAmountText);
    
    summaryGrid->Add(new wxStaticText(this, wxID_ANY, "Count:"));
    transactionCountText = new wxStaticText(this, wxID_ANY, "0");
    summaryGrid->Add(transactionCountText);
    
    summarySizer->Add(summaryGrid, 0, wxEXPAND | wxALL, 10);
    listSizer->Add(summarySizer, 0, wxEXPAND | wxALL, 5);
    
    contentSizer->Add(listSizer, 2, wxEXPAND | wxALL, 5);
    
    // Transaction form (right side)
    wxStaticBox* formBox = new wxStaticBox(this, wxID_ANY, "Transaction Details");
    wxStaticBoxSizer* formSizer = new wxStaticBoxSizer(formBox, wxVERTICAL);
    
    wxFlexGridSizer* formGrid = new wxFlexGridSizer(6, 2, 10, 10);
    formGrid->AddGrowableCol(1);
    
    formGrid->Add(new wxStaticText(this, wxID_ANY, "Customer:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    customerChoice = new wxChoice(this, wxID_ANY);
    formGrid->Add(customerChoice, 1, wxEXPAND);
    
    formGrid->Add(new wxStaticText(this, wxID_ANY, "Amount:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    amountText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    formGrid->Add(amountText, 1, wxEXPAND);
    
    formGrid->Add(new wxStaticText(this, wxID_ANY, "Payment Type:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    paymentTypeChoice = new wxChoice(this, wxID_ANY);
    paymentTypeChoice->Append("Cash");
    paymentTypeChoice->Append("M-Pesa");
    paymentTypeChoice->SetSelection(0);
    formGrid->Add(paymentTypeChoice, 1, wxEXPAND);
    
    formGrid->Add(new wxStaticText(this, wxID_ANY, "Transaction Type:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    transactionTypeChoice = new wxChoice(this, wxID_ANY);
    transactionTypeChoice->Append("Membership Payment");
    transactionTypeChoice->Append("Daily Fee");
    transactionTypeChoice->Append("Renewal");
    transactionTypeChoice->Append("Refund");
    transactionTypeChoice->Append("Other");
    transactionTypeChoice->SetSelection(0);
    formGrid->Add(transactionTypeChoice, 1, wxEXPAND);
    
    formGrid->Add(new wxStaticText(this, wxID_ANY, "Description:"), 0, wxALIGN_RIGHT | wxALIGN_TOP);
    descriptionText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_PROCESS_ENTER);
    descriptionText->SetMinSize(wxSize(-1, 60));
    formGrid->Add(descriptionText, 1, wxEXPAND);
    
    formGrid->Add(new wxStaticText(this, wxID_ANY, "M-Pesa Code:"), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    mpesaCodeText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    mpesaCodeText->Enable(false); // Initially disabled
    formGrid->Add(mpesaCodeText, 1, wxEXPAND);
    
    formSizer->Add(formGrid, 0, wxEXPAND | wxALL, 10);
    
    contentSizer->Add(formSizer, 1, wxEXPAND | wxALL, 5);
    
    mainSizer->Add(contentSizer, 1, wxEXPAND);
    
    SetSizer(mainSizer);
    
    // Initially disable form buttons
    UpdateButtonStates();
}

void TransactionPanel::RefreshData() {
    PopulateTransactionList();
    PopulateCustomerChoice();
    ClearTransactionForm();
    selectedTransactionId = -1;
    UpdateButtonStates();
    UpdateSummary();
}

void TransactionPanel::PopulateTransactionList() {
    std::vector<Transaction> transactions = Transaction::getAllTransactions();
    PopulateTransactionList(transactions);
}

void TransactionPanel::PopulateTransactionList(const std::vector<Transaction>& transactions) {
    transactionList->DeleteAllItems();
    
    for (size_t i = 0; i < transactions.size(); ++i) {
        const Transaction& transaction = transactions[i];
        
        long index = transactionList->InsertItem(i, wxString::Format("%d", transaction.getTransactionId()));
        transactionList->SetItem(index, 1, transaction.getFormattedDate());
        
        // Get customer name
        Customer customer;
        if (customer.loadFromDatabase(transaction.getCustomerId())) {
            transactionList->SetItem(index, 2, customer.getFullName());
        } else {
            transactionList->SetItem(index, 2, "Unknown");
        }
        
        transactionList->SetItem(index, 3, transaction.getFormattedAmount());
        transactionList->SetItem(index, 4, transaction.getPaymentTypeString());
        transactionList->SetItem(index, 5, transaction.getTransactionTypeString());
        transactionList->SetItem(index, 6, transaction.getStatusString());
        
        // Store transaction ID in item data
        transactionList->SetItemData(index, transaction.getTransactionId());
    }
}

void TransactionPanel::PopulateCustomerChoice() {
    customerChoice->Clear();
    std::vector<Customer> customers = Customer::getAllCustomers();
    
    for (const Customer& customer : customers) {
        wxString customerInfo = wxString::Format("%s (%s)", 
                                               customer.getFullName(), 
                                               customer.getPhone());
        customerChoice->Append(customerInfo, new wxStringClientData(wxString::Format("%d", customer.getCustomerId())));
    }
    
    if (customerChoice->GetCount() > 0) {
        customerChoice->SetSelection(0);
    }
}

void TransactionPanel::UpdateTransactionForm(const Transaction& transaction) {
    // Find and select customer
    for (unsigned int i = 0; i < customerChoice->GetCount(); ++i) {
        wxStringClientData* data = dynamic_cast<wxStringClientData*>(customerChoice->GetClientObject(i));
        if (data && data->GetData() == wxString::Format("%d", transaction.getCustomerId())) {
            customerChoice->SetSelection(i);
            break;
        }
    }
    
    amountText->SetValue(wxString::Format("%.2f", transaction.getAmount()));
    
    // Set payment type
    switch (transaction.getPaymentType()) {
        case PaymentType::CASH:
            paymentTypeChoice->SetSelection(0);
            break;
        case PaymentType::MPESA:
            paymentTypeChoice->SetSelection(1);
            break;
    }
    
    // Set transaction type
    switch (transaction.getTransactionType()) {
        case TransactionType::MEMBERSHIP_PAYMENT:
            transactionTypeChoice->SetSelection(0);
            break;
        case TransactionType::DAILY_FEE:
            transactionTypeChoice->SetSelection(1);
            break;
        case TransactionType::RENEWAL:
            transactionTypeChoice->SetSelection(2);
            break;
        case TransactionType::REFUND:
            transactionTypeChoice->SetSelection(3);
            break;
        case TransactionType::OTHER:
            transactionTypeChoice->SetSelection(4);
            break;
    }
    
    descriptionText->SetValue(transaction.getDescription());
    mpesaCodeText->SetValue(transaction.getMpesaCode());
    
    // Enable/disable M-Pesa code field based on payment type
    wxCommandEvent dummyEvent;
    OnPaymentTypeChange(dummyEvent);
}

void TransactionPanel::ClearTransactionForm() {
    if (customerChoice->GetCount() > 0) {
        customerChoice->SetSelection(0);
    }
    amountText->Clear();
    paymentTypeChoice->SetSelection(0);
    transactionTypeChoice->SetSelection(0);
    descriptionText->Clear();
    mpesaCodeText->Clear();
    mpesaCodeText->Enable(false);
}

bool TransactionPanel::ValidateTransactionForm() {
    if (customerChoice->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a customer", "Validation Error", wxOK | wxICON_ERROR);
        return false;
    }
    
    double amount;
    if (!amountText->GetValue().ToDouble(&amount) || amount <= 0) {
        wxMessageBox("Please enter a valid amount", "Validation Error", wxOK | wxICON_ERROR);
        return false;
    }
    
    if (paymentTypeChoice->GetSelection() == 1) { // M-Pesa
        if (mpesaCodeText->GetValue().IsEmpty()) {
            wxMessageBox("M-Pesa code is required for M-Pesa payments", "Validation Error", wxOK | wxICON_ERROR);
            return false;
        }
    }
    
    return true;
}

Transaction TransactionPanel::GetTransactionFromForm() {
    Transaction transaction;
    
    // Get customer ID
    wxStringClientData* data = dynamic_cast<wxStringClientData*>(
        customerChoice->GetClientObject(customerChoice->GetSelection()));
    if (data) {
        long customerId;
        data->GetData().ToLong(&customerId);
        transaction.setCustomerId(customerId);
    }
    
    double amount;
    amountText->GetValue().ToDouble(&amount);
    transaction.setAmount(amount);
    
    // Set payment type
    PaymentType paymentType = (paymentTypeChoice->GetSelection() == 0) ? 
                             PaymentType::CASH : PaymentType::MPESA;
    transaction.setPaymentType(paymentType);
    
    // Set transaction type
    TransactionType transactionType;
    switch (transactionTypeChoice->GetSelection()) {
        case 0: transactionType = TransactionType::MEMBERSHIP_PAYMENT; break;
        case 1: transactionType = TransactionType::DAILY_FEE; break;
        case 2: transactionType = TransactionType::RENEWAL; break;
        case 3: transactionType = TransactionType::REFUND; break;
        case 4: transactionType = TransactionType::OTHER; break;
        default: transactionType = TransactionType::OTHER; break;
    }
    transaction.setTransactionType(transactionType);
    
    transaction.setDescription(descriptionText->GetValue().ToStdString());
    
    if (paymentType == PaymentType::MPESA) {
        transaction.setMpesaCode(mpesaCodeText->GetValue().ToStdString());
    }
    
    return transaction;
}

void TransactionPanel::UpdateSummary() {
    std::vector<Transaction> transactions = Transaction::getAllTransactions();
    
    double totalAmount = 0.0;
    double cashAmount = 0.0;
    double mpesaAmount = 0.0;
    int transactionCount = transactions.size();
    
    for (const Transaction& transaction : transactions) {
        if (transaction.getStatus() == TransactionStatus::COMPLETED) {
            totalAmount += transaction.getAmount();
            
            if (transaction.getPaymentType() == PaymentType::CASH) {
                cashAmount += transaction.getAmount();
            } else {
                mpesaAmount += transaction.getAmount();
            }
        }
    }
    
    totalAmountText->SetLabel(wxString::Format("KSh %.2f", totalAmount));
    cashAmountText->SetLabel(wxString::Format("KSh %.2f", cashAmount));
    mpesaAmountText->SetLabel(wxString::Format("KSh %.2f", mpesaAmount));
    transactionCountText->SetLabel(wxString::Format("%d", transactionCount));
}

void TransactionPanel::UpdateButtonStates() {
    bool hasSelection = (selectedTransactionId != -1);
    
    editTransactionBtn->Enable(hasSelection);
    deleteTransactionBtn->Enable(hasSelection);
    refundBtn->Enable(hasSelection);
    printReceiptBtn->Enable(hasSelection);
}

void TransactionPanel::SelectTransaction(int transactionId) {
    // Find and select transaction in list
    for (long i = 0; i < transactionList->GetItemCount(); ++i) {
        if (transactionList->GetItemData(i) == transactionId) {
            transactionList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            transactionList->EnsureVisible(i);
            break;
        }
    }
}

void TransactionPanel::AddTransactionForCustomer(int customerId, double amount) {
    // Find and select customer
    for (unsigned int i = 0; i < customerChoice->GetCount(); ++i) {
        wxStringClientData* data = dynamic_cast<wxStringClientData*>(customerChoice->GetClientObject(i));
        if (data && data->GetData() == wxString::Format("%d", customerId)) {
            customerChoice->SetSelection(i);
            break;
        }
    }
    
    if (amount > 0.0) {
        amountText->SetValue(wxString::Format("%.2f", amount));
    }
}

void TransactionPanel::SetTransactionTypeToDaily() {
    // Set transaction type to Daily Fee
    transactionTypeChoice->SetSelection(1); // Daily Fee is at index 1
    
    // Also set a default description
    descriptionText->SetValue("Daily gym access fee");
}

// Transaction Panel Event Handlers
void TransactionPanel::OnTransactionSelected(wxListEvent& event) {
    long item = event.GetIndex();
    if (item != -1) {
        selectedTransactionId = transactionList->GetItemData(item);
        
        // Load transaction details
        Transaction transaction;
        if (transaction.loadFromDatabase(selectedTransactionId)) {
            UpdateTransactionForm(transaction);
        }
    }
    UpdateButtonStates();
}

void TransactionPanel::OnAddTransaction(wxCommandEvent& event) {
    if (!ValidateTransactionForm()) {
        return;
    }
    
    Transaction transaction = GetTransactionFromForm();
    
    if (transaction.saveToDatabase()) {
        wxMessageBox("Transaction added successfully", "Success", wxOK | wxICON_INFORMATION);
        RefreshData();
        
        // Refresh main dashboard
        MainFrame* mainFrame = dynamic_cast<MainFrame*>(GetParent()->GetParent());
        if (mainFrame) {
            mainFrame->RefreshDashboardPanel();
            mainFrame->RefreshCustomerPanel();
        }
    } else {
        wxMessageBox("Failed to add transaction", "Error", wxOK | wxICON_ERROR);
    }
}

void TransactionPanel::OnEditTransaction(wxCommandEvent& event) {
    if (selectedTransactionId == -1) return;
    
    if (!ValidateTransactionForm()) {
        return;
    }
    
    Transaction transaction = GetTransactionFromForm();
    transaction.setTransactionId(selectedTransactionId);
    
    if (transaction.updateInDatabase()) {
        wxMessageBox("Transaction updated successfully", "Success", wxOK | wxICON_INFORMATION);
        RefreshData();
        
        // Refresh main dashboard
        MainFrame* mainFrame = dynamic_cast<MainFrame*>(GetParent()->GetParent());
        if (mainFrame) {
            mainFrame->RefreshDashboardPanel();
        }
    } else {
        wxMessageBox("Failed to update transaction", "Error", wxOK | wxICON_ERROR);
    }
}

void TransactionPanel::OnDeleteTransaction(wxCommandEvent& event) {
    if (selectedTransactionId == -1) return;
    
    int result = wxMessageBox("Are you sure you want to delete this transaction?",
                             "Confirm Delete", 
                             wxYES_NO | wxICON_QUESTION);
    
    if (result == wxYES) {
        if (Transaction::deleteTransaction(selectedTransactionId)) {
            wxMessageBox("Transaction deleted successfully", "Success", wxOK | wxICON_INFORMATION);
            RefreshData();
            
            // Refresh main dashboard
            MainFrame* mainFrame = dynamic_cast<MainFrame*>(GetParent()->GetParent());
            if (mainFrame) {
                mainFrame->RefreshDashboardPanel();
            }
        } else {
            wxMessageBox("Failed to delete transaction", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void TransactionPanel::OnRefundTransaction(wxCommandEvent& event) {
    if (selectedTransactionId == -1) return;
    
    wxString reason = wxGetTextFromUser("Enter refund reason:", "Refund Transaction", "");
    if (!reason.IsEmpty()) {
        Transaction transaction;
        if (transaction.loadFromDatabase(selectedTransactionId)) {
            if (transaction.refundTransaction(reason.ToStdString())) {
                wxMessageBox("Transaction refunded successfully", "Success", wxOK | wxICON_INFORMATION);
                RefreshData();
                
                // Refresh main dashboard
                MainFrame* mainFrame = dynamic_cast<MainFrame*>(GetParent()->GetParent());
                if (mainFrame) {
                    mainFrame->RefreshDashboardPanel();
                }
            } else {
                wxMessageBox("Failed to refund transaction", "Error", wxOK | wxICON_ERROR);
            }
        }
    }
}

void TransactionPanel::OnPrintReceipt(wxCommandEvent& event) {
    if (selectedTransactionId == -1) return;
    
    // TODO: Implement receipt printing
    wxMessageBox("Receipt printing functionality will be implemented next", 
                 "Print Receipt", wxOK | wxICON_INFORMATION);
}

void TransactionPanel::OnFilterTransactions(wxCommandEvent& event) {
    // TODO: Implement date and type filtering
    wxMessageBox("Transaction filtering will be implemented next", 
                 "Filter Transactions", wxOK | wxICON_INFORMATION);
}

void TransactionPanel::OnClearFilter(wxCommandEvent& event) {
    // Reset date pickers to current date
    wxDateTime today = wxDateTime::Now();
    fromDate->SetValue(today);
    toDate->SetValue(today);
    
    // Reset choice controls
    paymentTypeFilter->SetSelection(0);
    transactionTypeFilter->SetSelection(0);
    
    // Refresh with all transactions
    PopulateTransactionList();
    UpdateSummary();
}

void TransactionPanel::OnPaymentTypeChange(wxCommandEvent& event) {
    // Enable/disable M-Pesa code field based on payment type selection
    bool isMpesa = (paymentTypeChoice->GetSelection() == 1);
    mpesaCodeText->Enable(isMpesa);
    
    if (!isMpesa) {
        mpesaCodeText->Clear();
    }
}

void TransactionPanel::OnCustomerChange(wxCommandEvent& event) {
    // This can be used later for auto-filling transaction details based on customer
}

void TransactionPanel::OnTextEnter(wxCommandEvent& event) {
    wxObject* source = event.GetEventObject();
    
    if (source == amountText) {
        paymentTypeChoice->SetFocus();
    } else if (source == descriptionText) {
        if (paymentTypeChoice->GetSelection() == 1) { // M-Pesa
            mpesaCodeText->SetFocus();
        } else {
            // Try to save transaction
            wxCommandEvent dummyEvent;
            if (selectedTransactionId == -1) {
                OnAddTransaction(dummyEvent);
            } else {
                OnEditTransaction(dummyEvent);
            }
        }
    } else if (source == mpesaCodeText) {
        // Save transaction
        wxCommandEvent dummyEvent;
        if (selectedTransactionId == -1) {
            OnAddTransaction(dummyEvent);
        } else {
            OnEditTransaction(dummyEvent);
        }
    }
}