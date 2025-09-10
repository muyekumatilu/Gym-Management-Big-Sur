#ifndef QUICK_CHECKIN_DIALOG_H
#define QUICK_CHECKIN_DIALOG_H

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include "AppConstants.h"
#include "Customer.h"

class QuickCheckInDialog : public wxDialog {
public:
    QuickCheckInDialog(wxWindow* parent);

private:
    void CreateLayout();
    void PopulateCustomerList();
    void PopulateCustomerList(const std::vector<Customer>& customers);
    void UpdateButtonStates();
    bool ProcessCheckIn();

    // Event handlers
    void OnSearch(wxCommandEvent& event);
    void OnCustomerSelected(wxListEvent& event);
    void OnCustomerDoubleClick(wxListEvent& event);
    void OnCheckIn(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnPaymentTypeChange(wxCommandEvent& event);

    // UI components
    wxTextCtrl* searchText;
    wxListCtrl* customerList;
    wxChoice* paymentTypeChoice;
    wxTextCtrl* mpesaCodeText;
    wxButton* checkInBtn;
    wxButton* cancelBtn;
    
    int selectedCustomerId;

    wxDECLARE_EVENT_TABLE();
};

#endif // QUICK_CHECKIN_DIALOG_H