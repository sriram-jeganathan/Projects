#include <QApplication>
#include "mainwindow.h"

int main ( int argc, char ** argv ) {
  QApplication theApp( argc, argv );

  MainWindow window;
  window.show();

  return theApp.exec();
}
