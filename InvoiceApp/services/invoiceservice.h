#ifndef INVOICESERVICE_H
#define INVOICESERVICE_H

#include <QString>
#include <QVector>
#include "models/invoice.h"
#include "models/invoiceitem.h"

class InvoiceService
{
public:
    static QVector<Invoice> listInvoices(const QString& filter = QString());
    static bool saveInvoice(Invoice& invoice, const QVector<InvoiceItem>& items, QString* errorOut);
    static bool loadInvoiceItems(int invoiceId, QVector<InvoiceItem>* items, QString* errorOut);
    static bool deleteInvoice(int invoiceId, QString* errorOut);
    static bool exportInvoicePdf(const Invoice& invoice,
                                 const QVector<InvoiceItem>& items,
                                 const QString& filePath,
                                 const QString& companyName,
                                 const QString& companyGst,
                                 const QString& companyAddress,
                                 const QString& companyPhone,
                                 const QString& companyEmail,
                                 QString* errorOut);
};

#endif // INVOICESERVICE_H
