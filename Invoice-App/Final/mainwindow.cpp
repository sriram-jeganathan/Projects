#include "mainwindow.h"

#include "pages/dashboardpage.h"
#include "pages/clientspage.h"
#include "pages/invoicespage.h"
#include "pages/reportspage.h"
#include "pages/settingspage.h"
#include "auth/loginoverlay.h"

#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QList>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    setupUI();
}

void MainWindow::setupUI() {

    resize(1550, 900);

    /*
     * CENTRAL WIDGET
     */

    centralWidget =
        new QWidget();

    setCentralWidget(
        centralWidget
    );

    centralWidget->setStyleSheet(R"(

        background-color: #EDF2FA;

    )");

    /*
     * MAIN LAYOUT
     */

    QHBoxLayout *mainLayout =
        new QHBoxLayout(centralWidget);

    mainLayout->setContentsMargins(
        0,
        0,
        0,
        0
    );

    mainLayout->setSpacing(0);

    /*
     * SIDEBAR
     */

    sidebar =
        new QFrame();

    sidebar->setFixedWidth(270);

    sidebar->setStyleSheet(R"(

        background-color: #1E3A8A;

    )");

    QVBoxLayout *sidebarLayout =
        new QVBoxLayout(sidebar);

    sidebarLayout->setContentsMargins(
        18,
        30,
        18,
        20
    );

    /*
     * LOGO
     */

    QLabel *logo =
        new QLabel("InvoicePro");

    logo->setStyleSheet(R"(

        color: white;
        font-size: 42px;
        font-weight: bold;
        padding-bottom: 20px;

    )");

    /*
     * BUTTONS
     */

    dashboardBtn =
        new QPushButton("Dashboard");

    clientsBtn =
        new QPushButton("Clients");

    invoicesBtn =
        new QPushButton("Invoices");

    reportsBtn =
        new QPushButton("Reports");

    settingsBtn =
        new QPushButton("Settings");

    logoutBtn =
        new QPushButton("Logout");

    /*
     * NAV BUTTON STYLES
     */

    const QString inactiveStyle = R"(

        QPushButton {
            background-color: transparent;
            color: white;
            border: none;
            border-radius: 18px;
            text-align: left;
            padding-left: 22px;
            font-size: 17px;
            font-weight: 600;
        }

        QPushButton:hover {
            background-color: rgba(255,255,255,0.10);
        }

    )";

    const QString activeStyle = R"(

        QPushButton {
            background-color: rgba(255,255,255,0.14);
            color: white;
            border: none;
            border-radius: 18px;
            text-align: left;
            padding-left: 22px;
            font-size: 17px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: rgba(255,255,255,0.18);
        }

    )";

    QList<QPushButton*> navButtons = {

        dashboardBtn,
        clientsBtn,
        invoicesBtn,
        reportsBtn,
        settingsBtn
    };

    for(QPushButton *btn : navButtons) {

        btn->setFixedHeight(58);

        btn->setCursor(
            Qt::PointingHandCursor
        );

        btn->setStyleSheet(inactiveStyle);
    }

    logoutBtn->setFixedHeight(58);

    logoutBtn->setCursor(
        Qt::PointingHandCursor
    );

    logoutBtn->setStyleSheet(inactiveStyle);

    // Dashboard is the landing page
    dashboardBtn->setStyleSheet(activeStyle);

    /*
     * SIDEBAR LAYOUT
     */

    sidebarLayout->addWidget(logo);

    sidebarLayout->addSpacing(25);

    sidebarLayout->addWidget(dashboardBtn);

    sidebarLayout->addWidget(clientsBtn);

    sidebarLayout->addWidget(invoicesBtn);

    sidebarLayout->addWidget(reportsBtn);

    sidebarLayout->addWidget(settingsBtn);

    sidebarLayout->addStretch();

    sidebarLayout->addWidget(logoutBtn);

    /*
     * STACKED PAGES
     */

    stack =
        new QStackedWidget();

    dashboardPage =
        new DashboardPage();

    clientsPage =
        new ClientsPage();

    invoicesPage =
        new InvoicesPage();

    reportsPage =
        new ReportsPage();

    settingsPage =
        new SettingsPage();

    stack->addWidget(dashboardPage);

    stack->addWidget(clientsPage);

    stack->addWidget(invoicesPage);

    stack->addWidget(reportsPage);

    stack->addWidget(settingsPage);

    /*
     * MAIN LAYOUT
     */

    mainLayout->addWidget(sidebar);

    mainLayout->addWidget(stack);

    /*
     * LOGIN OVERLAY
     */

    loginOverlay =
        new LoginOverlay(this);

    loginOverlay->setGeometry(
        this->rect()
    );

    loginOverlay->show();

    loginOverlay->raise();

    /*
     * ACTIVE-STATE HELPER
     *
     * Resets every nav button to the inactive style,
     * then highlights the one that was clicked.
     */

    auto setActive =
        [=](QPushButton *active) {

        for(QPushButton *btn : navButtons) {

            btn->setStyleSheet(inactiveStyle);
        }

        active->setStyleSheet(activeStyle);
    };

    /*
     * BUTTON CONNECTIONS
     */

    connect(
        dashboardBtn,
        &QPushButton::clicked,
        [=]() {

            stack->setCurrentWidget(dashboardPage);
            setActive(dashboardBtn);
        }
    );

    connect(
        clientsBtn,
        &QPushButton::clicked,
        [=]() {

            stack->setCurrentWidget(clientsPage);
            setActive(clientsBtn);
        }
    );

    connect(
        invoicesBtn,
        &QPushButton::clicked,
        [=]() {

            stack->setCurrentWidget(invoicesPage);
            setActive(invoicesBtn);
        }
    );

    connect(
        reportsBtn,
        &QPushButton::clicked,
        [=]() {

            stack->setCurrentWidget(reportsPage);
            setActive(reportsBtn);
        }
    );

    connect(
        settingsBtn,
        &QPushButton::clicked,
        [=]() {

            stack->setCurrentWidget(settingsPage);
            setActive(settingsBtn);
        }
    );

    connect(
        dashboardPage,
        &DashboardPage::newInvoiceRequested,
        [=]() {

            // Jump to Invoices and open the New Invoice form.
            stack->setCurrentWidget(invoicesPage);
            setActive(invoicesBtn);
            invoicesPage->newInvoice();
        }
    );

    connect(
        logoutBtn,
        &QPushButton::clicked,
        [=]() {

            // Return to the dashboard so the next
            // login starts on a clean page
            stack->setCurrentWidget(dashboardPage);
            setActive(dashboardBtn);

            loginOverlay->setGeometry(
                this->rect()
            );
            loginOverlay->clearFields();
            loginOverlay->show();
            loginOverlay->raise();
        }
    );

    connect(
        loginOverlay,
        &LoginOverlay::loginSuccess,
        [=]() {

            loginOverlay->hide();
        }
    );
}

void MainWindow::resizeEvent(QResizeEvent *event) {

    QMainWindow::resizeEvent(event);

    if(loginOverlay) {

        loginOverlay->setGeometry(
            this->rect()
        );
    }
}
