#ifndef LOGINOVERLAY_H
#define LOGINOVERLAY_H

#include <QWidget>

class QFrame;
class QLineEdit;
class QPushButton;

class LoginOverlay : public QWidget {

    Q_OBJECT

public:

    explicit LoginOverlay(QWidget *parent = nullptr);

    void clearFields();

signals:

    void loginSuccess();

protected:

    void resizeEvent(QResizeEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

private:

    QFrame *loginCard;

    QLineEdit *usernameInput;
    QLineEdit *passwordInput;

    QPushButton *loginBtn;

    void setupUI();
};

#endif