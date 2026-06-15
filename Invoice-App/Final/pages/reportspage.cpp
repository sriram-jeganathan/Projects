#include "reportspage.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QLocale>
#include <QSqlQuery>
#include <QShowEvent>

static QString rupees(double value) {
    QLocale in(QLocale::English, QLocale::India);
    return "₹" + in.toString(value, 'f', 0);
}

ReportsPage::ReportsPage(QWidget *parent)
    : QWidget(parent) {

    setupUI();
}

void ReportsPage::setupUI() {

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

    QVBoxLayout *titleLayout = new QVBoxLayout();

    QLabel *heading = new QLabel("Reports");
    heading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 42px; font-weight: bold;");

    QLabel *subheading =
        new QLabel("Revenue and GST summary at a glance");
    subheading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #64748B; font-size: 17px;");

    titleLayout->addWidget(heading);
    titleLayout->addWidget(subheading);

    /*
     * STATS CARDS
     */

    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(22);

    QStringList titles = {
        "Total Revenue", "GST Collected",
        "Outstanding", "Invoices This Month"
    };

    for(int i = 0; i < 4; i++) {

        QFrame *card = new QFrame();
        card->setMinimumHeight(220);
        card->setStyleSheet(
            "background-color: #F6EFE7;"
            "border-radius: 28px;"
            "border: 1px solid #E5D6C8;");

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(32, 28, 32, 28);

        QLabel *title = new QLabel(titles[i]);
        title->setStyleSheet(
            "background: transparent; border: none;"
            "color: #64748B; font-size: 19px; font-weight: 500;");

        QLabel *value = new QLabel("0");
        value->setStyleSheet(
            "background: transparent; border: none;"
            "color: #1E293B; font-size: 44px; font-weight: bold;");

        statValues.append(value);

        cardLayout->addWidget(title);
        cardLayout->addStretch();
        cardLayout->addWidget(value);

        statsLayout->addWidget(card);
    }

    /*
     * MONTHLY BREAKDOWN TABLE
     */

    QFrame *tableCard = new QFrame();
    tableCard->setStyleSheet(
        "background-color: #F6EFE7;"
        "border-radius: 28px;"
        "border: 1px solid #E5D6C8;");

    QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *tableTitle = new QLabel("Monthly Breakdown");
    tableTitle->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 26px; font-weight: bold;");

    monthlyTable = new QTableWidget();
    monthlyTable->setColumnCount(4);
    monthlyTable->setHorizontalHeaderLabels({
        "Month", "Invoices", "Revenue", "GST Collected"
    });
    monthlyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    monthlyTable->verticalHeader()->setVisible(false);
    monthlyTable->setShowGrid(false);
    monthlyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    monthlyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    monthlyTable->setMinimumHeight(340);
    monthlyTable->setStyleSheet(R"(

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
    tableLayout->addWidget(monthlyTable);

    /*
     * FINAL LAYOUT
     */

    contentLayout->addLayout(titleLayout);
    contentLayout->addSpacing(35);
    contentLayout->addLayout(statsLayout);
    contentLayout->addSpacing(30);
    contentLayout->addWidget(tableCard);
}

void ReportsPage::showEvent(QShowEvent *event) {

    QWidget::showEvent(event);

    loadReport();
}

void ReportsPage::loadReport() {

    double revenue = 0;
    double gst = 0;
    double outstanding = 0;
    int monthCount = 0;

    QSqlQuery q1("SELECT COALESCE(SUM(grand_total), 0) FROM invoices");
    if(q1.next()) revenue = q1.value(0).toDouble();

    // gst_total is stored per invoice, so just sum it.
    QSqlQuery q2("SELECT COALESCE(SUM(gst_total), 0) FROM invoices");
    if(q2.next()) gst = q2.value(0).toDouble();

    QSqlQuery q3(
        "SELECT COALESCE(SUM(grand_total), 0) "
        "FROM invoices WHERE status <> 'Paid'");
    if(q3.next()) outstanding = q3.value(0).toDouble();

    QSqlQuery q4(
        "SELECT COUNT(*) FROM invoices "
        "WHERE YEAR(invoice_date) = YEAR(CURDATE()) "
        "AND MONTH(invoice_date) = MONTH(CURDATE())");
    if(q4.next()) monthCount = q4.value(0).toInt();

    if(statValues.size() == 4) {
        statValues[0]->setText(rupees(revenue));
        statValues[1]->setText(rupees(gst));
        statValues[2]->setText(rupees(outstanding));
        statValues[3]->setText(QString::number(monthCount));
    }

    /*
     * MONTHLY BREAKDOWN
     */

    monthlyTable->setRowCount(0);

    QSqlQuery monthly(
        "SELECT DATE_FORMAT(invoice_date, '%M %Y') AS label, "
        "COUNT(*), "
        "COALESCE(SUM(grand_total), 0), "
        "COALESCE(SUM(gst_total), 0) "
        "FROM invoices "
        "GROUP BY YEAR(invoice_date), MONTH(invoice_date) "
        "ORDER BY YEAR(invoice_date), MONTH(invoice_date)");

    int row = 0;

    while(monthly.next()) {

        monthlyTable->insertRow(row);

        monthlyTable->setItem(row, 0,
            new QTableWidgetItem(monthly.value(0).toString()));
        monthlyTable->setItem(row, 1,
            new QTableWidgetItem(QString::number(monthly.value(1).toInt())));
        monthlyTable->setItem(row, 2,
            new QTableWidgetItem(rupees(monthly.value(2).toDouble())));
        monthlyTable->setItem(row, 3,
            new QTableWidgetItem(rupees(monthly.value(3).toDouble())));

        row++;
    }
}
