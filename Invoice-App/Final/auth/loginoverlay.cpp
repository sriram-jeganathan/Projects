#include "loginoverlay.h"
#include "authservice.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QMessageBox>


LoginOverlay::LoginOverlay(QWidget *parent)
    : QWidget(parent) {

    setupUI();
}

void LoginOverlay::setupUI() {

    setAutoFillBackground(true);

    if(parentWidget()) {

        resize(
            parentWidget()->size()
        );
    }

    /*
     * MAIN LAYOUT
     */

    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);

    mainLayout->setContentsMargins(
        0,
        0,
        0,
        0
    );

    mainLayout->setAlignment(
        Qt::AlignCenter
    );

    /*
     * LOGIN CARD
     */

    loginCard =
        new QFrame();

    loginCard->setFixedSize(520, 360);

    loginCard->setStyleSheet(R"(

        background-color: #F6EFE7;
        border-radius: 34px;

    )");

    /*
     * CARD LAYOUT
     */

    QVBoxLayout *cardLayout =
        new QVBoxLayout(loginCard);

    cardLayout->setContentsMargins(
        42,
        36,
        42,
        36
    );

    cardLayout->setSpacing(18);

    /*
     * LOGO
     */

    QLabel *logo =
        new QLabel("InvoicePro");

    logo->setStyleSheet(R"(

        color: #1E293B;
        font-size: 44px;
        font-weight: bold;
        background: transparent;

    )");

    QLabel *subtext =
        new QLabel("Login to continue");

    subtext->setStyleSheet(R"(

        color: #64748B;
        font-size: 17px;
        background: transparent;

    )");

    /*
     * USERNAME
     */

    usernameInput =
        new QLineEdit();

    usernameInput->setPlaceholderText(
        "Username"
    );

    usernameInput->setFixedHeight(58);

    usernameInput->setStyleSheet(R"(

        background-color: #FFF9F4;
        border: 2px solid #E5D6C8;
        border-radius: 16px;
        padding-left: 18px;
        font-size: 16px;
        color: #1E293B;

    )");

    /*
     * PASSWORD
     */

    passwordInput =
        new QLineEdit();

    passwordInput->setPlaceholderText(
        "Password"
    );

    passwordInput->setEchoMode(
        QLineEdit::Password
    );

    passwordInput->setFixedHeight(58);

    passwordInput->setStyleSheet(R"(

        background-color: #FFF9F4;
        border: 2px solid #E5D6C8;
        border-radius: 16px;
        padding-left: 18px;
        font-size: 16px;
        color: #1E293B;

    )");

    /*
     * LOGIN BUTTON
     */

    loginBtn =
        new QPushButton("Login");

    loginBtn->setFixedHeight(60);

    loginBtn->setCursor(
        Qt::PointingHandCursor
    );

    loginBtn->setStyleSheet(R"(

        QPushButton {
            background-color: #2563EB;
            color: white;
            border: none;
            border-radius: 18px;
            font-size: 18px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #1D4ED8;
        }

    )");

    /*
     * ADD WIDGETS
     */

    cardLayout->addWidget(logo);

    cardLayout->addWidget(subtext);

    cardLayout->addSpacing(10);

    cardLayout->addWidget(usernameInput);

    cardLayout->addWidget(passwordInput);

    cardLayout->addSpacing(8);

    cardLayout->addWidget(loginBtn);

    /*
     * CENTER CARD
     */

    mainLayout->addWidget(
        loginCard,
        0,
        Qt::AlignCenter
    );

    /*
     * LOGIN ACTION
     */

    connect(
    loginBtn,
    &QPushButton::clicked,
    [=]() {

        QString username =
            usernameInput->text().trimmed();

        QString password =
            passwordInput->text();

        AuthService auth;

        if(auth.login(
            username,
            password
        )) {

            emit loginSuccess();
        }
        else {

            QMessageBox::warning(
                this,
                "Login Failed",
                "Invalid username or password."
            );

            passwordInput->clear();

            passwordInput->setFocus();
        }
    }
);
}

void LoginOverlay::clearFields() {

    usernameInput->clear();

    passwordInput->clear();
}

void LoginOverlay::paintEvent(QPaintEvent *) {

    QPainter painter(this);

    painter.fillRect(
        rect(),
        QColor("#0F172A")
    );
}

void LoginOverlay::resizeEvent(QResizeEvent *event) {

    QWidget::resizeEvent(event);

    if(parentWidget()) {

        resize(
            parentWidget()->size()
        );
    }
}