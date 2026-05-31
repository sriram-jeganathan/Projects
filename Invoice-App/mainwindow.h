#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QFrame>

class DashboardPage;
class ClientsPage;
class InvoicesPage;
class ReportsPage;
class SettingsPage;
class LoginOverlay;

class MainWindow : public QMainWindow {
Q_OBJECT
private:
  QWidget *central;

  QFrame *sidebar;

  QPushButton *dashboardBtn;
  QPushButton *clientsBtn;
  QPushButton *invoicesBtn;
  QPushButton *reportsBtn;
  QPushButton *settingsBtn;
  QPushButton *logoutBtn;

  QStackedWidget *stack;

  DashboardPage *dashboardPage;
  ClientsPage *clientsPage;
  InvoicesPage *invoicesPage;
  ReportsPage *reportsPage;
  SettingsPage *settingsPage;

  LoginOverlay *loginOverlay;

  void setupUI();
  void setupSidebar();
  void setupPages();
  void connectSidebar();
public:
  MainWindow(QWidget *parent = nullptr);
};
