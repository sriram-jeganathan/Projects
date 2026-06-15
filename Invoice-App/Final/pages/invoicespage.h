#ifndef INVOICESPAGE_H
#define INVOICESPAGE_H

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QShowEvent;

class InvoicesPage : public QWidget {

    Q_OBJECT

public:
    explicit InvoicesPage(QWidget *parent = nullptr);

public slots:

    void newInvoice();   // public so the Dashboard button can trigger it

protected:

    void showEvent(QShowEvent *event) override;

private slots:

    void loadInvoices();
    void filterInvoices(const QString &text);

private:

    void changeStatus(int invoiceId, const QString &current);
    void deleteInvoice(int invoiceId, const QString &invoiceNumber);
    void exportPdf(int invoiceId);
    QWidget *makeActions(int invoiceId, const QString &invoiceNumber,
                         const QString &status);

    QTableWidget *table;
    QLineEdit *searchBar;

    void setupUI();
};

#endif
