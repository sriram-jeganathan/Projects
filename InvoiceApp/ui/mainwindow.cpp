#include "ui/mainwindow.h"
#include "models/invoiceitemmodel.h"
#include "services/vendorservice.h"
#include "services/invoiceservice.h"
#include "services/settingsservice.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QTableView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDateEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QCompleter>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QApplication>
#include <QTextStream>
#include <QRegularExpression>
#include <QStandardItem>
#ifdef HAVE_QTPDF
#include <QPdfDocument>
#include <QPdfView>
#endif
#include <QDialog>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("InvoiceApp");
    resize(1024, 720);

    auto tabs = new QTabWidget(this);
    tabs->addTab(createDashboardTab(), tr("Dashboard"));
    tabs->addTab(createVendorTab(), tr("Vendors"));
    tabs->addTab(createInvoiceTab(), tr("Invoices"));
    tabs->addTab(createHistoryTab(), tr("History"));
    tabs->addTab(createSettingsTab(), tr("Settings"));

    setCentralWidget(tabs);

    loadSettings();
    refreshVendors();
    refreshInvoiceHistory();
    refreshVendorCompleter();
    resetInvoiceForm();
}

QWidget* MainWindow::createDashboardTab()
{
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    m_dashboardLabel = new QLabel(tr("Welcome to InvoiceApp. Use the tabs to manage vendors, create invoices, and export records."), widget);
    m_dashboardLabel->setWordWrap(true);
    layout->addWidget(m_dashboardLabel);
    layout->addStretch();
    return widget;
}

QWidget* MainWindow::createVendorTab()
{
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    auto searchLayout = new QHBoxLayout;
    m_vendorFilter = new QLineEdit(widget);
    m_vendorFilter->setPlaceholderText(tr("Search vendors by name, GST, phone or email"));
    connect(m_vendorFilter, &QLineEdit::textChanged, this, &MainWindow::filterVendors);
    searchLayout->addWidget(m_vendorFilter);
    layout->addLayout(searchLayout);

    m_vendorTable = new QTableWidget(widget);
    m_vendorTable->setColumnCount(6);
    m_vendorTable->setHorizontalHeaderLabels({tr("Name"), tr("GST"), tr("Phone"), tr("Email"), tr("Address"), tr("Tax details")});
    m_vendorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_vendorTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_vendorTable->horizontalHeader()->setStretchLastSection(true);
    m_vendorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_vendorTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::updateVendorSelection);
    layout->addWidget(m_vendorTable);

    auto formLayout = new QFormLayout;
    m_vendorName = new QLineEdit(widget);
    m_vendorGst = new QLineEdit(widget);
    m_vendorPhone = new QLineEdit(widget);
    m_vendorEmail = new QLineEdit(widget);
    m_vendorAddress = new QLineEdit(widget);
    m_vendorTaxDetails = new QLineEdit(widget);
    formLayout->addRow(tr("Name:"), m_vendorName);
    formLayout->addRow(tr("GST:"), m_vendorGst);
    formLayout->addRow(tr("Phone:"), m_vendorPhone);
    formLayout->addRow(tr("Email:"), m_vendorEmail);
    formLayout->addRow(tr("Address:"), m_vendorAddress);
    formLayout->addRow(tr("Tax details:"), m_vendorTaxDetails);
    layout->addLayout(formLayout);

    auto buttonsLayout = new QHBoxLayout;
    m_saveVendorButton = new QPushButton(tr("Save vendor"), widget);
    m_deleteVendorButton = new QPushButton(tr("Delete vendor"), widget);
    m_clearVendorButton = new QPushButton(tr("Clear form"), widget);
    connect(m_saveVendorButton, &QPushButton::clicked, this, &MainWindow::saveVendor);
    connect(m_deleteVendorButton, &QPushButton::clicked, this, &MainWindow::deleteVendor);
    connect(m_clearVendorButton, &QPushButton::clicked, this, &MainWindow::clearVendorForm);
    buttonsLayout->addWidget(m_saveVendorButton);
    buttonsLayout->addWidget(m_deleteVendorButton);
    buttonsLayout->addWidget(m_clearVendorButton);
    layout->addLayout(buttonsLayout);

    return widget;
}

