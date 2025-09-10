#include "CustomerPanel.h"
#include "Customer.h"
#include <wx/sizer.h>
#include <ctime>
#include <algorithm>
#include <cctype>

// Event table - maps UI events to handler methods
wxBEGIN_EVENT_TABLE(CustomerPanel, wxPanel)
    // Search events
    EVT_TEXT_ENTER(wxID_ANY, CustomerPanel::OnSearchEnter)
    EVT_BUTTON(wxID_FIND, CustomerPanel::OnSearchButton)
    EVT_BUTTON(wxID_CLEAR, CustomerPanel::OnClearSearch)
    
    // List events
    EVT_LIST_ITEM_SELECTED(wxID_ANY, CustomerPanel::OnCustomerSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, CustomerPanel::OnCustomerDeselected)
    
    // Form events
    EVT_BUTTON(wxID_ADD, CustomerPanel::OnAddNewCustomer)
    EVT_BUTTON(wxID_SAVE, CustomerPanel::OnSaveCustomer)
    EVT_TEXT_ENTER(wxID_ANY, CustomerPanel::OnFormFieldEnter)
    EVT_BUTTON(wxID_DELETE, CustomerPanel::OnDeleteCustomer)
    EVT_BUTTON(ID_RENEW_MEMBERSHIP, CustomerPanel::OnRenewMembership)
    
    // Category events
    EVT_CHOICE(wxID_ANY, CustomerPanel::OnCategoryChoice)
    EVT_CHOICE(ID_MEMBERSHIP_CHOICE, CustomerPanel::OnMembershipChoice)
wxEND_EVENT_TABLE()

// Constructor - initialize panel and create layout
CustomerPanel::CustomerPanel(wxWindow* parent) 
    : wxPanel(parent, wxID_ANY), 
      selectedCustomerId(-1), 
      isEditMode(false), 
      isNewCustomer(false),
      showPricingInfo(true),
      searchPanel(nullptr),
      searchText(nullptr),
      searchButton(nullptr),
      clearButton(nullptr),
      customerList(nullptr),
      detailPanel(nullptr),
      firstNameText(nullptr),
      lastNameText(nullptr),
      phoneText(nullptr),
      emailText(nullptr),
      membershipChoice(nullptr),
      categoryChoice(nullptr),
      statusText(nullptr),
      daysLeftText(nullptr),
      pricingText(nullptr),
      categoryText(nullptr),
      addEditButton(nullptr),
      saveButton(nullptr),
      deleteButton(nullptr),
      changeCategoryButton(nullptr) {
    
    CreateLayout();
    RefreshData();
}

// ============================================================================
// LAYOUT CREATION METHODS
// ============================================================================

void CustomerPanel::CreateLayout() {
    // Main vertical sizer - search panel on top, content below
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Create panels in order
    CreateSearchPanel();
    CreateCustomerList();
    CreateDetailPanel();

    // Add search panel to top
    mainSizer->Add(searchPanel, 0, wxEXPAND | wxALL, 8);

    // Create horizontal sizer for list and details
    wxBoxSizer* contentSizer = new wxBoxSizer(wxHORIZONTAL);
    contentSizer->Add(customerList, 2, wxEXPAND | wxALL, 5);
    contentSizer->Add(detailPanel, 1, wxEXPAND | wxALL, 5);

    // Add content to main sizer
    mainSizer->Add(contentSizer, 1, wxEXPAND);

    SetSizer(mainSizer);
    ClearDetailPanel();
}

void CustomerPanel::CreateSearchPanel() {
    // Create search panel with horizontal layout
    searchPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* searchSizer = new wxBoxSizer(wxHORIZONTAL);

    // Search label
    searchSizer->Add(new wxStaticText(searchPanel, wxID_ANY, "Search Customer:"), 
                    0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    // Search text input with Enter key processing
    searchText = new wxTextCtrl(searchPanel, wxID_ANY, "", 
                               wxDefaultPosition, wxSize(280, -1), wxTE_PROCESS_ENTER);
    searchSizer->Add(searchText, 0, wxRIGHT, 8);

    // Search button
    searchButton = new wxButton(searchPanel, wxID_FIND, "Search", 
                               wxDefaultPosition, wxSize(80, -1));
    searchSizer->Add(searchButton, 0, wxRIGHT, 8);

    // Clear search button
    clearButton = new wxButton(searchPanel, wxID_CLEAR, "Clear", 
                              wxDefaultPosition, wxSize(80, -1));
    searchSizer->Add(clearButton, 0, wxRIGHT, 15);

    // Add instruction text
    searchSizer->AddStretchSpacer(1);
    wxStaticText* instructions = new wxStaticText(searchPanel, wxID_ANY, 
                                                 "Press Enter or click Search");
    instructions->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
    searchSizer->Add(instructions, 0, wxALIGN_CENTER_VERTICAL);

    searchPanel->SetSizer(searchSizer);
}

void CustomerPanel::CreateCustomerList() {
    // Create customer list with report view and single selection
    customerList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL);
    
    // Bind mouse event for empty area click detection
    customerList->Bind(wxEVT_LEFT_DOWN, &CustomerPanel::OnListClick, this);
    
// Setup list columns with appropriate widths
    customerList->AppendColumn("ID", wxLIST_FORMAT_LEFT, 50);
    customerList->AppendColumn("First Name", wxLIST_FORMAT_LEFT, 120);
    customerList->AppendColumn("Last Name", wxLIST_FORMAT_LEFT, 120);
    customerList->AppendColumn("Phone", wxLIST_FORMAT_LEFT, 120);
    customerList->AppendColumn("Membership", wxLIST_FORMAT_LEFT, 140);
    customerList->AppendColumn("Category", wxLIST_FORMAT_LEFT, 80);
    customerList->AppendColumn("Status", wxLIST_FORMAT_LEFT, 80);
    customerList->AppendColumn("Expiry Date", wxLIST_FORMAT_LEFT, 100);
    customerList->AppendColumn("Days Left", wxLIST_FORMAT_LEFT, 80);
}

