#ifndef CLIENTSPAGE_H
#define CLIENTSPAGE_H

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QShowEvent;

class ClientsPage : public QWidget {

    Q_OBJECT

public:
    explicit ClientsPage(QWidget *parent = nullptr);

protected:

    void showEvent(QShowEvent *event) override;

private slots:

    void loadClients();
    void addClient();
    void filterClients(const QString &text);

private:

    void editClient(int id);
    void deleteClient(int id, const QString &name);
    QWidget *makeActions(int id, const QString &name);

    QTableWidget *table;
    QLineEdit *searchBar;

    void setupUI();
};

#endif
