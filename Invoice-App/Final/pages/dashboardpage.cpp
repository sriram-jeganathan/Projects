#include "dashboardpage.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QColor>
#include <QDate>
#include <QLocale>
#include <QSqlQuery>
#include <QShowEvent>

static QString rupees(double value) {
    QLocale in(QLocale::English, QLocale::India);
    return "₹" + in.toString(value, 'f', 0);
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent) {

    setupUI();
}

void DashboardPage::setupUI() {

    setStyleSheet(R"(

        background-color: #EDF2FA;
        color: #1E293B;
        font-size: 15px;

    )");

    QVBoxLayout *contentLayout = new QVBoxLayout(this);
    contentLayout->setContentsMargins(35, 35, 35, 35);

    /*
     * HEADER
     */

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QVBoxLayout *titleLayout = new QVBoxLayout();

    QLabel *heading = new QLabel("Dashboard");
    heading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 42px; font-weight: bold;");

    QLabel *subheading =
        new QLabel("Overview of your invoices and clients");
    subheading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #64748B; font-size: 17px;");

    titleLayout->addWidget(heading);
    titleLayout->addWidget(subheading);

    QPushButton *newInvoiceBtn = new QPushButton("+ New Invoice");
    newInvoiceBtn->setFixedHeight(55);
    newInvoiceBtn->setCursor(Qt::PointingHandCursor);
    newInvoiceBtn->setStyleSheet(R"(

        QPushButton {
            background-color: #2563EB;
            color: white;
            border: none;
            border-radius: 16px;
            padding-left: 24px;
            padding-right: 24px;
            font-size: 15px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #1D4ED8;
        }

    )");

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(newInvoiceBtn);

    /*
     * STATS CARDS
     */

    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(22);

    QStringList titles = {
        "Total Clients", "Total Invoices",
        "Paid Amount", "Outstanding"
    };

    for(int i = 0; i < 4; i++) {

        QFrame *card = new QFrame();
        card->setMinimumHeight(240);
        card->setStyleSheet(
            "background-color: #F6EFE7;"
            "border-radius: 28px;"
            "border: 1px solid #E5D6C8;");

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(32, 28, 32, 28);

        QLabel *title = new QLabel(titles[i]);
        title->setStyleSheet(
            "background: transparent; border: none;"
            "color: #64748B; font-size: 20px; font-weight: 500;");

        QLabel *value = new QLabel("0");
        value->setStyleSheet(
            "background: transparent; border: none;"
            "color: #1E293B; font-size: 48px; font-weight: bold;");

        statValues.append(value);

        cardLayout->addWidget(title);
        cardLayout->addStretch();
        cardLayout->addWidget(value);

        statsLayout->addWidget(card);
    }

    /*
     * RECENT INVOICES TABLE
     */

    QFrame *tableCard = new QFrame();
    tableCard->setStyleSheet(
        "background-color: #F6EFE7;"
        "border-radius: 28px;"
        "border: 1px solid #E5D6C8;");

    QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *tableTitle = new QLabel("Recent Invoices");
    tableTitle->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 26px; font-weight: bold;");

    recentTable = new QTableWidget();
    recentTable->setColumnCount(6);
    recentTable->setHorizontalHeaderLabels({
        "Invoice No", "Client", "Date", "GST %", "Total", "Status"
    });
    recentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentTable->verticalHeader()->setVisible(false);
    recentTable->setShowGrid(false);
    recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentTable->setMinimumHeight(320);
    recentTable->setStyleSheet(R"(

        QTableWidget {
            background-color: #FFF9F4;
            border-radius: 20px;
            border: 1px solid #E5D6C8;
            padding: 14px;
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
            padding: 14px;
        }

    )");

    tableLayout->addWidget(tableTitle);
    tableLayout->addSpacing(18);
    tableLayout->addWidget(recentTable);

    /*
     * FINAL LAYOUT
     */

    contentLayout->addLayout(headerLayout);
    contentLayout->addSpacing(35);
    contentLayout->addLayout(statsLayout);
    contentLayout->addSpacing(30);
    contentLayout->addWidget(tableCard);

    /*
     * The button hands off to MainWindow, which switches to the
     * Invoices page and opens the New Invoice form there.
     */
    connect(newInvoiceBtn, &QPushButton::clicked,
            this, &DashboardPage::newInvoiceRequested);
}

void DashboardPage::showEvent(QShowEvent *event) {

    QWidget::showEvent(event);

    loadStats();
    loadRecentInvoices();
}

void DashboardPage::loadStats() {

    int clients = 0;
    int invoices = 0;
    double paid = 0;
    double outstanding = 0;

    QSqlQuery q1("SELECT COUNT(*) FROM clients");
    if(q1.next()) clients = q1.value(0).toInt();

    QSqlQuery q2("SELECT COUNT(*) FROM invoices");
    if(q2.next()) invoices = q2.value(0).toInt();

    QSqlQuery q3(
        "SELECT COALESCE(SUM(grand_total), 0) "
        "FROM invoices WHERE status = 'Paid'");
    if(q3.next()) paid = q3.value(0).toDouble();

    QSqlQuery q4(
        "SELECT COALESCE(SUM(grand_total), 0) "
        "FROM invoices WHERE status <> 'Paid'");
    if(q4.next()) outstanding = q4.value(0).toDouble();

    if(statValues.size() == 4) {
        statValues[0]->setText(QString::number(clients));
        statValues[1]->setText(QString::number(invoices));
        statValues[2]->setText(rupees(paid));
        statValues[3]->setText(rupees(outstanding));
    }
}

void DashboardPage::loadRecentInvoices() {

    recentTable->setRowCount(0);

    QSqlQuery query(
        "SELECT i.invoice_number, COALESCE(c.name, '-'), "
        "i.invoice_date, i.gst_percent, i.grand_total, i.status "
        "FROM invoices i "
        "LEFT JOIN clients c ON i.client_id = c.id "
        "ORDER BY i.invoice_date DESC, i.id DESC "
        "LIMIT 5");

    int row = 0;

    while(query.next()) {

        recentTable->insertRow(row);

        recentTable->setItem(row, 0,
            new QTableWidgetItem(query.value(0).toString()));
        recentTable->setItem(row, 1,
            new QTableWidgetItem(query.value(1).toString()));

        QDate date = query.value(2).toDate();
        recentTable->setItem(row, 2,
            new QTableWidgetItem(date.isValid() ? date.toString("dd MMM") : ""));

        recentTable->setItem(row, 3,
            new QTableWidgetItem(
                QString::number(query.value(3).toDouble(), 'f', 0) + "%"));

        recentTable->setItem(row, 4,
            new QTableWidgetItem(rupees(query.value(4).toDouble())));

        const QString status = query.value(5).toString();
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);

        if(status == "Paid")
            statusItem->setForeground(QColor("#16A34A"));
        else if(status == "Pending")
            statusItem->setForeground(QColor("#EA580C"));
        else
            statusItem->setForeground(QColor("#DC2626"));

        recentTable->setItem(row, 5, statusItem);

        row++;
    }
}
