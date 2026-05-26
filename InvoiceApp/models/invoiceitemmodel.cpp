#include "models/invoiceitemmodel.h"

#include <QBrush>

InvoiceItemModel::InvoiceItemModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int InvoiceItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.count();
}

int InvoiceItemModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 8;
    return 8;
}

QVariant InvoiceItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.count())
        return {};

    const InvoiceItem& item = m_items.at(index.row());
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return item.itemName;
        case 1: return item.quantity;
        case 2: return item.price;
        case 3: return item.gstPercent;
        case 4: return item.cgst;
        case 5: return item.sgst;
        case 6: return item.igst;
        case 7: return item.total;
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0)
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        return int(Qt::AlignRight | Qt::AlignVCenter);
    }

    if (role == Qt::BackgroundRole && index.row() % 2 == 1) {
        return QBrush(QColor(248, 248, 248));
    }

    return {};
}

QVariant InvoiceItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case 0: return "Item";
    case 1: return "Qty";
    case 2: return "Price";
    case 3: return "GST %";
    case 4: return "CGST";
    case 5: return "SGST";
    case 6: return "IGST";
    case 7: return "Total";
    }
    return {};
}

Qt::ItemFlags InvoiceItemModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == 0 || index.column() == 1 || index.column() == 2 || index.column() == 3)
        flags |= Qt::ItemIsEditable;
    return flags;
}

bool InvoiceItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    InvoiceItem& item = m_items[index.row()];
    switch (index.column()) {
    case 0: item.itemName = value.toString(); break;
    case 1: item.quantity = value.toInt(); break;
    case 2: item.price = value.toDouble(); break;
    case 3: item.gstPercent = value.toDouble(); break;
    default: return false;
    }

    recalcRow(index.row());
    emit dataChanged(index, index);
    emit dataChanged(createIndex(index.row(), 4), createIndex(index.row(), 7));
    return true;
}

void InvoiceItemModel::addRow()
{
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    InvoiceItem item;
    item.quantity = 1;
    item.price = 0.0;
    item.gstPercent = 0.0;
    item.total = 0.0;
    m_items.append(item);
    endInsertRows();
}

void InvoiceItemModel::removeRow(int row)
{
    if (row < 0 || row >= m_items.count())
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}

void InvoiceItemModel::resetRows()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

QVector<InvoiceItem> InvoiceItemModel::items() const
{
    return m_items;
}

void InvoiceItemModel::setItems(const QVector<InvoiceItem>& items)
{
    beginResetModel();
    m_items = items;
    for (int row = 0; row < m_items.count(); ++row)
        recalcRow(row);
    endResetModel();
}

double InvoiceItemModel::subtotal() const
{
    double sum = 0.0;
    for (const auto& item : m_items)
        sum += item.price * item.quantity;
    return sum;
}

double InvoiceItemModel::gstTotal() const
{
    double sum = 0.0;
    for (const auto& item : m_items)
        sum += item.cgst + item.sgst + item.igst;
    return sum;
}

double InvoiceItemModel::grandTotal() const
{
    return subtotal() + gstTotal();
}

void InvoiceItemModel::recalcRow(int row)
{
    if (row < 0 || row >= m_items.count())
        return;

    InvoiceItem& item = m_items[row];
    const double base = item.price * item.quantity;
    const double tax = base * item.gstPercent / 100.0;
    item.cgst = tax / 2.0;
    item.sgst = tax / 2.0;
    item.igst = 0.0;
    item.total = base + tax;
}
