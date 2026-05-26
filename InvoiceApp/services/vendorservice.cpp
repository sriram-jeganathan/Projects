#include "services/vendorservice.h"
#include "core/database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

QVector<Vendor> VendorService::listVendors(const QString& filter)
{
    QVector<Vendor> result;
    QSqlQuery q(Database::instance().db());
    QString sql = "SELECT id, name, gst, phone, email, address, tax_details, created_at"
                  " FROM vendors";
    if (!filter.isEmpty()) {
        sql += " WHERE name LIKE :filter OR gst LIKE :filter OR phone LIKE :filter OR email LIKE :filter";
    }
    q.prepare(sql);
    if (!filter.isEmpty()) {
        q.bindValue(":filter", "%" + filter + "%");
    }

    if (!q.exec()) {
        return result;
    }

    while (q.next()) {
        Vendor vendor;
        vendor.id = q.value(0).toInt();
        vendor.name = q.value(1).toString();
        vendor.gst = q.value(2).toString();
        vendor.phone = q.value(3).toString();
        vendor.email = q.value(4).toString();
        vendor.address = q.value(5).toString();
        vendor.taxDetails = q.value(6).toString();
        vendor.createdAt = q.value(7).toString();
        result.append(vendor);
    }
    return result;
}

QVector<QString> VendorService::vendorNames()
{
    QVector<QString> result;
    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT name FROM vendors ORDER BY name");
    if (!q.exec()) {
        return result;
    }

    while (q.next()) {
        result.append(q.value(0).toString());
    }
    return result;
}

bool VendorService::addOrUpdateVendor(const Vendor& vendor, QString* errorOut, Vendor* outVendor)
{
    QSqlQuery q(Database::instance().db());
    if (vendor.id == -1) {
        q.prepare("INSERT INTO vendors(name, gst, phone, email, address, tax_details, created_at)"
                  " VALUES(:name, :gst, :phone, :email, :address, :tax_details, :created_at)");
        q.bindValue(":created_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    } else {
        q.prepare("UPDATE vendors SET name = :name, gst = :gst, phone = :phone, email = :email,"
                  " address = :address, tax_details = :tax_details WHERE id = :id");
        q.bindValue(":id", vendor.id);
    }

    q.bindValue(":name", vendor.name);
    q.bindValue(":gst", vendor.gst);
    q.bindValue(":phone", vendor.phone);
    q.bindValue(":email", vendor.email);
    q.bindValue(":address", vendor.address);
    q.bindValue(":tax_details", vendor.taxDetails);

    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }

    if (outVendor) {
        *outVendor = vendor;
        if (vendor.id == -1) {
            outVendor->id = q.lastInsertId().toInt();
            outVendor->createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
        }
    }

    return true;
}

bool VendorService::removeVendor(int vendorId, QString* errorOut)
{
    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM vendors WHERE id = :id");
    q.bindValue(":id", vendorId);
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }
    return true;
}

bool VendorService::findVendorByName(const QString& name, Vendor* outVendor)
{
    if (!outVendor)
        return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT id, name, gst, phone, email, address, tax_details, created_at"
              " FROM vendors WHERE name = :name LIMIT 1");
    q.bindValue(":name", name);
    if (!q.exec() || !q.next()) {
        return false;
    }

    outVendor->id = q.value(0).toInt();
    outVendor->name = q.value(1).toString();
    outVendor->gst = q.value(2).toString();
    outVendor->phone = q.value(3).toString();
    outVendor->email = q.value(4).toString();
    outVendor->address = q.value(5).toString();
    outVendor->taxDetails = q.value(6).toString();
    outVendor->createdAt = q.value(7).toString();
    return true;
}
