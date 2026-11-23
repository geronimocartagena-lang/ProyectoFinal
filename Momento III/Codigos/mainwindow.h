#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "VentanaNivel1.h"
#include "VentanaNivel2.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    VentanaNivel1* ventanaNivel1;
    VentanaNivel2* ventanaNivel2;
    QPushButton* btnIniciar;
    QPushButton* btnNivel2Directo;
    QMediaPlayer* reproductorMenu;
    QAudioOutput* salidaAudioMenu;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void iniciarJuego();
    void abrirNivel2();
    void volverAlMenu();
};

#endif // MAINWINDOW_H