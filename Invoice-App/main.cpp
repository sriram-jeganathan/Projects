#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

int main ( int argc, char ** argv ) {
  QApplication theApp( argc, argv );

  QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

  db.setHostName("localhost");

  db.setDatabaseName("invoice_system");

  db.setUserName("sriram");

  db.setPassword("root");

  if ( db.open() ) {
    qDebug() << "Database Connected";
  } else {
    qDebug() << db.lastError().text();
  }

  return theApp.exec();
}
