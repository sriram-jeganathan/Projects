#ifndef INVOICEITEM_H
#define INVOICEITEM_H

#include <QString>

struct InvoiceItem {
    int id = -1;
    int invoiceId = -1;
    QString itemName;
    int quantity = 1;
    double price = 0.0;
    double gstPercent = 0.0;
    double cgst = 0.0;
    double sgst = 0.0;
    double igst = 0.0;
    double total = 0.0;
};

#endif // INVOICEITEM_H
