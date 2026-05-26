#include "services/invoiceservice.h"
#include "core/database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QPdfWriter>
#include <QPainter>
#include <QTextOption>

QVector<Invoice> InvoiceService::listInvoices(const QString& filter)
{
    QVector<Invoice> result;
    QSqlQuery q(Database::instance().db());
    QString sql = "SELECT invoices.id, invoice_no, vendor_id, invoice_date, due_date, subtotal, gst_total, grand_total, status, notes, created_at, vendors.name"
                  " FROM invoices"
                  " LEFT JOIN vendors ON invoices.vendor_id = vendors.id";
    if (!filter.isEmpty()) {
        sql += " WHERE invoice_no LIKE :filter OR vendors.name LIKE :filter";
    }
    sql += " ORDER BY invoice_date DESC";
    q.prepare(sql);
    if (!filter.isEmpty()) {
        q.bindValue(":filter", "%" + filter + "%");
    }
    if (!q.exec()) {
        return result;
    }

    while (q.next()) {
        Invoice invoice;
        invoice.id = q.value(0).toInt();
        invoice.invoiceNo = q.value(1).toString();
        invoice.vendorId = q.value(2).toInt();
        invoice.invoiceDate = QDate::fromString(q.value(3).toString(), Qt::ISODate);
        invoice.dueDate = QDate::fromString(q.value(4).toString(), Qt::ISODate);
        invoice.subtotal = q.value(5).toDouble();
        invoice.gstTotal = q.value(6).toDouble();
        invoice.grandTotal = q.value(7).toDouble();
        invoice.status = q.value(8).toString();
        invoice.notes = q.value(9).toString();
        invoice.createdAt = q.value(10).toString();
        invoice.vendorName = q.value(11).toString();
        result.append(invoice);
    }
    return result;
}

static bool upsertInvoiceItems(int invoiceId, const QVector<InvoiceItem>& items, QString* errorOut)
{
    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM invoice_items WHERE invoice_id = :invoice_id");
    q.bindValue(":invoice_id", invoiceId);
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }

    q.prepare("INSERT INTO invoice_items(invoice_id, item_name, quantity, price, gst_percent, cgst, sgst, igst, total)"
              " VALUES(:invoice_id, :item_name, :quantity, :price, :gst_percent, :cgst, :sgst, :igst, :total)");

    for (const auto& item : items) {
        q.bindValue(":invoice_id", invoiceId);
        q.bindValue(":item_name", item.itemName);
        q.bindValue(":quantity", item.quantity);
        q.bindValue(":price", item.price);
        q.bindValue(":gst_percent", item.gstPercent);
        q.bindValue(":cgst", item.cgst);
        q.bindValue(":sgst", item.sgst);
        q.bindValue(":igst", item.igst);
        q.bindValue(":total", item.total);
        if (!q.exec()) {
            if (errorOut) *errorOut = q.lastError().text();
            return false;
        }
    }
    return true;
}

bool InvoiceService::saveInvoice(Invoice& invoice, const QVector<InvoiceItem>& items, QString* errorOut)
{
    if (invoice.invoiceNo.trimmed().isEmpty()) {
        invoice.invoiceNo = QString("INV-%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    }

    QSqlQuery q(Database::instance().db());
    if (invoice.id == -1 && !invoice.invoiceNo.trimmed().isEmpty()) {
        QSqlQuery check(Database::instance().db());
        check.prepare("SELECT id FROM invoices WHERE invoice_no = :invoice_no");
        check.bindValue(":invoice_no", invoice.invoiceNo.trimmed());
        if (check.exec() && check.next()) {
            invoice.id = check.value(0).toInt();
        }
    }

    if (invoice.id == -1) {
        q.prepare("INSERT INTO invoices(invoice_no, vendor_id, invoice_date, due_date, subtotal, gst_total, grand_total, status, notes, created_at)"
                  " VALUES(:invoice_no, :vendor_id, :invoice_date, :due_date, :subtotal, :gst_total, :grand_total, :status, :notes, :created_at)");
        q.bindValue(":created_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    } else {
        q.prepare("UPDATE invoices SET invoice_no = :invoice_no, vendor_id = :vendor_id, invoice_date = :invoice_date, due_date = :due_date,"
                  " subtotal = :subtotal, gst_total = :gst_total, grand_total = :grand_total, status = :status, notes = :notes"
                  " WHERE id = :id");
        q.bindValue(":id", invoice.id);
    }

    q.bindValue(":invoice_no", invoice.invoiceNo);
    q.bindValue(":vendor_id", invoice.vendorId);
    q.bindValue(":invoice_date", invoice.invoiceDate.toString(Qt::ISODate));
    q.bindValue(":due_date", invoice.dueDate.toString(Qt::ISODate));
    q.bindValue(":subtotal", invoice.subtotal);
    q.bindValue(":gst_total", invoice.gstTotal);
    q.bindValue(":grand_total", invoice.grandTotal);
    q.bindValue(":status", invoice.status);
    q.bindValue(":notes", invoice.notes);

    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }

    if (invoice.id == -1) {
        invoice.id = q.lastInsertId().toInt();
    }

    return upsertInvoiceItems(invoice.id, items, errorOut);
}

bool InvoiceService::loadInvoiceItems(int invoiceId, QVector<InvoiceItem>* items, QString* errorOut)
{
    if (!items)
        return false;

    items->clear();
    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT id, invoice_id, item_name, quantity, price, gst_percent, cgst, sgst, igst, total"
              " FROM invoice_items WHERE invoice_id = :invoice_id ORDER BY id");
    q.bindValue(":invoice_id", invoiceId);
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }

    while (q.next()) {
        InvoiceItem item;
        item.id = q.value(0).toInt();
        item.invoiceId = q.value(1).toInt();
        item.itemName = q.value(2).toString();
        item.quantity = q.value(3).toInt();
        item.price = q.value(4).toDouble();
        item.gstPercent = q.value(5).toDouble();
        item.cgst = q.value(6).toDouble();
        item.sgst = q.value(7).toDouble();
        item.igst = q.value(8).toDouble();
        item.total = q.value(9).toDouble();
        items->append(item);
    }
    return true;
}