QWidget* MainWindow::createInvoiceTab()
{
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    auto headerLayout = new QFormLayout;
    m_invoiceNo = new QLineEdit(widget);
    m_invoiceVendor = new QLineEdit(widget);
    m_invoiceDate = new QDateEdit(QDate::currentDate(), widget);
    m_invoiceDate->setCalendarPopup(true);
    m_invoiceDueDate = new QDateEdit(QDate::currentDate().addDays(15), widget);
    m_invoiceDueDate->setCalendarPopup(true);
    m_invoiceNotes = new QLineEdit(widget);
    headerLayout->addRow(tr("Invoice #:"), m_invoiceNo);
    headerLayout->addRow(tr("Customer:"), m_invoiceVendor);
    headerLayout->addRow(tr("Invoice date:"), m_invoiceDate);
    headerLayout->addRow(tr("Due date:"), m_invoiceDueDate);
    headerLayout->addRow(tr("Notes:"), m_invoiceNotes);
    layout->addLayout(headerLayout);

    m_invoiceItemModel = new InvoiceItemModel(this);
    m_invoiceItemsView = new QTableView(widget);
    m_invoiceItemsView->setModel(m_invoiceItemModel);
    m_invoiceItemsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_invoiceItemsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_invoiceItemsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_invoiceItemsView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    layout->addWidget(m_invoiceItemsView);

    auto invoiceButtons = new QHBoxLayout;
    m_addInvoiceItemButton = new QPushButton(tr("Add item"), widget);
    m_removeInvoiceItemButton = new QPushButton(tr("Remove item"), widget);
    invoiceButtons->addWidget(m_addInvoiceItemButton);
    invoiceButtons->addWidget(m_removeInvoiceItemButton);
    layout->addLayout(invoiceButtons);

    connect(m_addInvoiceItemButton, &QPushButton::clicked, this, &MainWindow::addInvoiceItem);
    connect(m_removeInvoiceItemButton, &QPushButton::clicked, this, &MainWindow::removeInvoiceItem);
    connect(m_invoiceItemModel, &QAbstractItemModel::dataChanged, this, &MainWindow::updateInvoiceTotals);
    connect(m_invoiceItemModel, &QAbstractItemModel::rowsInserted, this, &MainWindow::updateInvoiceTotals);
    connect(m_invoiceItemModel, &QAbstractItemModel::rowsRemoved, this, &MainWindow::updateInvoiceTotals);

    auto totalsLayout = new QFormLayout;
    m_invoiceSubtotal = new QLabel(tr("0.00"), widget);
    m_invoiceGst = new QLabel(tr("0.00"), widget);
    m_invoiceTotal = new QLabel(tr("0.00"), widget);
    totalsLayout->addRow(tr("Subtotal:"), m_invoiceSubtotal);
    totalsLayout->addRow(tr("GST:"), m_invoiceGst);
    totalsLayout->addRow(tr("Grand total:"), m_invoiceTotal);
    layout->addLayout(totalsLayout);

    auto actionLayout = new QHBoxLayout;
    m_saveInvoiceButton = new QPushButton(tr("Save invoice"), widget);
    m_saveAndExportInvoiceButton = new QPushButton(tr("Save && Export"), widget);
    m_exportInvoiceButton = new QPushButton(tr("Export PDF"), widget);
    connect(m_saveInvoiceButton, &QPushButton::clicked, this, &MainWindow::saveInvoice);
    connect(m_saveAndExportInvoiceButton, &QPushButton::clicked, this, &MainWindow::saveAndExportInvoice);
    connect(m_exportInvoiceButton, &QPushButton::clicked, this, &MainWindow::exportInvoice);
    actionLayout->addWidget(m_saveInvoiceButton);
    actionLayout->addWidget(m_saveAndExportInvoiceButton);
    actionLayout->addWidget(m_exportInvoiceButton);
    layout->addLayout(actionLayout);

    return widget;
}

