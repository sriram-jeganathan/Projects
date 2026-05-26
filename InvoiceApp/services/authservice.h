#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <QString>
#include "models/user.h"

class AuthService {
public:
  // Returns true and fills outUser on success.
  bool signup(const QString& username, const QString& password,
  QString* errorOut, User* outUser = nullptr);
  bool login(const QString& username, const QString& password,
  QString* errorOut, User* outUser);
private:
  static QString makeSalt();
  static QString hashWithSalt(const QString& password, const QString& salt);
};

#endif
