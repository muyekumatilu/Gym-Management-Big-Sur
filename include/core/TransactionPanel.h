#ifndef TRANSACTION_PANEL_H
#define TRANSACTION_PANEL_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/datectrl.h>
#include "AppConstants.h"
#include "Transaction.h"

class TransactionPanel : public wxPanel {
public:
    TransactionPanel(wxWindow* parent);
    
    void RefreshData();
    void SetTransactionTypeToDaily();
    void SelectTransaction(int transactionId);
    void AddTransactionForCustomer(int customerId, double amount = 0.0);

private:
    void CreateLayout();
    void PopulateTransactionList();
    void PopulateTransactionList(const std::vector<Transaction>& transactions);
    void PopulateCustomerChoice();
    void UpdateTransactionForm(const Transaction& transaction);
    void ClearTransactionForm();
    bool ValidateTransactionForm();
    Transaction GetTransactionFromForm();
    void UpdateSummary();
    void UpdateButtonStates();

    // Event handlers
    void OnTransactionSelected(wxListEvent& event);
    void OnAddTransaction(wxCommandEvent& event);
    void OnEditTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnRefundTransaction(wxCommandEvent& event);
    void OnPrintReceipt(wxCommandEvent& event);
    void OnFilterTransactions(wxCommandEvent& event);
    void OnClearFilter(wxCommandEvent& event);
    void OnPaymentTypeChange(wxCommandEvent& event);
    void OnCustomerChange(wxCommandEvent& event);
    void OnTextEnter(wxCommandEvent& event);

    // UI components
    wxListCtrl* transactionList;
    wxDatePickerCtrl* fromDate;
    wxDatePickerCtrl* toDate;
    wxChoice* paymentTypeFilter;
    wxChoice* transactionTypeFilter;
    wxChoice* customerChoice;
    wxTextCtrl* amountText;
    wxChoice* paymentTypeChoice;
    wxChoice* transactionTypeChoice;
    wxTextCtrl* descriptionText;
    wxTextCtrl* mpesaCodeText;
    wxButton* addTransactionBtn;
    wxButton* editTransactionBtn;
    wxButton* deleteTransactionBtn;
    wxButton* refundBtn;
    wxButton* printReceiptBtn;
    wxStaticText* totalAmountText;
    wxStaticText* cashAmountText;
    wxStaticText* mpesaAmountText;
    wxStaticText* transactionCountText;
    
    int selectedTransactionId;

    wxDECLARE_EVENT_TABLE();
};

#endif // TRANSACTION_PANEL_H