QWidget* MainWindow::createHistoryTab()
{
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    m_invoiceFilter = new QLineEdit(widget);
    m_invoiceFilter->setPlaceholderText(tr("Filter by invoice number or customer"));
    connect(m_invoiceFilter, &QLineEdit::textChanged, this, &MainWindow::filterInvoices);
    layout->addWidget(m_invoiceFilter);

    m_invoiceHistoryModel = new QStandardItemModel(0, 7, this);
    m_invoiceHistoryModel->setHorizontalHeaderLabels({tr("Invoice #"), tr("Date"), tr("Due"), tr("Customer"), tr("Subtotal"), tr("GST"), tr("Total")});

    m_invoiceHistoryProxy = new QSortFilterProxyModel(this);
    m_invoiceHistoryProxy->setSourceModel(m_invoiceHistoryModel);
    m_invoiceHistoryProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_invoiceHistoryProxy->setFilterKeyColumn(-1);

    m_invoiceHistoryView = new QTableView(widget);
    m_invoiceHistoryView->setModel(m_invoiceHistoryProxy);
    m_invoiceHistoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_invoiceHistoryView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_invoiceHistoryView->horizontalHeader()->setStretchLastSection(true);
    m_invoiceHistoryView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_invoiceHistoryView);

    return widget;
}

QWidget* MainWindow::createSettingsTab()
{
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    auto formLayout = new QFormLayout;
    m_companyName = new QLineEdit(widget);
    m_companyGst = new QLineEdit(widget);
    m_companyAddress = new QLineEdit(widget);
    m_companyPhone = new QLineEdit(widget);
    m_companyEmail = new QLineEdit(widget);
    formLayout->addRow(tr("Company name:"), m_companyName);
    formLayout->addRow(tr("Company GST:"), m_companyGst);
    formLayout->addRow(tr("Address:"), m_companyAddress);
    formLayout->addRow(tr("Phone:"), m_companyPhone);
    formLayout->addRow(tr("Email:"), m_companyEmail);
    layout->addLayout(formLayout);

    auto buttons = new QHBoxLayout;
    m_saveSettingsButton = new QPushButton(tr("Save settings"), widget);
    m_clearSettingsButton = new QPushButton(tr("Reset form"), widget);
    m_darkModeButton = new QPushButton(tr("Dark mode"), widget);
    m_darkModeButton->setCheckable(true);
    buttons->addWidget(m_saveSettingsButton);
    buttons->addWidget(m_clearSettingsButton);
    buttons->addWidget(m_darkModeButton);
    layout->addLayout(buttons);

    connect(m_saveSettingsButton, &QPushButton::clicked, this, &MainWindow::saveSettings);
    connect(m_clearSettingsButton, &QPushButton::clicked, [this](){ loadSettings(); });
    connect(m_darkModeButton, &QPushButton::toggled, this, &MainWindow::toggleDarkMode);

    return widget;
}

void MainWindow::refreshVendors()
{
    const QString filter = m_vendorFilter ? m_vendorFilter->text().trimmed() : QString();
    const QVector<Vendor> vendors = VendorService::listVendors(filter);

    m_vendorTable->setRowCount(vendors.count());
    for (int row = 0; row < vendors.count(); ++row) {
        const Vendor& vendor = vendors.at(row);
        QList<QTableWidgetItem*> rowItems;
        rowItems.append(new QTableWidgetItem(vendor.name));
        rowItems.append(new QTableWidgetItem(vendor.gst));
        rowItems.append(new QTableWidgetItem(vendor.phone));
        rowItems.append(new QTableWidgetItem(vendor.email));
        rowItems.append(new QTableWidgetItem(vendor.address));
        rowItems.append(new QTableWidgetItem(vendor.taxDetails));

        for (auto* item : rowItems) {
            item->setData(Qt::UserRole, vendor.id);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }

        for (int col = 0; col < rowItems.count(); ++col)
            m_vendorTable->setItem(row, col, rowItems[col]);
    }
    m_vendorTable->resizeRowsToContents();
    refreshVendorCompleter();
}

void MainWindow::filterVendors(const QString& filter)
{
    Q_UNUSED(filter)
    refreshVendors();
}

