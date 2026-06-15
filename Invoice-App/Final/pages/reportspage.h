#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H

#include <QWidget>
#include <QList>

class QLabel;
class QTableWidget;
class QShowEvent;

class ReportsPage : public QWidget {

    Q_OBJECT

public:
    explicit ReportsPage(QWidget *parent = nullptr);

protected:

    void showEvent(QShowEvent *event) override;

private:

    QList<QLabel*> statValues;
    QTableWidget *monthlyTable;

    void setupUI();
    void loadReport();
};

#endif