void CustomerPanel::CreateDetailPanel() {
    // Create detail panel for customer information
    detailPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* detailSizer = new wxBoxSizer(wxVERTICAL);
    
    // Panel title
    wxStaticText* title = new wxStaticText(detailPanel, wxID_ANY, "Customer Details");
    title->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    detailSizer->Add(title, 0, wxALL | wxALIGN_CENTER, 10);
    
    // Create form fields using grid layout
    wxFlexGridSizer* formSizer = new wxFlexGridSizer(8, 2, 10, 10);
    formSizer->AddGrowableCol(1);
    
    // First Name field
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "First Name:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    firstNameText = new wxTextCtrl(detailPanel, wxID_ANY, "", 
                                  wxDefaultPosition, wxDefaultSize, 
                                  wxTE_READONLY | wxTE_PROCESS_ENTER);
    formSizer->Add(firstNameText, 1, wxEXPAND);

    // Last Name field
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Last Name:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    lastNameText = new wxTextCtrl(detailPanel, wxID_ANY, "", 
                                 wxDefaultPosition, wxDefaultSize, 
                                 wxTE_READONLY | wxTE_PROCESS_ENTER);
    formSizer->Add(lastNameText, 1, wxEXPAND);

    // Phone field
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Phone:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    phoneText = new wxTextCtrl(detailPanel, wxID_ANY, "", 
                              wxDefaultPosition, wxDefaultSize, 
                              wxTE_READONLY | wxTE_PROCESS_ENTER);
    formSizer->Add(phoneText, 1, wxEXPAND);

    // Email field
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Email:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    emailText = new wxTextCtrl(detailPanel, wxID_ANY, "", 
                              wxDefaultPosition, wxDefaultSize, 
                              wxTE_READONLY | wxTE_PROCESS_ENTER);
    formSizer->Add(emailText, 1, wxEXPAND);

    // Membership Type dropdown
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Membership:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    membershipChoice = new wxChoice(detailPanel, ID_MEMBERSHIP_CHOICE);
    membershipChoice->Enable(false);
    formSizer->Add(membershipChoice, 1, wxEXPAND);
    
    // Customer Category dropdown
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Category:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    categoryChoice = new wxChoice(detailPanel, wxID_ANY);
    categoryChoice->Enable(false);
    formSizer->Add(categoryChoice, 1, wxEXPAND);
    
    // Status display
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Status:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    statusText = new wxStaticText(detailPanel, wxID_ANY, "");
    formSizer->Add(statusText, 1, wxEXPAND);
    
    // Days Left display
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Days Left:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    daysLeftText = new wxStaticText(detailPanel, wxID_ANY, "");
    formSizer->Add(daysLeftText, 1, wxEXPAND);
    
    // Pricing information display
    formSizer->Add(new wxStaticText(detailPanel, wxID_ANY, "Price:"), 
                  0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    pricingText = new wxStaticText(detailPanel, wxID_ANY, "");
    pricingText->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    formSizer->Add(pricingText, 1, wxEXPAND);
    
    detailSizer->Add(formSizer, 0, wxEXPAND | wxALL, 20);

    // Create first button row (horizontal)
    wxBoxSizer* firstButtonSizer = new wxBoxSizer(wxHORIZONTAL);

    // Add/Edit button (text changes based on selection)
    addEditButton = new wxButton(detailPanel, wxID_ADD, "Add New Customer", 
                                wxDefaultPosition, wxSize(150, 35));
    firstButtonSizer->Add(addEditButton, 0, wxRIGHT, 10);

    // Renew membership button (hidden initially)
    renewMembershipButton = new wxButton(detailPanel, ID_RENEW_MEMBERSHIP, "Renew/Change Membership", 
                                    wxDefaultPosition, wxSize(180, 35));
    renewMembershipButton->Show(false);
    firstButtonSizer->Add(renewMembershipButton, 0, wxRIGHT, 10);

    // Change category button (hidden initially)
    changeCategoryButton = new wxButton(detailPanel, wxID_ANY, "Change Category", 
                                       wxDefaultPosition, wxSize(120, 35));
    changeCategoryButton->Show(false);
    firstButtonSizer->Add(changeCategoryButton, 0, wxRIGHT, 10);

    // Save button (hidden initially)
    saveButton = new wxButton(detailPanel, wxID_SAVE, "Save Customer", 
                            wxDefaultPosition, wxSize(120, 35));
    saveButton->Show(false);
    firstButtonSizer->Add(saveButton, 0);

    // Create second button row for delete button
    wxBoxSizer* secondButtonSizer = new wxBoxSizer(wxHORIZONTAL);

    // Delete button (hidden initially) - now on its own row
    deleteButton = new wxButton(detailPanel, wxID_DELETE, "Delete Customer", 
                            wxDefaultPosition, wxSize(120, 35));
    deleteButton->Show(false);
    secondButtonSizer->Add(deleteButton, 0);

    detailSizer->Add(secondButtonSizer, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 15);

    // Add instruction text
    wxStaticText* instructions = new wxStaticText(detailPanel, wxID_ANY, 
        "Select a customer from the list to view details\nor click 'Add New Customer' to add a new one.");
    instructions->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
    detailSizer->Add(instructions, 0, wxALL | wxALIGN_CENTER, 10);
    
    detailPanel->SetSizer(detailSizer);
}

