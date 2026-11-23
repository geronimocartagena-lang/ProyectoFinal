#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include "VentanaNivel1.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    VentanaNivel1* ventanaNivel1;
    QPushButton* btnIniciar;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void iniciarJuego();
};

#endif // MAINWINDOW_H