void MainWindow::updateVendorSelection()
{
    const QModelIndex current = m_vendorTable->currentIndex();
    if (!current.isValid()) {
        clearVendorForm();
        return;
    }

    const int vendorId = m_vendorTable->item(current.row(), 0)->data(Qt::UserRole).toInt();
    const QVector<Vendor> matches = VendorService::listVendors();
    for (const Vendor& vendor : matches) {
        if (vendor.id == vendorId) {
            m_currentVendorId = vendor.id;
            m_vendorName->setText(vendor.name);
            m_vendorGst->setText(vendor.gst);
            m_vendorPhone->setText(vendor.phone);
            m_vendorEmail->setText(vendor.email);
            m_vendorAddress->setText(vendor.address);
            m_vendorTaxDetails->setText(vendor.taxDetails);
            return;
        }
    }
}

void MainWindow::saveVendor()
{
    Vendor vendor;
    vendor.id = m_currentVendorId;
    vendor.name = m_vendorName->text().trimmed();
    vendor.gst = m_vendorGst->text().trimmed();
    vendor.phone = m_vendorPhone->text().trimmed();
    vendor.email = m_vendorEmail->text().trimmed();
    vendor.address = m_vendorAddress->text().trimmed();
    vendor.taxDetails = m_vendorTaxDetails->text().trimmed();

    if (vendor.name.isEmpty()) {
        QMessageBox::warning(this, tr("Validation error"), tr("Vendor name is required."));
        return;
    }

    QString error;
    if (!VendorService::addOrUpdateVendor(vendor, &error, nullptr)) {
        QMessageBox::critical(this, tr("Save failed"), error);
        return;
    }

    refreshVendors();
    clearVendorForm();
    QMessageBox::information(this, tr("Saved"), tr("Vendor saved successfully."));
}

void MainWindow::deleteVendor()
{
    if (m_currentVendorId == -1) {
        QMessageBox::warning(this, tr("Remove vendor"), tr("Select a vendor first."));
        return;
    }

    QString error;
    if (!VendorService::removeVendor(m_currentVendorId, &error)) {
        QMessageBox::critical(this, tr("Delete failed"), error);
        return;
    }

    refreshVendors();
    clearVendorForm();
    QMessageBox::information(this, tr("Deleted"), tr("Vendor removed."));
}

void MainWindow::clearVendorForm()
{
    m_currentVendorId = -1;
    m_vendorName->clear();
    m_vendorGst->clear();
    m_vendorPhone->clear();
    m_vendorEmail->clear();
    m_vendorAddress->clear();
    m_vendorTaxDetails->clear();
    m_vendorTable->clearSelection();
}

void MainWindow::addInvoiceItem()
{
    m_invoiceItemModel->addRow();
    updateInvoiceTotals();
}

void MainWindow::removeInvoiceItem()
{
    const QModelIndex current = m_invoiceItemsView->currentIndex();
    if (!current.isValid()) {
        QMessageBox::warning(this, tr("Remove item"), tr("Select an item row first."));
        return;
    }

    m_invoiceItemModel->removeRow(current.row());
    updateInvoiceTotals();
}

void MainWindow::updateInvoiceTotals()
{
    const double subtotal = m_invoiceItemModel->subtotal();
    const double gst = m_invoiceItemModel->gstTotal();
    const double total = m_invoiceItemModel->grandTotal();
    m_invoiceSubtotal->setText(QString::number(subtotal, 'f', 2));
    m_invoiceGst->setText(QString::number(gst, 'f', 2));
    m_invoiceTotal->setText(QString::number(total, 'f', 2));
}

void MainWindow::resetInvoiceForm()
{
    m_currentInvoiceId = -1;
    m_invoiceNo->clear();
    m_invoiceVendor->clear();
    m_invoiceDate->setDate(QDate::currentDate());
    m_invoiceDueDate->setDate(QDate::currentDate().addDays(15));
    m_invoiceNotes->clear();
    m_invoiceItemModel->resetRows();
    m_invoiceItemModel->addRow();
    updateInvoiceTotals();
}

