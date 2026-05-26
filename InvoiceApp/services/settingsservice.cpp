#include "services/settingsservice.h"
#include "core/database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

bool SettingsService::loadSettings(CompanySettings* settings, QString* errorOut)
{
    if (!settings)
        return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT company_name, company_gst, address, phone, email, logo_path FROM company_settings WHERE id = 1");
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }

    if (q.next()) {
        settings->companyName = q.value(0).toString();
        settings->companyGst = q.value(1).toString();
        settings->address = q.value(2).toString();
        settings->phone = q.value(3).toString();
        settings->email = q.value(4).toString();
        settings->logoPath = q.value(5).toString();
        return true;
    }

    q.prepare("INSERT INTO company_settings(id, company_name, company_gst, address, phone, email, logo_path)"
              " VALUES(1, '', '', '', '', '', '')");
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }

    return true;
}

bool SettingsService::saveSettings(const CompanySettings& settings, QString* errorOut)
{
    QSqlQuery q(Database::instance().db());
    q.prepare("UPDATE company_settings SET company_name = :company_name, company_gst = :company_gst,"
              " address = :address, phone = :phone, email = :email, logo_path = :logo_path WHERE id = 1");
    q.bindValue(":company_name", settings.companyName);
    q.bindValue(":company_gst", settings.companyGst);
    q.bindValue(":address", settings.address);
    q.bindValue(":phone", settings.phone);
    q.bindValue(":email", settings.email);
    q.bindValue(":logo_path", settings.logoPath);

    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }
    return true;
}
