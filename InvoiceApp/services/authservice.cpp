#include "authservice.h"
#include "core/database.h"

#include <QCryptographicHash>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QRandomGenerator>

QString AuthService::makeSalt()
{
    // Generate 16 random bytes
    QByteArray bytes(16, 0);

    for (int i = 0; i < bytes.size(); ++i) {
        bytes[i] =
            static_cast<char>(QRandomGenerator::global()->bounded(256));
    }

    return QString(bytes.toHex());
}

QString AuthService::hashWithSalt(const QString& password,
                                  const QString& salt)
{
    QByteArray combined = (salt + password).toUtf8();

    QByteArray hash =
        QCryptographicHash::hash(combined,
                                 QCryptographicHash::Sha256);

    return QString(hash.toHex());
}

bool AuthService::signup(const QString& username,
                         const QString& password,
                         QString* errorOut,
                         User* outUser)
{
    if (username.size() < 3) {
        *errorOut = "Username too short";
        return false;
    }

    if (password.size() < 6) {
        *errorOut = "Password too short";
        return false;
    }

    QSqlQuery q(Database::instance().db());

    // Check if username already exists
    q.prepare("SELECT id FROM users WHERE username = :u");
    q.bindValue(":u", username);
    q.exec();

    if (q.next()) {
        *errorOut = "Username already exists";
        return false;
    }

    QString salt = makeSalt();
    QString hash = hashWithSalt(password, salt);

    QString now =
        QDateTime::currentDateTime().toString(Qt::ISODate);

    q.prepare(
        "INSERT INTO users("
        "username, password_hash, salt, created_at"
        ") "
        "VALUES(:u, :h, :s, :t)"
    );

    q.bindValue(":u", username);
    q.bindValue(":h", hash);
    q.bindValue(":s", salt);
    q.bindValue(":t", now);

    if (!q.exec()) {
        *errorOut = q.lastError().text();
        return false;
    }

    if (outUser) {
        outUser->id = q.lastInsertId().toInt();
        outUser->username = username;
        outUser->passwordHash = hash;
        outUser->salt = salt;
        outUser->createdAt = now;
    }

    return true;
}

bool AuthService::login(const QString& username,
                        const QString& password,
                        QString* errorOut,
                        User* outUser)
{
    QSqlQuery q(Database::instance().db());

    q.prepare(
        "SELECT id, password_hash, salt, created_at "
        "FROM users "
        "WHERE username = :u"
    );

    q.bindValue(":u", username);
    q.exec();

    if (!q.next()) {
        *errorOut = "User not found";
        return false;
    }

    QString storedHash = q.value(1).toString();
    QString storedSalt = q.value(2).toString();

    if (hashWithSalt(password, storedSalt) != storedHash) {
        *errorOut = "Wrong password";
        return false;
    }

    outUser->id = q.value(0).toInt();
    outUser->username = username;
    outUser->passwordHash = storedHash;
    outUser->salt = storedSalt;
    outUser->createdAt = q.value(3).toString();

    return true;
}
