#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "models/user.h"

class SessionManager {
public:
  static SessionManager& instance();
  void setUser (const User& u);
  void clear ();
  bool isLoggedIn () const;
  const User& currentUser () const;
private:
  SessionManager () = default;
  User m_user;
  bool m_loggedIn = false;
};

#endif
