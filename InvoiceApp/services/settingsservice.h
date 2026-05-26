#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QString>

struct CompanySettings {
    QString companyName;
    QString companyGst;
    QString address;
    QString phone;
    QString email;
    QString logoPath;
};

class SettingsService
{
public:
    static bool loadSettings(CompanySettings* settings, QString* errorOut);
    static bool saveSettings(const CompanySettings& settings, QString* errorOut);
};

#endif // SETTINGSSERVICE_H
