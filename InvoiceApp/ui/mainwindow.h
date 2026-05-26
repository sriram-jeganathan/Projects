#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QLineEdit;
class QTableWidget;
class QTableView;
class QPushButton;
class QDateEdit;
class QSortFilterProxyModel;
class QStandardItemModel;
class InvoiceItemModel;

struct Invoice;
struct InvoiceItem;

#ifdef HAVE_QTPDF
class QPdfDocument;
class QPdfView;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void refreshVendors();
    void refreshInvoiceHistory();
    void updateVendorSelection();
    void saveVendor();
    void deleteVendor();
    void clearVendorForm();
    void addInvoiceItem();
    void removeInvoiceItem();
    void saveInvoice();
    void exportInvoice();
    void saveAndExportInvoice();
    void updateInvoiceTotals();
    void refreshVendorCompleter();
    void filterVendors(const QString& filter);
    void filterInvoices(const QString& filter);
    void saveSettings();
    void toggleDarkMode(bool checked);

private:
    QWidget* createDashboardTab();
    QWidget* createVendorTab();
    QWidget* createInvoiceTab();
    QWidget* createHistoryTab();
    QWidget* createSettingsTab();

    void createActions();
    void loadSettings();
    void loadVendorForm(const QVariant& vendorId);
    void resetInvoiceForm();
    bool buildInvoiceFromForm(Invoice& invoice, QVector<InvoiceItem>& items, QString* errorOut);

    QLabel* m_dashboardLabel = nullptr;
    QTableWidget* m_vendorTable = nullptr;
    QLineEdit* m_vendorFilter = nullptr;
    QLineEdit* m_vendorName = nullptr;
    QLineEdit* m_vendorGst = nullptr;
    QLineEdit* m_vendorPhone = nullptr;
    QLineEdit* m_vendorEmail = nullptr;
    QLineEdit* m_vendorAddress = nullptr;
    QLineEdit* m_vendorTaxDetails = nullptr;
    QPushButton* m_saveVendorButton = nullptr;
    QPushButton* m_deleteVendorButton = nullptr;
    QPushButton* m_clearVendorButton = nullptr;
    int m_currentVendorId = -1;

    QLineEdit* m_invoiceNo = nullptr;
    QLineEdit* m_invoiceVendor = nullptr;
    QDateEdit* m_invoiceDate = nullptr;
    QDateEdit* m_invoiceDueDate = nullptr;
    QTableView* m_invoiceItemsView = nullptr;
    InvoiceItemModel* m_invoiceItemModel = nullptr;
    QLabel* m_invoiceSubtotal = nullptr;
    QLabel* m_invoiceGst = nullptr;
    QLabel* m_invoiceTotal = nullptr;
    QLineEdit* m_invoiceNotes = nullptr;
    QPushButton* m_addInvoiceItemButton = nullptr;
    QPushButton* m_removeInvoiceItemButton = nullptr;
    QPushButton* m_saveInvoiceButton = nullptr;
    QPushButton* m_saveAndExportInvoiceButton = nullptr;
    QPushButton* m_exportInvoiceButton = nullptr;

    int m_currentInvoiceId = -1;

    QTableView* m_invoiceHistoryView = nullptr;
    QStandardItemModel* m_invoiceHistoryModel = nullptr;
    QSortFilterProxyModel* m_invoiceHistoryProxy = nullptr;
    QLineEdit* m_invoiceFilter = nullptr;

    QLineEdit* m_companyName = nullptr;
    QLineEdit* m_companyGst = nullptr;
    QLineEdit* m_companyAddress = nullptr;
    QLineEdit* m_companyPhone = nullptr;
    QLineEdit* m_companyEmail = nullptr;
    QPushButton* m_saveSettingsButton = nullptr;
    QPushButton* m_clearSettingsButton = nullptr;
    QPushButton* m_darkModeButton = nullptr;
#ifdef HAVE_QTPDF
    QPdfDocument* m_pdfDocument = nullptr;
    QPdfView* m_pdfView = nullptr;
#endif
    void showPdfPreview(const QString& filePath);
};

#endif // MAINWINDOW_H
