#include "clientspage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLocale>
#include <QSqlQuery>
#include <QSqlError>
#include <QShowEvent>

static QString rupees(double value) {
    QLocale in(QLocale::English, QLocale::India);
    return "₹" + in.toString(value, 'f', 0);
}

static QPushButton *actionButton(const QString &text, const QString &color) {
    QPushButton *b = new QPushButton(text);
    b->setCursor(Qt::PointingHandCursor);
    b->setFixedHeight(32);
    b->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: white; border: none;"
        "border-radius: 8px; padding: 0 12px; font-size: 13px;"
        "font-weight: 600; }").arg(color));
    return b;
}

ClientsPage::ClientsPage(QWidget *parent)
    : QWidget(parent) {

    setupUI();
}

void ClientsPage::setupUI() {

    setStyleSheet(R"(

        QWidget {
            background-color: #EDF2FA;
            color: #1E293B;
            font-size: 15px;
        }

    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 35, 35, 35);

    /*
     * HEADER
     */

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QVBoxLayout *titleLayout = new QVBoxLayout();

    QLabel *heading = new QLabel("Clients");
    heading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 40px; font-weight: bold;");

    QLabel *subheading =
        new QLabel("Manage your clients and GST details");
    subheading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #64748B; font-size: 16px;");

    titleLayout->addWidget(heading);
    titleLayout->addWidget(subheading);

    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Search clients...");
    searchBar->setFixedWidth(260);
    searchBar->setFixedHeight(48);
    searchBar->setStyleSheet(R"(

        QLineEdit {
            background-color: #FFF9F4;
            border: 1px solid #E5D6C8;
            border-radius: 14px;
            padding-left: 14px;
            color: #1E293B;
            font-size: 15px;
        }

        QLineEdit:focus {
            border: 2px solid #2563EB;
        }

    )");

    QPushButton *addClientBtn = new QPushButton("+ Add Client");
    addClientBtn->setFixedHeight(50);
    addClientBtn->setCursor(Qt::PointingHandCursor);
    addClientBtn->setStyleSheet(R"(

        QPushButton {
            background-color: #2563EB;
            color: white;
            border: none;
            border-radius: 16px;
            padding-left: 22px;
            padding-right: 22px;
            font-size: 15px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #1D4ED8;
        }

    )");

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(searchBar);
    headerLayout->addSpacing(14);
    headerLayout->addWidget(addClientBtn);

    /*
     * TABLE CARD
     */

    QFrame *tableCard = new QFrame();
    tableCard->setStyleSheet(
        "background-color: #F6EFE7;"
        "border-radius: 26px;"
        "border: 1px solid #E5D6C8;");

    QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *tableTitle = new QLabel("Client List");
    tableTitle->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 25px; font-weight: bold;");

    table = new QTableWidget();
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({
        "Client Name", "Phone", "Email", "GST Number",
        "Address", "Invoices", "Total Billed", "Actions"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    table->setColumnWidth(7, 170);
    table->verticalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setMinimumHeight(450);
    table->verticalHeader()->setDefaultSectionSize(54);
    table->setStyleSheet(R"(

        QTableWidget {
            background-color: #FFF9F4;
            border-radius: 20px;
            border: 1px solid #E5D6C8;
            padding: 12px;
            font-size: 15px;
        }

        QHeaderView::section {
            background-color: #EFE5DA;
            color: #1E293B;
            border: none;
            font-size: 15px;
            font-weight: bold;
            padding: 16px;
        }

        QTableWidget::item {
            padding: 10px;
        }

    )");

    tableLayout->addWidget(tableTitle);
    tableLayout->addSpacing(18);
    tableLayout->addWidget(table);

    /*
     * FINAL LAYOUT
     */

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(tableCard);

    /*
     * CONNECTIONS
     */

    connect(addClientBtn, &QPushButton::clicked,
            this, &ClientsPage::addClient);

    connect(searchBar, &QLineEdit::textChanged,
            this, &ClientsPage::filterClients);
}

void ClientsPage::showEvent(QShowEvent *event) {

    QWidget::showEvent(event);

    loadClients();
}

void ClientsPage::loadClients() {

    table->setRowCount(0);

    QSqlQuery query(
        "SELECT c.id, c.name, c.phone, c.email, c.gst_number, c.address, "
        "(SELECT COUNT(*) FROM invoices WHERE client_id = c.id), "
        "(SELECT COALESCE(SUM(grand_total), 0) FROM invoices WHERE client_id = c.id) "
        "FROM clients c ORDER BY c.name");

    int row = 0;

    while(query.next()) {

        const int id = query.value(0).toInt();
        const QString name = query.value(1).toString();

        table->insertRow(row);

        // text columns 0..4 (name, phone, email, gst, address)
        for(int col = 0; col < 5; col++)
            table->setItem(row, col,
                new QTableWidgetItem(query.value(col + 1).toString()));

        table->setItem(row, 5,
            new QTableWidgetItem(QString::number(query.value(6).toInt())));
        table->setItem(row, 6,
            new QTableWidgetItem(rupees(query.value(7).toDouble())));

        table->setCellWidget(row, 7, makeActions(id, name));

        row++;
    }

    filterClients(searchBar->text());
}

QWidget *ClientsPage::makeActions(int id, const QString &name) {

    QWidget *w = new QWidget();
    QHBoxLayout *l = new QHBoxLayout(w);
    l->setContentsMargins(6, 4, 6, 4);
    l->setSpacing(6);

    QPushButton *editBtn = actionButton("Edit", "#2563EB");
    QPushButton *delBtn = actionButton("Delete", "#DC2626");

    l->addWidget(editBtn);
    l->addWidget(delBtn);

    connect(editBtn, &QPushButton::clicked, this, [=]() { editClient(id); });
    connect(delBtn, &QPushButton::clicked, this,
            [=]() { deleteClient(id, name); });

    return w;
}

void ClientsPage::addClient() {

    QDialog dialog(this);
    dialog.setWindowTitle("Add Client");
    dialog.setMinimumWidth(380);

    QFormLayout form(&dialog);

    QLineEdit *name = new QLineEdit(&dialog);
    QLineEdit *phone = new QLineEdit(&dialog);
    QLineEdit *email = new QLineEdit(&dialog);
    QLineEdit *gst = new QLineEdit(&dialog);
    QLineEdit *address = new QLineEdit(&dialog);

    form.addRow("Name", name);
    form.addRow("Phone", phone);
    form.addRow("Email", email);
    form.addRow("GST Number", gst);
    form.addRow("Address", address);

    QDialogButtonBox buttons(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);

    form.addRow(&buttons);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if(dialog.exec() != QDialog::Accepted)
        return;

    if(name->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Add Client", "Name is required.");
        return;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO clients (name, phone, email, gst_number, address) "
        "VALUES (:name, :phone, :email, :gst, :address)");

    query.bindValue(":name", name->text().trimmed());
    query.bindValue(":phone", phone->text().trimmed());
    query.bindValue(":email", email->text().trimmed());
    query.bindValue(":gst", gst->text().trimmed());
    query.bindValue(":address", address->text().trimmed());

    if(!query.exec()) {
        QMessageBox::critical(this, "Add Client",
            "Could not save client:\n" + query.lastError().text());
        return;
    }

    loadClients();
}

void ClientsPage::editClient(int id) {

    QSqlQuery load;
    load.prepare(
        "SELECT name, phone, email, gst_number, address "
        "FROM clients WHERE id = :id");
    load.bindValue(":id", id);

    if(!load.exec() || !load.next())
        return;

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Client");
    dialog.setMinimumWidth(380);

    QFormLayout form(&dialog);

    QLineEdit *name = new QLineEdit(load.value(0).toString(), &dialog);
    QLineEdit *phone = new QLineEdit(load.value(1).toString(), &dialog);
    QLineEdit *email = new QLineEdit(load.value(2).toString(), &dialog);
    QLineEdit *gst = new QLineEdit(load.value(3).toString(), &dialog);
    QLineEdit *address = new QLineEdit(load.value(4).toString(), &dialog);

    form.addRow("Name", name);
    form.addRow("Phone", phone);
    form.addRow("Email", email);
    form.addRow("GST Number", gst);
    form.addRow("Address", address);

    QDialogButtonBox buttons(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);

    form.addRow(&buttons);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if(dialog.exec() != QDialog::Accepted)
        return;

    if(name->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Edit Client", "Name is required.");
        return;
    }

    QSqlQuery query;
    query.prepare(
        "UPDATE clients SET name = :name, phone = :phone, email = :email, "
        "gst_number = :gst, address = :address WHERE id = :id");

    query.bindValue(":name", name->text().trimmed());
    query.bindValue(":phone", phone->text().trimmed());
    query.bindValue(":email", email->text().trimmed());
    query.bindValue(":gst", gst->text().trimmed());
    query.bindValue(":address", address->text().trimmed());
    query.bindValue(":id", id);

    if(!query.exec()) {
        QMessageBox::critical(this, "Edit Client",
            "Could not update client:\n" + query.lastError().text());
        return;
    }

    loadClients();
}

void ClientsPage::deleteClient(int id, const QString &name) {

    if(QMessageBox::question(this, "Delete Client",
           "Delete \"" + name + "\"? This cannot be undone.")
       != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM clients WHERE id = :id");
    query.bindValue(":id", id);

    if(!query.exec()) {
        QMessageBox::critical(this, "Delete Client",
            "Could not delete client. They may still have invoices:\n"
            + query.lastError().text());
        return;
    }

    loadClients();
}

void ClientsPage::filterClients(const QString &text) {

    for(int row = 0; row < table->rowCount(); row++) {

        bool match = text.isEmpty();

        // Only scan the text columns (0..6); the Actions column has no item.
        for(int col = 0; col < 7 && !match; col++) {

            QTableWidgetItem *item = table->item(row, col);

            if(item && item->text().contains(text, Qt::CaseInsensitive))
                match = true;
        }

        table->setRowHidden(row, !match);
    }
}
