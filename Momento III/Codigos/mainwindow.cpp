#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QFont>
#include <QPixmap>
#include <QPalette>
#include <QBrush>
#include <QUrl>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ventanaNivel1(nullptr) {

    setWindowTitle("Capitán América en la Guerra de los Mil Días");
    setMinimumSize(960, 540);

    // Música del menú principal (Inicio.mp3)
    reproductorMenu = new QMediaPlayer(this);
    salidaAudioMenu = new QAudioOutput(this);
    reproductorMenu->setAudioOutput(salidaAudioMenu);
    salidaAudioMenu->setVolume(0.65f);
    reproductorMenu->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/Inicio.mp3"));
    reproductorMenu->setLoops(QMediaPlayer::Infinite);
    reproductorMenu->play();

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QPixmap fondo(":/new/prefix1/FondoN1.jpg");
    if (!fondo.isNull()) {
        QPalette paleta;
        paleta.setBrush(QPalette::Window, QBrush(fondo.scaled(1280, 720, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
        central->setAutoFillBackground(true);
        central->setPalette(paleta);
    } else {
        central->setStyleSheet("background-color: #1c2430;");
    }

    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setAlignment(Qt::AlignCenter);

    QWidget* panelTitulo = new QWidget(central);
    panelTitulo->setObjectName("panelMenu");
    panelTitulo->setStyleSheet(
        "QWidget#panelMenu {"
        "   background-color: rgba(15, 18, 24, 215);"
        "   border: 4px solid #e6c86e;"
        "   border-radius: 6px;"
        "}"
        );

    QVBoxLayout* panelLayout = new QVBoxLayout(panelTitulo);
    panelLayout->setContentsMargins(46, 32, 46, 32);
    panelLayout->setAlignment(Qt::AlignCenter);
    panelLayout->setSpacing(16);

    QLabel* lblSubtitulo = new QLabel("MISIÓN HISTÓRICA • NIVEL 1", panelTitulo);
    lblSubtitulo->setAlignment(Qt::AlignCenter);
    lblSubtitulo->setStyleSheet(
        "color: #f1c40f;"
        "font-family: 'Courier New';"
        "font-size: 16px;"
        "font-weight: bold;"
        "background: transparent;"
        );

    QLabel* lblTitulo = new QLabel("CAPITÁN AMÉRICA\nEN LA GUERRA DE LOS MIL DÍAS", panelTitulo);
    lblTitulo->setAlignment(Qt::AlignCenter);
    lblTitulo->setStyleSheet(
        "color: #ffffff;"
        "font-family: 'Courier New';"
        "font-size: 34px;"
        "font-weight: bold;"
        "letter-spacing: 2px;"
        "background: transparent;"
        );

    QGraphicsDropShadowEffect* sombraTexto = new QGraphicsDropShadowEffect(lblTitulo);
    sombraTexto->setBlurRadius(4);
    sombraTexto->setOffset(3, 3);
    sombraTexto->setColor(QColor(0, 0, 0, 230));
    lblTitulo->setGraphicsEffect(sombraTexto);

    btnIniciar = new QPushButton("INICIAR", panelTitulo);
    btnIniciar->setCursor(Qt::PointingHandCursor);
    btnIniciar->setFixedSize(250, 62);
    btnIniciar->setStyleSheet(
        "QPushButton {"
        "   background-color: #c0392b;"
        "   color: #ffffff;"
        "   font-family: 'Courier New';"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   padding: 0px;"
        "   border: 3px solid #f5d76e;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e74c3c;"
        "   border: 3px solid #ffffff;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #922b21;"
        "}"
        );

    panelLayout->addWidget(lblSubtitulo);
    panelLayout->addWidget(lblTitulo);
    panelLayout->addSpacing(12);
    panelLayout->addWidget(btnIniciar, 0, Qt::AlignCenter);

    layout->addWidget(panelTitulo);

    connect(btnIniciar, &QPushButton::clicked, this, &MainWindow::iniciarJuego);
}

MainWindow::~MainWindow() {
    if (ventanaNivel1) {
        delete ventanaNivel1;
    }
}

void MainWindow::iniciarJuego() {
    reproductorMenu->stop();
    ventanaNivel1 = new VentanaNivel1();
    ventanaNivel1->setWindowTitle("Capitán América en la Guerra de los Mil Días - Nivel 1");
    ventanaNivel1->showMaximized();
    this->hide();
}