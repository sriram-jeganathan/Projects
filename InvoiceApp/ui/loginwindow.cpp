#include "ui/loginwindow.h"
#include "services/authservice.h"
#include "core/sessionmanager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("InvoiceApp Login");
    setMinimumSize(360, 280);

    auto layout = new QVBoxLayout(this);
    m_titleLabel = new QLabel("Sign in", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    layout->addWidget(m_titleLabel);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("Username");
    layout->addWidget(m_usernameEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("Password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passwordEdit);

    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setPlaceholderText("Confirm password");
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setVisible(false);
    layout->addWidget(m_confirmEdit);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: red;");
    layout->addWidget(m_statusLabel);

    m_actionButton = new QPushButton("Login", this);
    m_toggleButton = new QPushButton("Create account", this);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_actionButton);
    buttonLayout->addWidget(m_toggleButton);
    layout->addLayout(buttonLayout);

    connect(m_actionButton, &QPushButton::clicked, this, &LoginWindow::handleAuth);
    connect(m_toggleButton, &QPushButton::clicked, this, &LoginWindow::toggleMode);

    updateMode();
}

void LoginWindow::handleAuth()
{
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    const QString confirm = m_confirmEdit->text();
    QString error;
    AuthService service;
    User user;

    if (m_loginMode) {
        if (!service.login(username, password, &error, &user)) {
            m_statusLabel->setText(error);
            return;
        }
    } else {
        if (password != confirm) {
            m_statusLabel->setText("Passwords do not match.");
            return;
        }
        if (!service.signup(username, password, &error, &user)) {
            m_statusLabel->setText(error);
            return;
        }
    }

    SessionManager::instance().setUser(user);
    accept();
}

void LoginWindow::toggleMode()
{
    m_loginMode = !m_loginMode;
    updateMode();
}

void LoginWindow::updateMode()
{
    if (m_loginMode) {
        m_titleLabel->setText("Sign in");
        m_actionButton->setText("Login");
        m_toggleButton->setText("Create account");
        m_confirmEdit->setVisible(false);
    } else {
        m_titleLabel->setText("Sign up");
        m_actionButton->setText("Register");
        m_toggleButton->setText("Already have an account");
        m_confirmEdit->setVisible(true);
    }
    m_statusLabel->clear();
}
