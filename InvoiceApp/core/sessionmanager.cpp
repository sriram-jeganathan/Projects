#include "sessionmanager.h"

SessionManager& SessionManager::instance() {
  static SessionManager s;
  return s;
}

void SessionManager::setUser (const User& u) {
  m_user = u; 
  m_loggedIn = true; 
}

void SessionManager::clear() { 
  m_user = User(); 
  m_loggedIn = false; 
}

bool SessionManager::isLoggedIn() const { 
  return m_loggedIn; 
}

const User& SessionManager::currentUser() const { 
  return m_user; 
}
