#ifndef INVOICE_H
#define INVOICE_H

#include <QString>
#include <QDate>

struct Invoice {
    int id = -1;
    QString invoiceNo;
    int vendorId = -1;
    QString vendorName;
    QDate invoiceDate;
    QDate dueDate;
    double subtotal = 0.0;
    double gstTotal = 0.0;
    double grandTotal = 0.0;
    QString status = "Pending";
    QString notes;
    QString createdAt;
};

#endif // INVOICE_H