bool InvoiceService::deleteInvoice(int invoiceId, QString* errorOut)
{
    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM invoices WHERE id = :id");
    q.bindValue(":id", invoiceId);
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }
    return true;
}

bool InvoiceService::exportInvoicePdf(const Invoice& invoice,
                                     const QVector<InvoiceItem>& items,
                                     const QString& filePath,
                                     const QString& companyName,
                                     const QString& companyGst,
                                     const QString& companyAddress,
                                     const QString& companyPhone,
                                     const QString& companyEmail,
                                     QString* errorOut)
{
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    writer.setPageMargins(QMarginsF(30, 30, 30, 30));

    QPainter painter(&writer);
    if (!painter.isActive()) {
        if (errorOut) *errorOut = "Unable to initialize PDF writer.";
        return false;
    }
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const int pageWidth = writer.width();
    const int leftMargin = 40;
    const int rightMargin = 40;
    const int contentWidth = pageWidth - leftMargin - rightMargin;

    QFont titleFont("Helvetica", 18, QFont::Bold);
    QFont labelFont("Helvetica", 10);
    QFont headerFont("Helvetica", 11, QFont::Bold);
    QFont tableFont("Helvetica", 10);

    int y = 40;
    painter.setFont(titleFont);
    painter.drawText(leftMargin, y, companyName.isEmpty() ? "InvoiceApp" : companyName);

    y += 30;
    painter.setFont(labelFont);
    painter.drawText(leftMargin, y, QString("GST: %1").arg(companyGst));
    painter.drawText(leftMargin, y + 18, QString("Phone: %1").arg(companyPhone));
    painter.drawText(leftMargin, y + 36, QString("Email: %1").arg(companyEmail));

    QRectF addressRect(leftMargin, y + 54, contentWidth / 2.0, 80);
    painter.drawText(addressRect, Qt::TextWordWrap, companyAddress);

    const int rightColumnX = leftMargin + contentWidth / 2 + 20;
    painter.drawText(rightColumnX, y, QString("Invoice #: %1").arg(invoice.invoiceNo));
    painter.drawText(rightColumnX, y + 18, QString("Date: %1").arg(invoice.invoiceDate.toString(Qt::ISODate)));
    painter.drawText(rightColumnX, y + 36, QString("Due Date: %1").arg(invoice.dueDate.toString(Qt::ISODate)));
    painter.drawText(rightColumnX, y + 54, QString("Customer: %1").arg(invoice.vendorName));

    y += 110;
    painter.setFont(headerFont);
    painter.drawLine(leftMargin, y, pageWidth - rightMargin, y);
    y += 20;
    painter.drawText(leftMargin, y, "Item");
    painter.drawText(leftMargin + 260, y, "Qty");
    painter.drawText(leftMargin + 320, y, "Price");
    painter.drawText(leftMargin + 400, y, "GST %");
    painter.drawText(leftMargin + 470, y, "Total");
    y += 10;
    painter.drawLine(leftMargin, y, pageWidth - rightMargin, y);
    y += 20;

    painter.setFont(tableFont);
    const int lineHeight = 18;
    const int pageBottom = writer.height() - 60;

    for (const auto& item : items) {
        if (y + lineHeight > pageBottom) {
            writer.newPage();
            y = 60;
            painter.setFont(headerFont);
            painter.drawLine(leftMargin, y, pageWidth - rightMargin, y);
            y += 20;
            painter.drawText(leftMargin, y, "Item");
            painter.drawText(leftMargin + 260, y, "Qty");
            painter.drawText(leftMargin + 320, y, "Price");
            painter.drawText(leftMargin + 400, y, "GST %");
            painter.drawText(leftMargin + 470, y, "Total");
            y += 10;
            painter.drawLine(leftMargin, y, pageWidth - rightMargin, y);
            y += 20;
            painter.setFont(tableFont);
        }

        QRectF itemRect(leftMargin, y - 12, 240, lineHeight);
        painter.drawText(itemRect, Qt::TextSingleLine | Qt::AlignLeft | Qt::TextWordWrap, item.itemName);
        painter.drawText(leftMargin + 260, y, QString::number(item.quantity));
        painter.drawText(leftMargin + 320, y, QString::number(item.price, 'f', 2));
        painter.drawText(leftMargin + 400, y, QString::number(item.gstPercent, 'f', 2));
        painter.drawText(leftMargin + 470, y, QString::number(item.total, 'f', 2));
        y += lineHeight;
    }

    if (y + 80 > pageBottom) {
        writer.newPage();
        y = 60;
    }

    y += 20;
    painter.drawLine(leftMargin, y, pageWidth - rightMargin, y);
    y += 24;
    painter.drawText(rightColumnX, y, QString("Subtotal: %1").arg(invoice.subtotal, 0, 'f', 2));
    y += 20;
    painter.drawText(rightColumnX, y, QString("GST: %1").arg(invoice.gstTotal, 0, 'f', 2));
    y += 20;
    painter.drawText(rightColumnX, y, QString("Total: %1").arg(invoice.grandTotal, 0, 'f', 2));

    painter.end();
    return true;
}
