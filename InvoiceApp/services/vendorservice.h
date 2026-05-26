#ifndef VENDORSERVICE_H
#define VENDORSERVICE_H

#include <QString>
#include <QVector>
#include "models/vendor.h"

class VendorService
{
public:
    static QVector<Vendor> listVendors(const QString& filter = QString());
    static QVector<QString> vendorNames();
    static bool addOrUpdateVendor(const Vendor& vendor, QString* errorOut, Vendor* outVendor = nullptr);
    static bool removeVendor(int vendorId, QString* errorOut);
    static bool findVendorByName(const QString& name, Vendor* outVendor);
};

#endif // VENDORSERVICE_H
