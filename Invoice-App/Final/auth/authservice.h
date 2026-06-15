#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <QString>

class AuthService {
public:
    bool login( const QString &username, const QString &password );

private:
    QString hashPassword ( const QString &password );
};
#endif