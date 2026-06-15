#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QWidget;
class QFrame;
class QPushButton;
class QStackedWidget;

class DashboardPage;
class ClientsPage;
class InvoicesPage;
class ReportsPage;
class SettingsPage;
class LoginOverlay;

class MainWindow : public QMainWindow {

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);

private:

    QWidget *centralWidget;

    QFrame *sidebar;

    QPushButton *dashboardBtn;
    QPushButton *clientsBtn;
    QPushButton *invoicesBtn;
    QPushButton *reportsBtn;
    QPushButton *settingsBtn;
    QPushButton *logoutBtn;

    DashboardPage *dashboardPage;
    ClientsPage *clientsPage;
    InvoicesPage *invoicesPage;
    ReportsPage *reportsPage;
    SettingsPage *settingsPage;

    LoginOverlay *loginOverlay;

    QStackedWidget *stack;

    void resizeEvent(QResizeEvent *event) override;

    void setupUI();
};

#endif
