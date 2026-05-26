#include "database.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

Database& Database::instance()
{
    static Database d;
    return d;
}

bool Database::connect(const QString& filePath)
{
    // Ensure parent directory exists
    QFileInfo info(filePath);
    QDir().mkpath(info.absolutePath());

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(filePath);

    if (!m_db.open()) {
        qWarning() << "DB open failed:" << m_db.lastError().text();
        return false;
    }

    // Enable foreign key support
    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON;");

    return true;
}

QSqlDatabase& Database::db()
{
    return m_db;
}

bool Database::initSchema()
{
    QSqlQuery q(m_db);

    const QStringList tables = {

        "CREATE TABLE IF NOT EXISTS users ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " username TEXT UNIQUE NOT NULL,"
        " password_hash TEXT NOT NULL,"
        " salt TEXT NOT NULL,"
        " created_at TEXT NOT NULL)",

        "CREATE TABLE IF NOT EXISTS vendors ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " name TEXT NOT NULL,"
        " gst TEXT,"
        " phone TEXT,"
        " email TEXT,"
        " address TEXT,"
        " tax_details TEXT,"
        " created_at TEXT NOT NULL)",

        "CREATE TABLE IF NOT EXISTS invoices ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " invoice_no TEXT UNIQUE NOT NULL,"
        " vendor_id INTEGER NOT NULL,"
        " invoice_date TEXT NOT NULL,"
        " due_date TEXT,"
        " subtotal REAL NOT NULL DEFAULT 0,"
        " gst_total REAL NOT NULL DEFAULT 0,"
        " grand_total REAL NOT NULL DEFAULT 0,"
        " status TEXT NOT NULL DEFAULT 'Pending',"
        " notes TEXT,"
        " created_at TEXT NOT NULL,"
        " FOREIGN KEY(vendor_id) REFERENCES vendors(id))",

        "CREATE TABLE IF NOT EXISTS invoice_items ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " invoice_id INTEGER NOT NULL,"
        " item_name TEXT NOT NULL,"
        " quantity INTEGER NOT NULL DEFAULT 1,"
        " price REAL NOT NULL DEFAULT 0,"
        " gst_percent REAL NOT NULL DEFAULT 0,"
        " cgst REAL NOT NULL DEFAULT 0,"
        " sgst REAL NOT NULL DEFAULT 0,"
        " igst REAL NOT NULL DEFAULT 0,"
        " total REAL NOT NULL DEFAULT 0,"
        " FOREIGN KEY(invoice_id) REFERENCES invoices(id) ON DELETE CASCADE)",

        "CREATE TABLE IF NOT EXISTS company_settings ("
        " id INTEGER PRIMARY KEY CHECK (id = 1),"
        " company_name TEXT,"
        " company_gst TEXT,"
        " address TEXT,"
        " phone TEXT,"
        " email TEXT,"
        " logo_path TEXT)"
    };

    for (const QString& sql : tables) {
        if (!q.exec(sql)) {
            qWarning() << "Schema error:" << q.lastError().text();
            return false;
        }
    }

    return true;
}
