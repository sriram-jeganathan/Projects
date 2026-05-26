#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);

private slots:
    void handleAuth();
    void toggleMode();

private:
    void updateMode();

    bool m_loginMode = true;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QLineEdit* m_confirmEdit = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QPushButton* m_actionButton = nullptr;
    QPushButton* m_toggleButton = nullptr;
};

#endif // LOGINWINDOW_H
