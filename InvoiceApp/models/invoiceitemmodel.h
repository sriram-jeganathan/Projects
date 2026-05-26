#ifndef INVOICEITEMMODEL_H
#define INVOICEITEMMODEL_H

#include <QAbstractTableModel>
#include "models/invoiceitem.h"

class InvoiceItemModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit InvoiceItemModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void addRow();
    void removeRow(int row);
    void resetRows();
    QVector<InvoiceItem> items() const;
    void setItems(const QVector<InvoiceItem>& items);
    double subtotal() const;
    double gstTotal() const;
    double grandTotal() const;

private:
    void recalcRow(int row);
    QVector<InvoiceItem> m_items;
};

#endif // INVOICEITEMMODEL_H
