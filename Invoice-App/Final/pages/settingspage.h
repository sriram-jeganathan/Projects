#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

class QLineEdit;
class QLabel;

class SettingsPage : public QWidget {

    Q_OBJECT

public:

    explicit SettingsPage(QWidget *parent = nullptr);

private slots:

    void saveSettings();

private:

    QLineEdit *companyInput;
    QLineEdit *gstinInput;
    QLineEdit *panInput;
    QLineEdit *stateInput;
    QLineEdit *addressInput;
    QLineEdit *gstRateInput;
    QLineEdit *prefixInput;

    QLabel *statusLabel;

    void loadSettings();

    void setupUI();
};

#endif
