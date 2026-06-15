#include "settingspage.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent) {

    setupUI();
    loadSettings();
}

void SettingsPage::setupUI() {

    setStyleSheet(R"(

        QWidget {
            background-color: #EDF2FA;
            color: #1E293B;
            font-size: 15px;
        }

    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 35, 35, 35);

    QVBoxLayout *titleLayout = new QVBoxLayout();

    QLabel *heading = new QLabel("Settings");
    heading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #1E293B; font-size: 42px; font-weight: bold;");

    QLabel *subheading = new QLabel("Company and invoicing defaults");
    subheading->setStyleSheet(
        "background: transparent; border: none;"
        "color: #64748B; font-size: 17px;");

    titleLayout->addWidget(heading);
    titleLayout->addWidget(subheading);

    QFrame *formCard = new QFrame();
    formCard->setMaximumWidth(760);
    formCard->setStyleSheet(
        "background-color: #F6EFE7;"
        "border-radius: 28px;"
        "border: 1px solid #E5D6C8;");

    QVBoxLayout *formLayout = new QVBoxLayout(formCard);
    formLayout->setContentsMargins(32, 32, 32, 32);
    formLayout->setSpacing(8);

    const QString inputStyle = R"(

        QLineEdit {
            background-color: #FFF9F4;
            border: 1px solid #E5D6C8;
            border-radius: 14px;
            padding-left: 14px;
            color: #1E293B;
            font-size: 15px;
        }

        QLineEdit:focus {
            border: 2px solid #2563EB;
        }

    )";

    const QString fieldLabelStyle =
        "background: transparent; border: none;"
        "color: #64748B; font-size: 15px; font-weight: 600;";

    auto addField =
        [&](const QString &labelText, QLineEdit *&target,
            const QString &placeholder) {

        QLabel *fieldLabel = new QLabel(labelText);
        fieldLabel->setStyleSheet(fieldLabelStyle);

        target = new QLineEdit();
        target->setPlaceholderText(placeholder);
        target->setFixedHeight(50);
        target->setStyleSheet(inputStyle);

        formLayout->addWidget(fieldLabel);
        formLayout->addWidget(target);
        formLayout->addSpacing(12);
    };

    addField("Company Name", companyInput, "e.g. InvoicePro Solutions");
    addField("GSTIN", gstinInput, "e.g. 29ABCDE1234F1Z5");
    addField("PAN", panInput, "e.g. ABCDE1234F");
    addField("Company State", stateInput, "e.g. Karnataka (29)");
    addField("Address", addressInput, "Registered business address");
    addField("Default GST Rate (%)", gstRateInput, "e.g. 18");
    addField("Invoice Number Prefix", prefixInput, "e.g. INV");

    QPushButton *saveBtn = new QPushButton("Save Settings");
    saveBtn->setFixedHeight(54);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(

        QPushButton {
            background-color: #2563EB;
            color: white;
            border: none;
            border-radius: 16px;
            font-size: 16px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #1D4ED8;
        }

    )");

    statusLabel = new QLabel("");
    statusLabel->setStyleSheet(
        "background: transparent; border: none;"
        "color: #16A34A; font-size: 14px; font-weight: 600;");

    formLayout->addSpacing(6);
    formLayout->addWidget(saveBtn);
    formLayout->addWidget(statusLabel);

    mainLayout->addLayout(titleLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(formCard, 0, Qt::AlignLeft);
    mainLayout->addStretch();

    connect(saveBtn, &QPushButton::clicked,
            this, &SettingsPage::saveSettings);
}

void SettingsPage::loadSettings() {

    QSettings settings("InvoicePro", "InvoicePro");

    companyInput->setText(settings.value("company/name").toString());
    gstinInput->setText(settings.value("company/gstin").toString());
    panInput->setText(settings.value("company/pan").toString());
    stateInput->setText(settings.value("company/state").toString());
    addressInput->setText(settings.value("company/address").toString());
    gstRateInput->setText(
        settings.value("invoice/defaultGstRate", "18").toString());
    prefixInput->setText(
        settings.value("invoice/prefix", "INV").toString());
}

void SettingsPage::saveSettings() {

    QSettings settings("InvoicePro", "InvoicePro");

    settings.setValue("company/name", companyInput->text().trimmed());
    settings.setValue("company/gstin", gstinInput->text().trimmed());
    settings.setValue("company/pan", panInput->text().trimmed());
    settings.setValue("company/state", stateInput->text().trimmed());
    settings.setValue("company/address", addressInput->text().trimmed());
    settings.setValue("invoice/defaultGstRate",
                      gstRateInput->text().trimmed());
    settings.setValue("invoice/prefix", prefixInput->text().trimmed());

    settings.sync();

    statusLabel->setText("Settings saved.");
}