void CustomerPanel::PopulateChoices() {
    // Populate membership type choices
    membershipChoice->Clear();
    membershipChoice->Append("Daily Gym");
    membershipChoice->Append("Daily Personal Training");
    membershipChoice->Append("Daily Martial Arts");
    membershipChoice->Append("Weekly");
    membershipChoice->Append("Monthly");
    membershipChoice->Append("Trainer");
    
    // Populate customer category choices
    categoryChoice->Clear();
    categoryChoice->Append("Regular");
    categoryChoice->Append("Student");
    categoryChoice->Append("Minor");
}

// ============================================================================
// DATA MANAGEMENT METHODS
// ============================================================================

void CustomerPanel::RefreshData() {
    // Clear selection and refresh customer list from database
    ClearSelection();
    PopulateCustomerList();
}

void CustomerPanel::PopulateCustomerList() {
    // Clear existing items and load all customers from database
    customerList->DeleteAllItems();
    
    std::vector<Customer> customers = Customer::getAllCustomers();
    PopulateCustomerList(customers);
}

void CustomerPanel::PopulateCustomerList(const std::vector<Customer>& customers) {
    // Populate list with provided customers (used for search results)
    customerList->DeleteAllItems();
    
    // Sort customers by membership type priority, then alphabetically
    std::vector<Customer> sortedCustomers = customers;
    std::sort(sortedCustomers.begin(), sortedCustomers.end(), 
        [](const Customer& a, const Customer& b) {
            // Define membership type priority for consistent ordering
            auto getMembershipPriority = [](MembershipType type) -> int {
                switch (type) {
                    case MembershipType::DAILY:   return 0;
                    case MembershipType::WEEKLY:  return 1;
                    case MembershipType::MONTHLY: return 2;
                    case MembershipType::TRAINER: return 3;
                    default: return 4;
                }
            };
            
            int priorityA = getMembershipPriority(a.getMembershipType());
            int priorityB = getMembershipPriority(b.getMembershipType());
            
            // Primary sort by membership priority
            if (priorityA != priorityB) {
                return priorityA < priorityB;
            }
            
            // Secondary sort by first name
            int firstNameComp = a.getFirstName().compare(b.getFirstName());
            if (firstNameComp != 0) {
                return firstNameComp < 0;
            }
            
            // Tertiary sort by last name
            return a.getLastName().compare(b.getLastName()) < 0;
        });
    
    // Add sorted customers to list
    for (size_t i = 0; i < sortedCustomers.size(); ++i) {
        const Customer& customer = sortedCustomers[i];
        
        // Insert customer row with ID as first column
        long index = customerList->InsertItem(i, wxString::Format("%d", customer.getCustomerId()));
        
        /// Populate all columns
        customerList->SetItem(index, 1, customer.getFirstName());
        customerList->SetItem(index, 2, customer.getLastName());
        customerList->SetItem(index, 3, FormatPhoneForDisplay(customer.getPhone()));
        customerList->SetItem(index, 4, customer.getMembershipTypeString());
        customerList->SetItem(index, 5, customer.getCategoryString());
        customerList->SetItem(index, 6, customer.getStatusString());
        
        // Format expiry date for weekly/monthly memberships
        std::time_t expiryDate = customer.getExpiryDate();
        if (expiryDate > 0) {
            struct tm* timeInfo = std::localtime(&expiryDate);
            char dateStr[20];
            std::strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", timeInfo);
            customerList->SetItem(index, 7, wxString(dateStr));
        } else {
            customerList->SetItem(index, 7, " "); // No expiry for daily/trainer
        }
        
        // Display days remaining
        int daysLeft = customer.getDaysRemaining();
        if (daysLeft >= 0) {
            customerList->SetItem(index, 8, wxString::Format("%d", daysLeft));
        } else {
            customerList->SetItem(index, 8, " ");
        }
        
        // Store customer ID for selection handling
        customerList->SetItemData(index, customer.getCustomerId());
    }
}

