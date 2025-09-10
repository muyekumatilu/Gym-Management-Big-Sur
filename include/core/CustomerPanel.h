#ifndef CUSTOMER_PANEL_H
#define CUSTOMER_PANEL_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include "AppConstants.h"
#include "Customer.h"

/**
 * CustomerPanel class manages the customer interface in the gym management system
 * Provides functionality for viewing, searching, adding, and editing customers
 * Layout: Search panel (top) -> Customer list (left) -> Details panel (right)
 */
class CustomerPanel : public wxPanel {
public:
    // Constructor
    CustomerPanel(wxWindow* parent);

    // Public interface methods
    void RefreshData();                       // Refresh customer list from database
    void ClearSelection();                    // Clear current selection and form

private:
    // Layout creation methods
    void CreateLayout();                      // Create the main panel layout
    void CreateSearchPanel();                 // Create search functionality panel
    void CreateCustomerList();               // Create customer list control
    void CreateDetailPanel();                // Create customer details panel

    // Data population methods
    void PopulateCustomerList();                                    // Load all customers
    void PopulateCustomerList(const std::vector<Customer>& customers); // Load filtered customers
    void UpdateDetailPanel(int customerId);                        // Show customer details
    void PopulateChoices();                                        // Populate dropdown choices with new options
    void UpdateCategoryChoice();                                   // Update category dropdown based on membership
    void ClearDetailPanel();                                       // Clear detail form

    // Search functionality
    void PerformSearch();                     // Execute customer search
    void ClearSearch();                       // Clear search and show all customers

    // Form management methods
    void SetEditMode(bool editMode, bool newCustomer = false);     // Toggle edit/view mode
    void UpdatePricingDisplay();                                   // Update pricing information display
    void PrepareNewCustomerForm();                                 // Setup form for new customer
    bool ValidateCustomerForm();                                   // Validate form data
    void UpdateButtonState();                                      // Update button labels

    // Membership management methods
    void ShowMembershipRenewalDialog(int customerId);     // Show renewal dialog
    void ShowCategorySelectionDialog(int customerId);             // Show category selection dialog
    bool ProcessMembershipRenewal(int customerId, MembershipType newType, CustomerCategory newCategory); // Process renewal with category
    bool ValidateMembershipCategoryCombo();                        // Validate membership-category combination

    //Delete functionality
    bool ConfirmCustomerDeletion();                             //confirm customer deletion with the user

    // Phone number formatting utilities (Kenyan format)
    std::string FormatPhoneForDisplay(const std::string& phone) const;    // +254 format for list
    std::string FormatPhoneForInput(const std::string& phone) const;      // 07xx format for input
    std::string NormalizePhoneNumber(const std::string& phone) const;     // Normalize for database
    bool IsValidKenyanPhone(const std::string& phone) const;              // Validate Kenyan phone

    // Pricing and category utilities
    std::string FormatPrice(double price) const;                  // Format price for display (KES format)
    std::string GetPricingInfo(MembershipType type, CustomerCategory category) const; // Get pricing information string
    bool CategoryHasDiscount(CustomerCategory category, MembershipType type) const;   // Check if category gets discount for type

    // Event handlers - search functionality
    void OnSearchButton(wxCommandEvent& event);      // Search button clicked
    void OnSearchEnter(wxCommandEvent& event);       // Enter key in search field
    void OnClearSearch(wxCommandEvent& event);       // Clear search button clicked

    // Event handlers - customer list
    void OnCustomerSelected(wxListEvent& event);     // Customer selected in list
    void OnCustomerDeselected(wxListEvent& event);   // Customer deselected
    void OnListClick(wxMouseEvent& event);           // Mouse click on list (for empty area detection)

    // Event handlers - form management
    void OnAddNewCustomer(wxCommandEvent& event);    // Add/Edit customer button clicked
    void OnFormFieldEnter(wxCommandEvent& event);    // Enter key in form fields
    void OnSaveCustomer(wxCommandEvent& event);      // Save customer button clicked
    void OnDeleteCustomer(wxCommandEvent& event);    // Delete customer button clicked
    void OnRenewMembership(wxCommandEvent& event);       // Renew membership button clicked

    // Event handlers - category management
    void OnCategoryChoice(wxCommandEvent& event);        // Category dropdown selection changed
    void OnMembershipChoice(wxCommandEvent& event);      // Membership dropdown selection changed

    // UI component pointers - search panel
    wxPanel* searchPanel;                     // Top panel containing search controls
    wxTextCtrl* searchText;                   // Search input field
    wxButton* searchButton;                   // Search execution button
    wxButton* clearButton;                    // Clear search button
    wxButton* deleteButton;                   // Delete customer button

    // UI component pointers - customer list
    wxListCtrl* customerList;                 // Left side customer list control

    // UI component pointers - detail panel
    wxPanel* detailPanel;                     // Right side details panel
    wxTextCtrl* firstNameText;                // First name input field
    wxTextCtrl* lastNameText;                 // Last name input field
    wxTextCtrl* phoneText;                    // Phone number input field
    wxTextCtrl* emailText;                    // Email input field
    wxChoice* membershipChoice;               // Membership type dropdown
    wxChoice* categoryChoice;                 // Customer category dropdown
    wxStaticText* statusText;                 // Customer status display
    wxStaticText* pricingText;                // Pricing information display
    wxStaticText* categoryText;               // Customer category display (read-only)
    wxStaticText* daysLeftText;               // Days remaining display
    wxButton* renewMembershipButton;          // Renew/Change membership button
    wxButton* changeCategoryButton;           // Change customer category button

    // UI component pointers - buttons
    wxButton* addEditButton;                  // Add new/Edit customer button
    wxButton* saveButton;                     // Save customer button

    // State management variables
    int selectedCustomerId;                   // Currently selected customer ID (-1 if none)
    bool isEditMode;                          // True when in add/edit mode
    bool isNewCustomer;                       // True when adding new customer (not editing)
    bool showPricingInfo;                     // True when pricing information should be displayed

    // Event table declaration
    wxDECLARE_EVENT_TABLE();
};

#endif // CUSTOMER_PANEL_H