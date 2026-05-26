#ifndef USER_H
#define USER_H

#include <QString>

struct User {
  int id = -1;
  QString username;
  QString passwordHash;
  QString salt;
  QString createdAt;
};

#endif