void CustomerPanel::UpdateDetailPanel(int customerId) {
    // Load and display customer details in the form
    Customer customer;
    if (!customer.loadFromDatabase(customerId)) {
        ClearDetailPanel();
        return;
    }
    
    // Populate form fields
    firstNameText->SetValue(customer.getFirstName());
    lastNameText->SetValue(customer.getLastName());
    phoneText->SetValue(FormatPhoneForInput(customer.getPhone()));
    emailText->SetValue(customer.getEmail());
    
    // Set membership type selection
    MembershipType membershipType = customer.getMembershipType();
    int selectionIndex = static_cast<int>(membershipType);
    membershipChoice->SetSelection(selectionIndex);
    
    // Set customer category selection
    CustomerCategory customerCategory = customer.getCustomerCategory();
    int categoryIndex = static_cast<int>(customerCategory);
    categoryChoice->SetSelection(categoryIndex);
    
    int daysLeft = customer.getDaysRemaining();
    if (daysLeft >= 0) {
        daysLeftText->SetLabel(wxString::Format("%d days", daysLeft));
    } else {
        daysLeftText->SetLabel("N/A");
    }
    
    // Update pricing display
    UpdatePricingDisplay();
    
    detailPanel->Layout();
    UpdateButtonState();
    
    detailPanel->Layout();
    UpdateButtonState();
}

void CustomerPanel::ClearDetailPanel() {
    // Clear all form fields and reset to view mode
    firstNameText->Clear();
    lastNameText->Clear();
    phoneText->Clear();
    emailText->Clear();
    membershipChoice->SetSelection(wxNOT_FOUND);
    categoryChoice->SetSelection(wxNOT_FOUND);
    statusText->SetLabel("");
    daysLeftText->SetLabel("");
    pricingText->SetLabel("");
    
    // Ensure we exit edit mode
    if (isEditMode) {
        SetEditMode(false, false);
    }
    
    detailPanel->Layout();
    UpdateButtonState();
}

