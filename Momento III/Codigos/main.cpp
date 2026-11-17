#include <QApplication>
#include "VentanaNivel1.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    VentanaNivel1 ventana;
    ventana.setWindowTitle("Capitán América: Guerra de los Mil Días - Nivel 1");
    ventana.show();

    return app.exec();
}