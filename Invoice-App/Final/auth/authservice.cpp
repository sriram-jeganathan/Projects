#include "authservice.h"

#include <QSqlQuery>

bool AuthService::login(
    const QString &username,
    const QString &password
) {

    QSqlQuery query;

    query.prepare(
        "SELECT * "
        "FROM users "
        "WHERE username = :username "
        "AND password_hash = :password"
    );

    query.bindValue(
        ":username",
        username
    );

    query.bindValue(
        ":password",
        password
    );

    if(!query.exec()) {

        return false;
    }

    return query.next();
}
