#include "invoicespage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QColor>
#include <QPen>
#include <QFont>
#include <QDate>
#include <QLocale>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QShowEvent>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
#include <QMarginsF>
#include <QDesktopServices>
#include <QUrl>

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
        "border-radius: 8px; padding: 0 10px; font-size: 13px;"
        "font-weight: 600; }").arg(color));
    return b;
}

InvoicesPage::InvoicesPage(QWidget *parent)
    : QWidget(parent) {

    setupUI();
}

void InvoicesPage::setupUI() {

    setStyleSheet(R"(

        QWidget {
            background-color: #EDF2FA;
            color: #1E293B;
            font-size: 15px;
        }

    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 35, 35, 35);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QVBoxLayout *titleLayout = new QVBoxLayout();

    QLabel *heading = new QLabel("Invoices");
    heading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 40px; font-weight: bold;");

    QLabel *subheading = new QLabel("Track and manage all your invoices");
    subheading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #64748B; font-size: 16px;");

    titleLayout->addWidget(heading);
    titleLayout->addWidget(subheading);

    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Search invoices...");
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

    QPushButton *newInvoiceBtn = new QPushButton("+ New Invoice");
    newInvoiceBtn->setFixedHeight(50);
    newInvoiceBtn->setCursor(Qt::PointingHandCursor);
    newInvoiceBtn->setStyleSheet(R"(

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
    headerLayout->addWidget(newInvoiceBtn);

    QFrame *tableCard = new QFrame();
    tableCard->setStyleSheet(
        "background-color: #F6EFE7;"
        "border-radius: 26px;"
        "border: 1px solid #E5D6C8;");

    QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *tableTitle = new QLabel("All Invoices");
    tableTitle->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 25px; font-weight: bold;");

    table = new QTableWidget();
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({
        "Invoice No", "Client", "Date", "Due Date",
        "GST %", "Total", "Status", "Actions"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    table->setColumnWidth(7, 250);
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

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(tableCard);

    connect(newInvoiceBtn, &QPushButton::clicked,
            this, &InvoicesPage::newInvoice);
    connect(searchBar, &QLineEdit::textChanged,
            this, &InvoicesPage::filterInvoices);
}

void InvoicesPage::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    loadInvoices();
}

void InvoicesPage::loadInvoices() {

    table->setRowCount(0);

    QSqlQuery query(
        "SELECT i.id, i.invoice_number, COALESCE(c.name, '-'), "
        "i.invoice_date, i.due_date, i.gst_percent, i.grand_total, i.status "
        "FROM invoices i "
        "LEFT JOIN clients c ON i.client_id = c.id "
        "ORDER BY i.invoice_date DESC, i.id DESC");

    int row = 0;

    while(query.next()) {

        const int id = query.value(0).toInt();
        const QString invNo = query.value(1).toString();
        const QString status = query.value(7).toString();

        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(invNo));
        table->setItem(row, 1, new QTableWidgetItem(query.value(2).toString()));

        QDate date = query.value(3).toDate();
        table->setItem(row, 2,
            new QTableWidgetItem(date.isValid() ? date.toString("dd MMM yyyy") : ""));

        QDate due = query.value(4).toDate();
        table->setItem(row, 3,
            new QTableWidgetItem(due.isValid() ? due.toString("dd MMM yyyy") : "-"));

        table->setItem(row, 4,
            new QTableWidgetItem(
                QString::number(query.value(5).toDouble(), 'f', 0) + "%"));

        table->setItem(row, 5,
            new QTableWidgetItem(rupees(query.value(6).toDouble())));

        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        if(status == "Paid")
            statusItem->setForeground(QColor("#16A34A"));
        else if(status == "Pending")
            statusItem->setForeground(QColor("#EA580C"));
        else
            statusItem->setForeground(QColor("#DC2626"));
        table->setItem(row, 6, statusItem);

        table->setCellWidget(row, 7, makeActions(id, invNo, status));

        row++;
    }

    filterInvoices(searchBar->text());
}

QWidget *InvoicesPage::makeActions(int id, const QString &invNo,
                                   const QString &status) {

    QWidget *w = new QWidget();
    QHBoxLayout *l = new QHBoxLayout(w);
    l->setContentsMargins(6, 4, 6, 4);
    l->setSpacing(6);

    QPushButton *statusBtn = actionButton("Status", "#2563EB");
    QPushButton *pdfBtn = actionButton("PDF", "#0EA5E9");
    QPushButton *delBtn = actionButton("Delete", "#DC2626");

    l->addWidget(statusBtn);
    l->addWidget(pdfBtn);
    l->addWidget(delBtn);

    connect(statusBtn, &QPushButton::clicked, this,
            [=]() { changeStatus(id, status); });
    connect(pdfBtn, &QPushButton::clicked, this, [=]() { exportPdf(id); });
    connect(delBtn, &QPushButton::clicked, this,
            [=]() { deleteInvoice(id, invNo); });

    return w;
}

void InvoicesPage::changeStatus(int invoiceId, const QString &current) {

    const QStringList options = {"Pending", "Paid", "Overdue"};
    int index = options.indexOf(current);
    if(index < 0) index = 0;

    bool ok = false;
    const QString choice = QInputDialog::getItem(
        this, "Update Status", "Status:", options, index, false, &ok);
    if(!ok) return;

    QSqlQuery query;
    query.prepare("UPDATE invoices SET status = :s WHERE id = :id");
    query.bindValue(":s", choice);
    query.bindValue(":id", invoiceId);

    if(!query.exec()) {
        QMessageBox::critical(this, "Update Status", query.lastError().text());
        return;
    }
    loadInvoices();
}

void InvoicesPage::deleteInvoice(int invoiceId, const QString &invoiceNumber) {

    if(QMessageBox::question(this, "Delete Invoice",
           "Delete invoice " + invoiceNumber + "? This cannot be undone.")
       != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM invoices WHERE id = :id");
    query.bindValue(":id", invoiceId);

    if(!query.exec()) {
        QMessageBox::critical(this, "Delete Invoice", query.lastError().text());
        return;
    }
    loadInvoices();
}

void InvoicesPage::exportPdf(int invoiceId) {

    QSqlQuery q;
    q.prepare(
        "SELECT i.invoice_number, i.invoice_date, i.due_date, i.invoice_items, "
        "i.subtotal, i.gst_percent, i.gst_total, i.grand_total, i.status, "
        "COALESCE(i.notes, ''), i.cgst, i.sgst, i.igst, "
        "COALESCE(i.place_of_supply, ''), "
        "COALESCE(c.name, '-'), COALESCE(c.address, ''), "
        "COALESCE(c.gst_number, ''), COALESCE(c.email, ''), "
        "COALESCE(c.phone, '') "
        "FROM invoices i LEFT JOIN clients c ON i.client_id = c.id "
        "WHERE i.id = :id");
    q.bindValue(":id", invoiceId);

    if(!q.exec() || !q.next()) {
        QMessageBox::critical(this, "Export PDF",
            "Invoice not found, or the cgst/sgst/igst columns are missing. "
            "Run alter.sql first.");
        return;
    }

    const QString invoiceNumber = q.value(0).toString();
    const QDate invoiceDate = q.value(1).toDate();
    const QDate dueDate = q.value(2).toDate();
    const QString itemsJson = q.value(3).toString();
    const double subtotal = q.value(4).toDouble();
    const double gstPercent = q.value(5).toDouble();
    const double grandTotal = q.value(7).toDouble();
    const QString status = q.value(8).toString();
    const QString notes = q.value(9).toString();
    const double cgst = q.value(10).toDouble();
    const double sgst = q.value(11).toDouble();
    const double igst = q.value(12).toDouble();
    const QString placeOfSupply = q.value(13).toString();
    const QString clientName = q.value(14).toString();
    const QString clientAddress = q.value(15).toString();
    const QString clientGst = q.value(16).toString();
    const QString clientEmail = q.value(17).toString();
    const QString clientPhone = q.value(18).toString();

    const bool interState = igst > 0.0001;

    QString path = QFileDialog::getSaveFileName(this, "Save Invoice PDF",
        invoiceNumber + ".pdf", "PDF Files (*.pdf)");
    if(path.isEmpty()) return;
    if(!path.endsWith(".pdf", Qt::CaseInsensitive)) path += ".pdf";

    QSettings s("InvoicePro", "InvoicePro");
    const QString coName = s.value("company/name", "InvoicePro").toString();
    const QString coGst = s.value("company/gstin", "").toString();
    const QString coPan = s.value("company/pan", "").toString();
    const QString coAddr = s.value("company/address", "").toString();

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    writer.setPageMargins(QMarginsF(14, 14, 14, 14), QPageLayout::Millimeter);

    QPainter p(&writer);
    if(!p.isActive()) {
        QMessageBox::critical(this, "Export PDF", "Could not write the PDF.");
        return;
    }

    const int W = p.viewport().width();

    // DejaVu Sans carries the rupee glyph and embeds cleanly.
    const QString fam = "DejaVu Sans";
    QFont titleF(fam, 22, QFont::Bold);
    QFont coNameF(fam, 12, QFont::Bold);
    QFont normal(fam, 9);
    QFont small(fam, 8);
    QFont hdr(fam, 9, QFont::Bold);
    QFont totalBold(fam, 11, QFont::Bold);

    QLocale in(QLocale::English, QLocale::India);
    auto money = [&](double v) { return "₹" + in.toString(v, 'f', 2); };

    const QColor dark("#1E293B");
    const QColor gray("#64748B");
    const QColor line("#E5D6C8");
    const QColor accent("#2563EB");

    // Logo monogram (top-left)
    p.setPen(Qt::NoPen);
    p.setBrush(accent);
    p.drawEllipse(QRect(0, 0, 150, 150));
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::white);
    p.setFont(QFont(fam, 34, QFont::Bold));
    p.drawText(QRect(0, 0, 150, 150), Qt::AlignCenter,
               coName.isEmpty() ? "I" : coName.left(1).toUpper());

    // Title + number (top-right)
    p.setPen(dark);
    p.setFont(titleF);
    p.drawText(QRect(W / 2, 0, W / 2, 110),
               Qt::AlignRight | Qt::AlignTop, "TAX INVOICE");
    p.setPen(gray);
    p.setFont(normal);
    p.drawText(QRect(W / 2, 112, W / 2, 50),
               Qt::AlignRight | Qt::AlignTop, "Invoice# " + invoiceNumber);

    // Company block (left)
    int yLeft = 185;
    p.setPen(dark);
    p.setFont(coNameF);
    p.drawText(QRect(0, yLeft, W / 2, 50), Qt::AlignLeft | Qt::AlignTop, coName);
    yLeft += 56;
    p.setFont(small);
    p.setPen(gray);
    QString coInfo;
    if(!coAddr.isEmpty()) coInfo += coAddr + "\n";
    if(!coGst.isEmpty())  coInfo += "GSTIN: " + coGst + "\n";
    if(!coPan.isEmpty())  coInfo += "PAN: " + coPan;
    p.drawText(QRect(0, yLeft, W / 2, 220), Qt::AlignLeft | Qt::AlignTop, coInfo);

    // Balance Due box (right)
    const double balanceDue =
        (status.compare("Paid", Qt::CaseInsensitive) == 0) ? 0.0 : grandTotal;
    QRect box(W - 760, 185, 760, 150);
    p.setPen(QPen(line, 4));
    p.setBrush(QColor("#FBF6F0"));
    p.drawRoundedRect(box, 14, 14);
    p.setBrush(Qt::NoBrush);
    p.setPen(gray);
    p.setFont(small);
    p.drawText(box.adjusted(28, 18, -28, 0),
               Qt::AlignLeft | Qt::AlignTop, "Balance Due");
    p.setPen(QColor("#EA580C"));
    p.setFont(QFont(fam, 18, QFont::Bold));
    p.drawText(box.adjusted(28, 0, -28, -16),
               Qt::AlignLeft | Qt::AlignBottom, money(balanceDue));

    int y = qMax(yLeft + 230, 380);

    // Bill To (left)
    int yBill = y;
    p.setPen(dark);
    p.setFont(hdr);
    p.drawText(QRect(0, yBill, W / 2, 40), Qt::AlignLeft, "Bill To");
    yBill += 48;
    p.setFont(QFont(fam, 10, QFont::Bold));
    p.drawText(QRect(0, yBill, W / 2, 40),
               Qt::AlignLeft | Qt::AlignTop, clientName);
    yBill += 46;
    p.setFont(small);
    p.setPen(gray);
    QString billInfo;
    if(!clientAddress.isEmpty()) billInfo += clientAddress + "\n";
    if(!clientGst.isEmpty())     billInfo += "GSTIN: " + clientGst + "\n";
    if(!clientPhone.isEmpty())   billInfo += "Phone: " + clientPhone + "\n";
    if(!clientEmail.isEmpty())   billInfo += clientEmail;
    p.drawText(QRect(0, yBill, W / 2, 220),
               Qt::AlignLeft | Qt::AlignTop, billInfo);

    // Meta (right)
    int yMeta = y;
    auto metaRow = [&](const QString &label, const QString &value) {
        p.setFont(small); p.setPen(gray);
        p.drawText(QRect(W / 2, yMeta, (W / 2) - 380, 46),
                   Qt::AlignRight | Qt::AlignVCenter, label + " :");
        p.setFont(normal); p.setPen(dark);
        p.drawText(QRect(W - 380, yMeta, 380, 46),
                   Qt::AlignRight | Qt::AlignVCenter, value);
        yMeta += 52;
    };
    metaRow("Invoice Date",
            invoiceDate.isValid() ? invoiceDate.toString("dd MMM yyyy") : "");
    metaRow("Due Date",
            dueDate.isValid() ? dueDate.toString("dd MMM yyyy") : "-");
    metaRow("Status", status);

    y = qMax(yBill + 240, yMeta) + 10;

    if(!placeOfSupply.isEmpty()) {
        p.setFont(small); p.setPen(gray);
        p.drawText(QRect(0, y, W, 40), Qt::AlignLeft | Qt::AlignVCenter,
                   "Place of Supply : " + placeOfSupply);
        y += 60;
    }

    // Items table
    const int wAmt = 330, wTax = 290, wRate = 270, wNum = 80;
    int xAmt = W - wAmt, xCgst = 0, xSgst = 0, xIgst = 0, xRate = 0, descEnd = 0;
    if(interState) {
        xIgst = xAmt - 360;
        xRate = xIgst - wRate;
        descEnd = xRate;
    } else {
        xSgst = xAmt - wTax;
        xCgst = xSgst - wTax;
        xRate = xCgst - wRate;
        descEnd = xRate;
    }
    const int descStart = wNum + 10;

    const int headH = 64;
    p.setPen(Qt::NoPen);
    p.setBrush(dark);
    p.drawRect(QRect(0, y, W, headH));
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::white);
    p.setFont(hdr);
    p.drawText(QRect(0, y, wNum, headH), Qt::AlignCenter, "#");
    p.drawText(QRect(descStart, y, descEnd - descStart, headH),
               Qt::AlignVCenter | Qt::AlignLeft, "Item & Description");
    p.drawText(QRect(xRate, y, wRate, headH),
               Qt::AlignVCenter | Qt::AlignRight, "Rate");
    if(interState) {
        p.drawText(QRect(xIgst, y, xAmt - xIgst - 10, headH),
                   Qt::AlignVCenter | Qt::AlignRight, "IGST");
    } else {
        p.drawText(QRect(xCgst, y, wTax - 10, headH),
                   Qt::AlignVCenter | Qt::AlignRight, "CGST");
        p.drawText(QRect(xSgst, y, wTax - 10, headH),
                   Qt::AlignVCenter | Qt::AlignRight, "SGST");
    }
    p.drawText(QRect(xAmt, y, wAmt - 10, headH),
               Qt::AlignVCenter | Qt::AlignRight, "Amount");
    y += headH;

    const QJsonArray arr = QJsonDocument::fromJson(itemsJson.toUtf8()).array();
    const double half = gstPercent / 2.0;
    int idx = 1;

    for(const QJsonValue &v : arr) {
        const QJsonObject o = v.toObject();
        const double amount = o["amount"].toDouble();
        const double rate = o["rate"].toDouble();
        const double itemCgst = interState ? 0 : amount * half / 100.0;
        const double itemIgst = interState ? amount * gstPercent / 100.0 : 0;
        const int rowH = 92;

        p.setFont(normal); p.setPen(dark);
        p.drawText(QRect(0, y, wNum, rowH),
                   Qt::AlignHCenter | Qt::AlignTop, QString::number(idx));
        p.drawText(QRect(descStart, y + 6, descEnd - descStart - 10, rowH),
                   Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop,
                   o["description"].toString());
        p.drawText(QRect(xRate, y, wRate, rowH),
                   Qt::AlignTop | Qt::AlignRight, in.toString(rate, 'f', 2));

        if(interState) {
            p.drawText(QRect(xIgst, y, xAmt - xIgst - 10, rowH),
                       Qt::AlignTop | Qt::AlignRight,
                       in.toString(itemIgst, 'f', 2));
            p.setFont(small); p.setPen(gray);
            p.drawText(QRect(xIgst, y + 38, xAmt - xIgst - 10, rowH),
                       Qt::AlignTop | Qt::AlignRight,
                       "(" + in.toString(gstPercent, 'f', 1) + "%)");
        } else {
            p.drawText(QRect(xCgst, y, wTax - 10, rowH),
                       Qt::AlignTop | Qt::AlignRight,
                       in.toString(itemCgst, 'f', 2));
            p.drawText(QRect(xSgst, y, wTax - 10, rowH),
                       Qt::AlignTop | Qt::AlignRight,
                       in.toString(itemCgst, 'f', 2));
            p.setFont(small); p.setPen(gray);
            p.drawText(QRect(xCgst, y + 38, wTax - 10, rowH),
                       Qt::AlignTop | Qt::AlignRight,
                       "(" + in.toString(half, 'f', 1) + "%)");
            p.drawText(QRect(xSgst, y + 38, wTax - 10, rowH),
                       Qt::AlignTop | Qt::AlignRight,
                       "(" + in.toString(half, 'f', 1) + "%)");
        }

        p.setFont(normal); p.setPen(dark);
        p.drawText(QRect(xAmt, y, wAmt - 10, rowH),
                   Qt::AlignTop | Qt::AlignRight, in.toString(amount, 'f', 2));

        y += rowH;
        p.setPen(line);
        p.drawLine(0, y, W, y);
        p.setPen(dark);
        idx++;
    }

    y += 40;

    // Totals (right)
    const int labelX = W / 2;
    const int valX = W - 330;
    auto tot = [&](const QString &label, const QString &val,
                   bool bold, bool bar) {
        if(bar) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#F1ECE4"));
            p.drawRect(QRect(labelX, y, W - labelX, 72));
            p.setBrush(Qt::NoBrush);
        }
        p.setFont(bold ? totalBold : normal);
        p.setPen(bold ? dark : gray);
        p.drawText(QRect(labelX + 20, y, valX - labelX - 40, bar ? 72 : 52),
                   Qt::AlignLeft | Qt::AlignVCenter, label);
        p.setPen(dark);
        p.drawText(QRect(valX, y, 320, bar ? 72 : 52),
                   Qt::AlignRight | Qt::AlignVCenter, val);
        y += bar ? 78 : 56;
    };

    tot("Sub Total", money(subtotal), false, false);
    if(interState) {
        tot(QString("IGST (%1%)").arg(in.toString(gstPercent, 'f', 1)),
            money(igst), false, false);
    } else {
        tot(QString("CGST (%1%)").arg(in.toString(half, 'f', 1)),
            money(cgst), false, false);
        tot(QString("SGST (%1%)").arg(in.toString(half, 'f', 1)),
            money(sgst), false, false);
    }
    tot("Total", money(grandTotal), true, false);
    tot("Balance Due", money(balanceDue), true, true);

    if(!notes.trimmed().isEmpty()) {
        y += 40;
        p.setPen(dark); p.setFont(hdr);
        p.drawText(QRect(0, y, W, 40), Qt::AlignLeft, "Notes");
        y += 48;
        p.setFont(small); p.setPen(gray);
        p.drawText(QRect(0, y, W, 300), Qt::AlignLeft | Qt::AlignTop, notes);
    }

    p.end();

    QSqlQuery up;
    up.prepare("UPDATE invoices SET pdf_path = :p WHERE id = :id");
    up.bindValue(":p", path);
    up.bindValue(":id", invoiceId);
    up.exec();

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void InvoicesPage::newInvoice() {

    QSqlQuery clientQuery("SELECT id, name FROM clients ORDER BY name");
    QList<QPair<int, QString>> clients;
    while(clientQuery.next())
        clients.append({clientQuery.value(0).toInt(),
                        clientQuery.value(1).toString()});

    if(clients.isEmpty()) {
        QMessageBox::information(this, "New Invoice",
            "Add a client first, then create an invoice.");
        return;
    }

    QSettings settings("InvoicePro", "InvoicePro");

    QDialog dialog(this);
    dialog.setWindowTitle("New Invoice");
    dialog.setMinimumWidth(580);

    QVBoxLayout *outer = new QVBoxLayout(&dialog);
    QFormLayout *form = new QFormLayout();

    QComboBox *client = new QComboBox(&dialog);
    for(const auto &c : clients)
        client->addItem(c.second, c.first);

    QDateEdit *invoiceDate = new QDateEdit(QDate::currentDate(), &dialog);
    invoiceDate->setCalendarPopup(true);
    invoiceDate->setDisplayFormat("dd MMM yyyy");

    QDateEdit *dueDate =
        new QDateEdit(QDate::currentDate().addDays(30), &dialog);
    dueDate->setCalendarPopup(true);
    dueDate->setDisplayFormat("dd MMM yyyy");

    QComboBox *gst = new QComboBox(&dialog);
    gst->addItems({"0", "5", "12", "18", "28"});
    gst->setCurrentText(
        settings.value("invoice/defaultGstRate", "18").toString());

    QComboBox *supply = new QComboBox(&dialog);
    supply->addItems({"Intra-state (CGST + SGST)", "Inter-state (IGST)"});

    QLineEdit *pos = new QLineEdit(&dialog);
    pos->setText(settings.value("company/state", "").toString());

    QComboBox *status = new QComboBox(&dialog);
    status->addItems({"Pending", "Paid", "Overdue"});

    form->addRow("Client", client);
    form->addRow("Invoice Date", invoiceDate);
    form->addRow("Due Date", dueDate);
    form->addRow("GST %", gst);
    form->addRow("Supply Type", supply);
    form->addRow("Place of Supply", pos);
    form->addRow("Status", status);
    outer->addLayout(form);

    QLabel *itemsLabel = new QLabel("Line Items");
    itemsLabel->setStyleSheet("font-weight: bold;");
    outer->addWidget(itemsLabel);

    QTableWidget *items = new QTableWidget(0, 4, &dialog);
    items->setHorizontalHeaderLabels({"Description", "Qty", "Rate", "Amount"});
    items->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    items->setMinimumHeight(160);
    outer->addWidget(items);

    QHBoxLayout *lineButtons = new QHBoxLayout();
    QPushButton *addLineBtn = new QPushButton("+ Add Line");
    QPushButton *removeLineBtn = new QPushButton("Remove Selected");
    lineButtons->addWidget(addLineBtn);
    lineButtons->addWidget(removeLineBtn);
    lineButtons->addStretch();
    outer->addLayout(lineButtons);

    QLabel *totals = new QLabel();
    totals->setAlignment(Qt::AlignRight);
    totals->setStyleSheet("font-size: 15px;");
    outer->addWidget(totals);

    bool updating = false;

    auto recalc = [&]() {
        if(updating) return;
        updating = true;

        double subtotal = 0;
        for(int r = 0; r < items->rowCount(); r++) {
            const double qty =
                items->item(r, 1) ? items->item(r, 1)->text().toDouble() : 0;
            const double rate =
                items->item(r, 2) ? items->item(r, 2)->text().toDouble() : 0;
            const double amount = qty * rate;
            subtotal += amount;

            QTableWidgetItem *amountItem = items->item(r, 3);
            if(!amountItem) {
                amountItem = new QTableWidgetItem();
                amountItem->setFlags(amountItem->flags() & ~Qt::ItemIsEditable);
                items->setItem(r, 3, amountItem);
            }
            amountItem->setText(QString::number(amount, 'f', 2));
        }

        const double gp = gst->currentText().toDouble();
        const double gstTotal = subtotal * gp / 100.0;

        QString taxStr;
        if(supply->currentIndex() == 1) {
            taxStr = "IGST (" + QString::number(gp, 'f', 1) + "%): "
                   + rupees(gstTotal);
        } else {
            const double h = gp / 2.0;
            taxStr = "CGST (" + QString::number(h, 'f', 1) + "%): "
                   + rupees(gstTotal / 2) + "    SGST ("
                   + QString::number(h, 'f', 1) + "%): " + rupees(gstTotal / 2);
        }

        totals->setText("Subtotal: " + rupees(subtotal) + "     " + taxStr
                        + "     Grand Total: " + rupees(subtotal + gstTotal));
        updating = false;
    };

    auto addLine = [&]() {
        const int r = items->rowCount();
        items->insertRow(r);
        items->setItem(r, 0, new QTableWidgetItem(""));
        items->setItem(r, 1, new QTableWidgetItem("1"));
        items->setItem(r, 2, new QTableWidgetItem("0"));
        QTableWidgetItem *amt = new QTableWidgetItem("0.00");
        amt->setFlags(amt->flags() & ~Qt::ItemIsEditable);
        items->setItem(r, 3, amt);
        recalc();
    };

    QObject::connect(items, &QTableWidget::itemChanged,
                     &dialog, [&](QTableWidgetItem *) { recalc(); });
    QObject::connect(gst, &QComboBox::currentTextChanged,
                     &dialog, [&](const QString &) { recalc(); });
    QObject::connect(supply, &QComboBox::currentTextChanged,
                     &dialog, [&](const QString &) { recalc(); });
    QObject::connect(addLineBtn, &QPushButton::clicked, &dialog, addLine);
    QObject::connect(removeLineBtn, &QPushButton::clicked, &dialog, [&]() {
        const int r = items->currentRow();
        if(r >= 0) items->removeRow(r);
        recalc();
    });

    addLine();

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    outer->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted,
                     &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected,
                     &dialog, &QDialog::reject);

    if(dialog.exec() != QDialog::Accepted)
        return;

    QJsonArray itemsArray;
    double subtotal = 0;

    for(int r = 0; r < items->rowCount(); r++) {
        const QString desc =
            items->item(r, 0) ? items->item(r, 0)->text().trimmed() : "";
        const double qty =
            items->item(r, 1) ? items->item(r, 1)->text().toDouble() : 0;
        const double rate =
            items->item(r, 2) ? items->item(r, 2)->text().toDouble() : 0;
        const double amount = qty * rate;

        if(desc.isEmpty() && amount == 0)
            continue;

        QJsonObject obj;
        obj["description"] = desc;
        obj["qty"] = qty;
        obj["rate"] = rate;
        obj["amount"] = amount;
        itemsArray.append(obj);
        subtotal += amount;
    }

    if(itemsArray.isEmpty() || subtotal <= 0) {
        QMessageBox::warning(this, "New Invoice",
            "Add at least one line item with an amount.");
        return;
    }

    const double gstPercent = gst->currentText().toDouble();
    const double gstTotal = subtotal * gstPercent / 100.0;

    const bool inter = supply->currentIndex() == 1;
    double cgst = 0, sgst = 0, igst = 0;
    if(inter) {
        igst = gstTotal;
    } else {
        cgst = gstTotal / 2.0;
        sgst = gstTotal / 2.0;
    }
    const double grandTotal = subtotal + gstTotal;

    const QString itemsJson = QString::fromUtf8(
        QJsonDocument(itemsArray).toJson(QJsonDocument::Compact));

    const QString prefix =
        settings.value("invoice/prefix", "INV").toString();

    int count = 0;
    QSqlQuery countQuery("SELECT COUNT(*) FROM invoices");
    if(countQuery.next()) count = countQuery.value(0).toInt();

    const QString invoiceNumber =
        prefix + QString("%1").arg(count + 1, 3, 10, QChar('0'));

    QSqlQuery query;
    query.prepare(
        "INSERT INTO invoices "
        "(invoice_number, client_id, invoice_date, due_date, invoice_items, "
        " subtotal, gst_percent, gst_total, cgst, sgst, igst, grand_total, "
        " place_of_supply, status) "
        "VALUES (:no, :cid, :idate, :ddate, :items, "
        " :sub, :gp, :gt, :cgst, :sgst, :igst, :grand, :pos, :status)");

    query.bindValue(":no", invoiceNumber);
    query.bindValue(":cid", client->currentData().toInt());
    query.bindValue(":idate", invoiceDate->date());
    query.bindValue(":ddate", dueDate->date());
    query.bindValue(":items", itemsJson);
    query.bindValue(":sub", subtotal);
    query.bindValue(":gp", gstPercent);
    query.bindValue(":gt", gstTotal);
    query.bindValue(":cgst", cgst);
    query.bindValue(":sgst", sgst);
    query.bindValue(":igst", igst);
    query.bindValue(":grand", grandTotal);
    query.bindValue(":pos", pos->text().trimmed());
    query.bindValue(":status", status->currentText());

    if(!query.exec()) {
        QMessageBox::critical(this, "New Invoice",
            "Could not save invoice:\n" + query.lastError().text());
        return;
    }

    loadInvoices();
}

void InvoicesPage::filterInvoices(const QString &text) {

    for(int row = 0; row < table->rowCount(); row++) {

        bool match = text.isEmpty();

        for(int col = 0; col < 7 && !match; col++) {

            QTableWidgetItem *item = table->item(row, col);

            if(item && item->text().contains(text, Qt::CaseInsensitive))
                match = true;
        }

        table->setRowHidden(row, !match);
    }
}
