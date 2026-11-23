#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow menuPrincipal;
    menuPrincipal.showMaximized();

    return app.exec();
}