void CustomerPanel::ClearSelection() {
    // Clear customer selection and reset form
    selectedCustomerId = -1;
    
    // Exit edit mode if active
    if (isEditMode) {
        SetEditMode(false, false);
    }
    
    // Clear list selection
    customerList->SetItemState(-1, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    
    // Clear detail panel
    ClearDetailPanel();

    // Hide delete button when no customer is selected
    deleteButton->Show(false);
}

// ============================================================================
// SEARCH FUNCTIONALITY
// ============================================================================

void CustomerPanel::PerformSearch() {
    // Execute customer search based on name input
    wxString searchTerm = searchText->GetValue().Trim().Trim(false);
    
    if (searchTerm.IsEmpty()) {
        ClearSearch();
        return;
    }
    
    // Get all customers and filter by name
    std::vector<Customer> allCustomers = Customer::getAllCustomers();
    std::vector<Customer> filteredCustomers;
    wxString searchLower = searchTerm.Lower();
    
    for (const Customer& customer : allCustomers) {
        wxString firstName = wxString(customer.getFirstName()).Lower();
        wxString lastName = wxString(customer.getLastName()).Lower();
        wxString fullName = firstName + " " + lastName;
        
        // Check if search term matches any part of customer names
        if (firstName.Contains(searchLower) || 
            lastName.Contains(searchLower) || 
            fullName.Contains(searchLower)) {
            filteredCustomers.push_back(customer);
        }
    }
    
    // Update list with search results
    PopulateCustomerList(filteredCustomers);
    ClearSelection();
}

void CustomerPanel::ClearSearch() {
    // Reset search and show all customers
    searchText->Clear();
    PopulateCustomerList();
    ClearSelection();
    Layout();
    Refresh();
}

// ============================================================================
// FORM MANAGEMENT METHODS
// ============================================================================

void CustomerPanel::SetEditMode(bool editMode, bool newCustomer) {
    // Toggle between view and edit modes
    isEditMode = editMode;
    isNewCustomer = newCustomer;
    
    // Enable/disable form controls
    firstNameText->SetEditable(editMode);
    lastNameText->SetEditable(editMode);
    phoneText->SetEditable(editMode);
    emailText->SetEditable(editMode);
    membershipChoice->Enable(false);  // Always read-only as requested
    
    // Show/hide buttons based on mode and selection
    addEditButton->Show(!editMode);
    saveButton->Show(editMode);
    deleteButton->Show(!editMode && selectedCustomerId > 0);  // Show delete only when customer is selected and not editing
    
    detailPanel->Layout();
    detailPanel->Refresh();
}

void CustomerPanel::PrepareNewCustomerForm() {
    // Setup form for new customer entry
    ClearDetailPanel();
    membershipChoice->SetSelection(0); // Default to Daily
    SetEditMode(true, true);
    firstNameText->SetFocus();
    
    // Clear list selection
    customerList->SetItemState(-1, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
}

void CustomerPanel::UpdateButtonState() {
    // Update button text and visibility based on current state
    if (selectedCustomerId > 0 && !isEditMode) {
        addEditButton->SetLabel("Edit Customer");
        deleteButton->Show(true);
        renewMembershipButton->Show(true);  // Show renew button when customer is selected
    } else {
        addEditButton->SetLabel("Add New Customer");
        deleteButton->Show(false);
        renewMembershipButton->Show(false); // Hide renew button when no selection or editing
    }
    
    addEditButton->Refresh();
    deleteButton->Refresh();
    detailPanel->Layout();
}

bool CustomerPanel::ConfirmCustomerDeletion() {
    // Get customer details for confirmation dialog
    Customer customer;
    if (!customer.loadFromDatabase(selectedCustomerId)) {
        return false;
    }
    
    // Create confirmation message with customer details
    wxString message = wxString::Format(
        "Are you sure you want to delete this customer?\n\n"
        "Name: %s %s\n"
        "Phone: %s\n"
        "Membership: %s\n\n"
        "This action cannot be undone!",
        customer.getFirstName(),
        customer.getLastName(), 
        FormatPhoneForDisplay(customer.getPhone()),
        customer.getMembershipTypeString()
    );
    
    // Show confirmation dialog
    int result = wxMessageBox(message, "Confirm Customer Deletion", 
                             wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT);
    
    return result == wxYES;
}

bool CustomerPanel::ValidateCustomerForm() {
    // Validate form fields before saving
    wxString firstName = firstNameText->GetValue().Trim();
    wxString lastName = lastNameText->GetValue().Trim();
    wxString phone = phoneText->GetValue().Trim();
    
    // Check required fields
    if (firstName.IsEmpty()) {
        wxMessageBox("First name is required.", "Validation Error", wxOK | wxICON_WARNING);
        firstNameText->SetFocus();
        return false;
    }
    
    if (lastName.IsEmpty()) {
        wxMessageBox("Last name is required.", "Validation Error", wxOK | wxICON_WARNING);
        lastNameText->SetFocus();
        return false;
    }
    
    if (phone.IsEmpty()) {
        wxMessageBox("Phone number is required.", "Validation Error", wxOK | wxICON_WARNING);
        phoneText->SetFocus();
        return false;
    }
    
    // Validate phone format
    if (!IsValidKenyanPhone(phone.ToStdString())) {
        wxMessageBox("Please enter a valid Kenyan phone number.\nExamples: 0722123456, 0111234567", 
                    "Invalid Phone Number", wxOK | wxICON_WARNING);
        phoneText->SetFocus();
        return false;
    }
    
    // Check for duplicate phone numbers (new customers only)
    if (isNewCustomer && Customer::phoneExists(phone.ToStdString())) {
        wxMessageBox("A customer with this phone number already exists.", 
                    "Duplicate Customer", wxOK | wxICON_WARNING);
        phoneText->SetFocus();
        return false;
    }
    
    return true;
}

// ============================================================================
// PHONE NUMBER FORMATTING UTILITIES
// ============================================================================

std::string CustomerPanel::FormatPhoneForDisplay(const std::string& phone) const {
    // Format phone number for display in customer list (+254 format)
    if (phone.empty()) return phone;
    
    std::string cleanPhone = phone;
    // Remove spaces, dashes, and plus signs
    cleanPhone.erase(std::remove_if(cleanPhone.begin(), cleanPhone.end(), 
                    [](char c) { return c == ' ' || c == '-' || c == '+'; }), cleanPhone.end());
    
    // Handle different input formats
    if (cleanPhone.length() >= 9) {
        if (cleanPhone.substr(0, 3) == "254") {
            // Already has country code
            return "+254 " + cleanPhone.substr(3, 3) + " " + cleanPhone.substr(6);
        } else if (cleanPhone[0] == '0') {
            // Remove leading 0 and add country code
            return "+254 " + cleanPhone.substr(1, 3) + " " + cleanPhone.substr(4);
        } else if (cleanPhone.length() >= 9) {
            // Assume without country code or leading 0
            return "+254 " + cleanPhone.substr(0, 3) + " " + cleanPhone.substr(3);
        }
    }
    
    return phone; // Return original if can't format
}

std::string CustomerPanel::FormatPhoneForInput(const std::string& phone) const {
    // Format phone number for input/editing (0xxx format)
    if (phone.empty()) return phone;
    
    std::string cleanPhone = phone;
    // Remove spaces and dashes but keep original prefix format
    cleanPhone.erase(std::remove_if(cleanPhone.begin(), cleanPhone.end(), 
                    [](char c) { return c == ' ' || c == '-'; }), cleanPhone.end());
    
    // Convert +254 format back to 0xxx format for editing
    if (cleanPhone.substr(0, 4) == "+254") {
        return "0" + cleanPhone.substr(4);
    } else if (cleanPhone.substr(0, 3) == "254" && cleanPhone.length() > 9) {
        return "0" + cleanPhone.substr(3);
    }
    
    return cleanPhone; // Return as-is if already in 0xxx format
}

std::string CustomerPanel::NormalizePhoneNumber(const std::string& phone) const {
    // Normalize phone number for consistent database storage
    if (phone.empty()) return phone;
    
    std::string cleanPhone = phone;
    // Remove all non-digit characters except +
    cleanPhone.erase(std::remove_if(cleanPhone.begin(), cleanPhone.end(), 
                    [](char c) { return !std::isdigit(c) && c != '+'; }), cleanPhone.end());
    
    // Convert to consistent +254xxxxxxx format for storage
    if (cleanPhone.substr(0, 4) == "+254" && cleanPhone.length() == 13) {
        return cleanPhone; // Already in correct format
    } else if (cleanPhone.substr(0, 3) == "254" && cleanPhone.length() == 12) {
        return "+" + cleanPhone; // Add + to 254xxxxxxx
    } else if (cleanPhone[0] == '0' && cleanPhone.length() == 10) {
        return "+254" + cleanPhone.substr(1); // Convert 0xxxxxxx to +254xxxxxxx
    } else if (cleanPhone.length() == 9 && (cleanPhone[0] == '7' || cleanPhone[0] == '1')) {
        return "+254" + cleanPhone; // Add +254 to 9-digit number
    }
    
    return phone; // Return original if can't normalize
}

bool CustomerPanel::IsValidKenyanPhone(const std::string& phone) const {
    // Validate Kenyan phone number format
    if (phone.empty()) return false;
    
    std::string normalized = NormalizePhoneNumber(phone);
    
    // Should be 13 characters total (+254xxxxxxxxx)
    if (normalized.length() != 13) return false;
    
    // Should start with +254
    if (normalized.substr(0, 4) != "+254") return false;
    
    // Fifth character should be 7 (mobile) or 1 (landline)
    char fifthChar = normalized[4];
    if (fifthChar != '7' && fifthChar != '1') return false;
    
    // Remaining characters should be digits
    for (size_t i = 5; i < normalized.length(); ++i) {
        if (!std::isdigit(normalized[i])) return false;
    }
    
    return true;
}

// ============================================================================
// MEMBERSHIP MANAGEMENT METHODS
// ============================================================================

void CustomerPanel::ShowMembershipRenewalDialog(int customerId) {
    // Load customer data to show current membership details
    Customer customer;
    if (!customer.loadFromDatabase(customerId)) {
        wxMessageBox("Failed to load customer data.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    // Create renewal dialog
    wxDialog* renewalDialog = new wxDialog(this, wxID_ANY, "Renew/Change Membership", 
                                          wxDefaultPosition, wxSize(400, 300));
    
    // Main sizer for dialog
    wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
    
    // Customer info display
    wxStaticText* customerInfo = new wxStaticText(renewalDialog, wxID_ANY,
        wxString::Format("Customer: %s %s\nCurrent: %s", 
                        customer.getFirstName(), customer.getLastName(),
                        customer.getMembershipTypeString()));
    customerInfo->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dialogSizer->Add(customerInfo, 0, wxALL | wxALIGN_CENTER, 15);
    
    // Membership selection
    dialogSizer->Add(new wxStaticText(renewalDialog, wxID_ANY, "Select New Membership Type:"), 
                    0, wxLEFT | wxRIGHT, 20);
    
    wxChoice* membershipChoiceDialog = new wxChoice(renewalDialog, wxID_ANY);
    membershipChoiceDialog->Append("Daily");
    membershipChoiceDialog->Append("Weekly"); 
    membershipChoiceDialog->Append("Monthly");
    membershipChoiceDialog->Append("Trainer");
    membershipChoiceDialog->SetSelection(static_cast<int>(customer.getMembershipType()));
    dialogSizer->Add(membershipChoiceDialog, 0, wxEXPAND | wxALL, 20);
    
    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* confirmButton = new wxButton(renewalDialog, wxID_OK, "Confirm Renewal");
    wxButton* cancelButton = new wxButton(renewalDialog, wxID_CANCEL, "Cancel");
    buttonSizer->Add(confirmButton, 0, wxRIGHT, 10);
    buttonSizer->Add(cancelButton, 0);
    dialogSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 20);
    
    renewalDialog->SetSizer(dialogSizer);
    
    // Show dialog and process result
    if (renewalDialog->ShowModal() == wxID_OK) {
        int selectedType = membershipChoiceDialog->GetSelection();
        MembershipType newMembershipType = static_cast<MembershipType>(selectedType);
        
        if (ProcessMembershipRenewal(customerId, newMembershipType)) {
            wxMessageBox("Membership renewed successfully!", "Success", wxOK | wxICON_INFORMATION);
            RefreshData();
            
            // Reselect the customer to show updated details
            for (long i = 0; i < customerList->GetItemCount(); i++) {
                if (customerList->GetItemData(i) == customerId) {
                    customerList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                    break;
                }
            }
        }
    }
    
    renewalDialog->Destroy();
}

bool CustomerPanel::ProcessMembershipRenewal(int customerId, MembershipType newType) {
    // Load customer and update membership type
    Customer customer;
    if (!customer.loadFromDatabase(customerId)) {
        wxMessageBox("Failed to load customer data.", "Error", wxOK | wxICON_ERROR);
        return false;
    }
    
    // Update membership type and calculate new expiry date
    customer.setMembershipType(newType);
    customer.calculateExpiryDate();  // This method should exist in Customer class
    
    // Update in database
    if (customer.updateInDatabase()) {
        // Create transaction record for the renewal
        double fee = 0.0;
        switch (newType) {
            case MembershipType::DAILY:   fee = AppConfig::DEFAULT_DAILY_FEE; break;
            case MembershipType::WEEKLY:  fee = AppConfig::DEFAULT_WEEKLY_FEE; break;
            case MembershipType::MONTHLY: fee = AppConfig::DEFAULT_MONTHLY_FEE; break;
            case MembershipType::TRAINER: fee = AppConfig::DEFAULT_TRAINER_FEE; break;
        }
        
        // Record the renewal transaction
        customer.recordPayment(fee, PaymentType::CASH, TransactionType::RENEWAL, 
                             "Membership renewal to " + MembershipTypeHelper::toString(newType));
        
        return true;
    }
    
    wxMessageBox("Failed to update membership in database.", "Error", wxOK | wxICON_ERROR);
    return false;
}

// ============================================================================
// EVENT HANDLERS - SEARCH FUNCTIONALITY
// ============================================================================

void CustomerPanel::OnSearchButton(wxCommandEvent& event) {
    // Handle search button click
    PerformSearch();
}

void CustomerPanel::OnSearchEnter(wxCommandEvent& event) {
    // Handle Enter key in search field
    wxObject* eventObj = event.GetEventObject();
    if (eventObj == searchText) {
        PerformSearch();
    } else {
        // If Enter was pressed in a form field, handle form navigation
        OnFormFieldEnter(event);
    }
}

void CustomerPanel::OnClearSearch(wxCommandEvent& event) {
    // Handle clear search button click
    ClearSearch();
}

// ============================================================================
// EVENT HANDLERS - CUSTOMER LIST
// ============================================================================

void CustomerPanel::OnCustomerSelected(wxListEvent& event) {
    // Handle customer selection in list
    selectedCustomerId = static_cast<int>(event.GetData());
    UpdateDetailPanel(selectedCustomerId);
}

void CustomerPanel::OnCustomerDeselected(wxListEvent& event) {
    // Handle customer deselection
    selectedCustomerId = -1;
    ClearDetailPanel();
}

void CustomerPanel::OnListClick(wxMouseEvent& event) {
    // Handle mouse click on list (detect empty area clicks)
    int flags;
    long item = customerList->HitTest(event.GetPosition(), flags);
    
    if (item == wxNOT_FOUND) {
        // Clicked on empty area - clear selection
        ClearSelection();
    }
    
    event.Skip(); // Allow normal processing
}

// ============================================================================
// EVENT HANDLERS - FORM MANAGEMENT
// ============================================================================

void CustomerPanel::OnAddNewCustomer(wxCommandEvent& event) {
    // Handle Add New/Edit Customer button click
    if (addEditButton->GetLabel() == "Edit Customer") {
        // Edit existing customer
        if (selectedCustomerId > 0) {
            SetEditMode(true, false);
            firstNameText->SetFocus();
        }
    } else {
        // Add new customer
        PrepareNewCustomerForm();
    }
}

void CustomerPanel::OnFormFieldEnter(wxCommandEvent& event) {
    // Handle Enter key in form fields for navigation
    if (!isEditMode) return;
    
    wxTextCtrl* currentField = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if (!currentField) return;
    
    // Navigate to next field or save if on last field
    if (currentField == firstNameText) {
        lastNameText->SetFocus();
    } else if (currentField == lastNameText) {
        phoneText->SetFocus();
    } else if (currentField == phoneText) {
        emailText->SetFocus();
    } else if (currentField == emailText) {
        membershipChoice->SetFocus();
    } else {
        // Save customer if Enter pressed on last field
        wxCommandEvent saveEvent(wxEVT_COMMAND_BUTTON_CLICKED, wxID_SAVE);
        ProcessEvent(saveEvent);
    }
}

void CustomerPanel::OnSaveCustomer(wxCommandEvent& event) {
    // Handle Save Customer button click
    if (!ValidateCustomerForm()) {
        return;
    }
    
    // Extract form data
    std::string firstName = firstNameText->GetValue().Trim().ToStdString();
    std::string lastName = lastNameText->GetValue().Trim().ToStdString();
    std::string phoneInput = phoneText->GetValue().Trim().ToStdString();
    std::string phone = NormalizePhoneNumber(phoneInput);
    std::string email = emailText->GetValue().Trim().ToStdString();
    
    // Get membership type from dropdown
    int membershipIndex = membershipChoice->GetSelection();
    MembershipType membershipType = static_cast<MembershipType>(membershipIndex);
    
    if (isNewCustomer) {
        // Create new customer
        Customer newCustomer(firstName, lastName, phone, membershipType);
        newCustomer.setEmail(email);
        
        if (newCustomer.saveToDatabase()) {
            wxMessageBox("Customer added successfully!", "Success", wxOK | wxICON_INFORMATION);
            
            // Exit edit mode and refresh
            SetEditMode(false, false);
            RefreshData();
            
            // Select newly created customer
            int newCustomerId = newCustomer.getCustomerId();
            for (long i = 0; i < customerList->GetItemCount(); i++) {
                if (customerList->GetItemData(i) == newCustomerId) {
                    customerList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                    customerList->EnsureVisible(i);
                    break;
                }
            }
        } else {
            wxMessageBox("Failed to save customer to database.", "Error", wxOK | wxICON_ERROR);
        }
    } else {
        // Update existing customer
        Customer existingCustomer;
        if (existingCustomer.loadFromDatabase(selectedCustomerId)) {
            existingCustomer.setFirstName(firstName);
            existingCustomer.setLastName(lastName);
            existingCustomer.setPhone(phone);
            existingCustomer.setEmail(email);
            existingCustomer.setMembershipType(membershipType);
            
            if (existingCustomer.updateInDatabase()) {
                wxMessageBox("Customer updated successfully!", "Success", wxOK | wxICON_INFORMATION);
                
                // Exit edit mode and refresh
                SetEditMode(false, false);
                RefreshData();
                
                // Reselect updated customer
                for (long i = 0; i < customerList->GetItemCount(); i++) {
                    if (customerList->GetItemData(i) == selectedCustomerId) {
                        customerList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                        customerList->EnsureVisible(i);
                        break;
                    }
                }
            } else {
                wxMessageBox("Failed to update customer in database.", "Error", wxOK | wxICON_ERROR);
            }
        } else {
            wxMessageBox("Failed to load customer data.", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void CustomerPanel::OnDeleteCustomer(wxCommandEvent& event) {
    // Verify we have a selected customer
    if (selectedCustomerId <= 0) {
        wxMessageBox("Please select a customer to delete.", "No Customer Selected", 
                    wxOK | wxICON_WARNING);
        return;
    }
    
    // Exit edit mode if active
    if (isEditMode) {
        SetEditMode(false, false);
    }
    
    // Confirm deletion with user
    if (!ConfirmCustomerDeletion()) {
        return; // User cancelled deletion
    }
    
    // Load customer to get details for logging
    Customer customerToDelete;
    if (!customerToDelete.loadFromDatabase(selectedCustomerId)) {
        wxMessageBox("Failed to load customer data for deletion.", "Error", 
                    wxOK | wxICON_ERROR);
        return;
    }
    
    // Attempt to delete customer from database
    if (customerToDelete.deleteFromDatabase()) {
        wxMessageBox(
            wxString::Format("Customer '%s %s' has been successfully deleted.", 
                           customerToDelete.getFirstName(), 
                           customerToDelete.getLastName()),
            "Customer Deleted", wxOK | wxICON_INFORMATION);
        
        // Clear selection and refresh data
        ClearSelection();
        RefreshData();
        
    } else {
    wxMessageBox("Failed to delete customer from database.\n\n"
                "Possible reasons:\n"
                "• Database connection issue\n"
                "• Customer has linked transaction records\n"
                "• Insufficient permissions\n\n"
                "Please check the console for detailed error information.", 
                "Deletion Failed", wxOK | wxICON_ERROR);
    }
}

void CustomerPanel::OnRenewMembership(wxCommandEvent& event) {
    // Handle renew membership button click
    if (selectedCustomerId > 0) {
        ShowMembershipRenewalDialog(selectedCustomerId);
    } else {
        wxMessageBox("Please select a customer to renew membership.", 
                    "No Customer Selected", wxOK | wxICON_WARNING);
    }
}