void MainWindow::refreshVendorCompleter()
{
    const QVector<QString> vendorNames = VendorService::vendorNames();
    QStringList list;
    for (const QString& name : vendorNames) {
        list.append(name);
    }
    auto completer = new QCompleter(list, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    m_invoiceVendor->setCompleter(completer);
}

bool MainWindow::buildInvoiceFromForm(Invoice& invoice, QVector<InvoiceItem>& items, QString* errorOut)
{
    const QString customer = m_invoiceVendor->text().trimmed();
    if (customer.isEmpty()) {
        if (errorOut) *errorOut = tr("Select a customer for the invoice.");
        return false;
    }

    Vendor vendor;
    if (!VendorService::findVendorByName(customer, &vendor)) {
        if (errorOut) *errorOut = tr("Customer not found. Add the vendor first.");
        return false;
    }

    invoice.id = m_currentInvoiceId;
    invoice.invoiceNo = m_invoiceNo->text().trimmed();
    invoice.vendorId = vendor.id;
    invoice.vendorName = vendor.name;
    invoice.invoiceDate = m_invoiceDate->date();
    invoice.dueDate = m_invoiceDueDate->date();
    invoice.notes = m_invoiceNotes->text().trimmed();
    invoice.subtotal = m_invoiceItemModel->subtotal();
    invoice.gstTotal = m_invoiceItemModel->gstTotal();
    invoice.grandTotal = m_invoiceItemModel->grandTotal();
    invoice.status = "Pending";

    if (invoice.subtotal <= 0.0) {
        if (errorOut) *errorOut = tr("Invoice must contain at least one item with a positive value.");
        return false;
    }

    items = m_invoiceItemModel->items();
    return true;
}

void MainWindow::saveInvoice()
{
    Invoice invoice;
    QVector<InvoiceItem> items;
    QString error;
    if (!buildInvoiceFromForm(invoice, items, &error)) {
        QMessageBox::warning(this, tr("Validation error"), error);
        return;
    }

    if (!InvoiceService::saveInvoice(invoice, items, &error)) {
        QMessageBox::critical(this, tr("Save failed"), error);
        return;
    }
    m_currentInvoiceId = invoice.id;    refreshInvoiceHistory();
    QMessageBox::information(this, tr("Saved"), tr("Invoice saved successfully."));
    resetInvoiceForm();
}

void MainWindow::exportInvoice()
{
    Invoice invoice;
    QVector<InvoiceItem> items;
    QString error;
    if (!buildInvoiceFromForm(invoice, items, &error)) {
        QMessageBox::warning(this, tr("Export error"), error);
        return;
    }

    if (invoice.invoiceNo.isEmpty()) {
        invoice.invoiceNo = QString("INV-%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    }

    const QString path = QFileDialog::getSaveFileName(this, tr("Export invoice to PDF"), invoice.invoiceNo + ".pdf", tr("PDF files (*.pdf)"));
    if (path.isEmpty())
        return;

    if (invoice.id == -1) {
        QString saveError;
        if (!InvoiceService::saveInvoice(invoice, items, &saveError)) {
            QMessageBox::critical(this, tr("Export failed"), saveError);
            return;
        }
        m_currentInvoiceId = invoice.id;
        refreshInvoiceHistory();
    }

    CompanySettings settings;
    QString settingsError;
    SettingsService::loadSettings(&settings, &settingsError);
    if (!InvoiceService::exportInvoicePdf(invoice, items, path,
                                         settings.companyName, settings.companyGst,
                                         settings.address, settings.phone,
                                         settings.email, &error)) {
        QMessageBox::critical(this, tr("Export failed"), error);
        return;
    }
    // Show preview inside the application
    showPdfPreview(path);
    QMessageBox::information(this, tr("Exported"), tr("Invoice exported to PDF."));
}

void MainWindow::saveAndExportInvoice()
{
    Invoice invoice;
    QVector<InvoiceItem> items;
    QString error;
    if (!buildInvoiceFromForm(invoice, items, &error)) {
        QMessageBox::warning(this, tr("Validation error"), error);
        return;
    }

    if (!InvoiceService::saveInvoice(invoice, items, &error)) {
        QMessageBox::critical(this, tr("Save failed"), error);
        return;
    }

    m_currentInvoiceId = invoice.id;
    refreshInvoiceHistory();

    const QString path = QFileDialog::getSaveFileName(this, tr("Save and export invoice to PDF"), invoice.invoiceNo + ".pdf", tr("PDF files (*.pdf)"));
    if (path.isEmpty())
        return;

    CompanySettings settings;
    QString settingsError;
    SettingsService::loadSettings(&settings, &settingsError);
    if (!InvoiceService::exportInvoicePdf(invoice, items, path,
                                         settings.companyName, settings.companyGst,
                                         settings.address, settings.phone,
                                         settings.email, &error)) {
        QMessageBox::critical(this, tr("Export failed"), error);
        return;
    }
    // Show preview inside the application
    showPdfPreview(path);
    QMessageBox::information(this, tr("Exported"), tr("Invoice saved and exported to PDF."));
    resetInvoiceForm();
}

void MainWindow::showPdfPreview(const QString& filePath)
{
#ifdef HAVE_QTPDF
    QDialog dlg(this);
    dlg.setWindowTitle(tr("PDF Preview"));
    dlg.resize(800, 1000);

    auto layout = new QVBoxLayout(&dlg);

    auto doc = new QPdfDocument(&dlg);
    auto view = new QPdfView(&dlg);
    view->setDocument(doc);

    doc->load(filePath);

    layout->addWidget(view);

    auto buttons = new QHBoxLayout;
    auto closeBtn = new QPushButton(tr("Close"), &dlg);
    buttons->addStretch();
    buttons->addWidget(closeBtn);
    layout->addLayout(buttons);

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
#else
    // Fallback: open with external PDF viewer
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
#endif
}

void MainWindow::refreshInvoiceHistory()
{
    const QVector<Invoice> invoices = InvoiceService::listInvoices();
    m_invoiceHistoryModel->setRowCount(invoices.count());

    for (int row = 0; row < invoices.count(); ++row) {
        const Invoice& invoice = invoices.at(row);
        m_invoiceHistoryModel->setItem(row, 0, new QStandardItem(invoice.invoiceNo));
        m_invoiceHistoryModel->setItem(row, 1, new QStandardItem(invoice.invoiceDate.toString(Qt::ISODate)));
        m_invoiceHistoryModel->setItem(row, 2, new QStandardItem(invoice.dueDate.toString(Qt::ISODate)));
        m_invoiceHistoryModel->setItem(row, 3, new QStandardItem(invoice.vendorName));
        m_invoiceHistoryModel->setItem(row, 4, new QStandardItem(QString::number(invoice.subtotal, 'f', 2)));
        m_invoiceHistoryModel->setItem(row, 5, new QStandardItem(QString::number(invoice.gstTotal, 'f', 2)));
        m_invoiceHistoryModel->setItem(row, 6, new QStandardItem(QString::number(invoice.grandTotal, 'f', 2)));
    }
}

void MainWindow::filterInvoices(const QString& filter)
{
    m_invoiceHistoryProxy->setFilterRegularExpression(QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption));
}

void MainWindow::loadSettings()
{
    CompanySettings settings;
    QString error;
    if (!SettingsService::loadSettings(&settings, &error)) {
        QMessageBox::warning(this, tr("Settings"), tr("Unable to load settings: %1").arg(error));
    }
    m_companyName->setText(settings.companyName);
    m_companyGst->setText(settings.companyGst);
    m_companyAddress->setText(settings.address);
    m_companyPhone->setText(settings.phone);
    m_companyEmail->setText(settings.email);
}

void MainWindow::saveSettings()
{
    CompanySettings settings;
    settings.companyName = m_companyName->text().trimmed();
    settings.companyGst = m_companyGst->text().trimmed();
    settings.address = m_companyAddress->text().trimmed();
    settings.phone = m_companyPhone->text().trimmed();
    settings.email = m_companyEmail->text().trimmed();

    QString error;
    if (!SettingsService::saveSettings(settings, &error)) {
        QMessageBox::critical(this, tr("Settings"), tr("Unable to save settings: %1").arg(error));
        return;
    }

    QMessageBox::information(this, tr("Settings"), tr("Company settings saved."));
}

void MainWindow::toggleDarkMode(bool checked)
{
    if (checked) {
        qApp->setStyleSheet("QWidget { background-color: #2b2b2b; color: #eeeeee; }"
                            "QLineEdit, QTableWidget, QTableView, QDateEdit { background: #3c3f41; color: #ffffff; }"
                            "QPushButton { background: #4a4a4a; color: #ffffff; }"
                            "QHeaderView::section { background: #3c3f41; color: #ffffff; }");
    } else {
        qApp->setStyleSheet("");
    }
}
