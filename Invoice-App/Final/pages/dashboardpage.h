#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include <QList>

class QLabel;
class QTableWidget;
class QShowEvent;

class DashboardPage : public QWidget {

    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

signals:

    void newInvoiceRequested();

protected:

    void showEvent(QShowEvent *event) override;

private:

    QList<QLabel*> statValues;
    QTableWidget *recentTable;

    void setupUI();
    void loadStats();
    void loadRecentInvoices();
};

#endif
