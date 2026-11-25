#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFont>
#include <QPixmap>
#include <QPalette>
#include <QBrush>
#include <QUrl>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ventanaNivel1(nullptr), ventanaNivel2(nullptr) {

    setWindowTitle("Capitán América en la Guerra de los Mil Días");
    setMinimumSize(960, 540);

    reproductorMenu = new QMediaPlayer(this);
    salidaAudioMenu = new QAudioOutput(this);
    reproductorMenu->setAudioOutput(salidaAudioMenu);
    salidaAudioMenu->setVolume(0.85f);
    reproductorMenu->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/Inicio.mp3"));
    reproductorMenu->setLoops(QMediaPlayer::Once);

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
    panelLayout->setSpacing(14);

    QLabel* lblSubtitulo = new QLabel("", panelTitulo);
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

    QHBoxLayout* filaNivel1 = new QHBoxLayout();
    filaNivel1->setSpacing(12);

    btnIniciar = new QPushButton("NIVEL 1: NORMAL", panelTitulo);
    btnIniciar->setCursor(Qt::PointingHandCursor);
    btnIniciar->setFixedSize(230, 58);
    btnIniciar->setStyleSheet(
        "QPushButton {"
        "   background-color: #27ae60;"
        "   color: #ffffff;"
        "   font-family: 'Courier New';"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   border: 3px solid #f5d76e;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2ecc71;"
        "   border: 3px solid #ffffff;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e8449;"
        "}"
        );

    btnIniciarDificil = new QPushButton("NIVEL 1: DIFÍCIL", panelTitulo);
    btnIniciarDificil->setCursor(Qt::PointingHandCursor);
    btnIniciarDificil->setFixedSize(230, 58);
    btnIniciarDificil->setStyleSheet(
        "QPushButton {"
        "   background-color: #c0392b;"
        "   color: #ffffff;"
        "   font-family: 'Courier New';"
        "   font-size: 18px;"
        "   font-weight: bold;"
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

    filaNivel1->addWidget(btnIniciar);
    filaNivel1->addWidget(btnIniciarDificil);

    btnNivel2Directo = new QPushButton("NIVEL 2: JEFE FINAL", panelTitulo);
    btnNivel2Directo->setCursor(Qt::PointingHandCursor);
    btnNivel2Directo->setFixedSize(472, 58);
    btnNivel2Directo->setStyleSheet(
        "QPushButton {"
        "   background-color: #1f618d;"
        "   color: #ffffff;"
        "   font-family: 'Courier New';"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   border: 3px solid #f5d76e;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "   border: 3px solid #ffffff;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #154360;"
        "}"
        );

    panelLayout->addWidget(lblSubtitulo);
    panelLayout->addWidget(lblTitulo);
    panelLayout->addSpacing(10);
    panelLayout->addLayout(filaNivel1);
    panelLayout->addWidget(btnNivel2Directo, 0, Qt::AlignCenter);

    layout->addWidget(panelTitulo);

    connect(btnIniciar, &QPushButton::clicked, this, [this]() { iniciarJuego(false); });
    connect(btnIniciarDificil, &QPushButton::clicked, this, [this]() { iniciarJuego(true); });
    connect(btnNivel2Directo, &QPushButton::clicked, this, &MainWindow::abrirNivel2);
}

MainWindow::~MainWindow() {
    if (ventanaNivel1) delete ventanaNivel1;
    if (ventanaNivel2) delete ventanaNivel2;
}

void MainWindow::iniciarJuego(bool modoDificil) {
    btnIniciar->setEnabled(false);
    btnIniciarDificil->setEnabled(false);
    btnNivel2Directo->setEnabled(false);
    reproductorMenu->setPosition(0);
    reproductorMenu->play();

    if (ventanaNivel1) {
        ventanaNivel1->deleteLater();
        ventanaNivel1 = nullptr;
    }

    ventanaNivel1 = new VentanaNivel1(modoDificil);
    ventanaNivel1->setAttribute(Qt::WA_DeleteOnClose, false);
    ventanaNivel1->setWindowTitle(modoDificil
                                      ? "Capitán América - Nivel 1: Rescate en Colombia [DIFÍCIL]"
                                      : "Capitán América - Nivel 1: Rescate en Colombia [NORMAL]");
    connect(ventanaNivel1, &VentanaNivel1::regresarAlMenu, this, &MainWindow::volverAlMenu);
    connect(ventanaNivel1, &VentanaNivel1::iniciarNivel2, this, &MainWindow::abrirNivel2);

    ventanaNivel1->showMaximized();
    this->hide();
}

void MainWindow::abrirNivel2() {
    btnIniciar->setEnabled(false);
    btnIniciarDificil->setEnabled(false);
    btnNivel2Directo->setEnabled(false);
    reproductorMenu->setPosition(0);
    reproductorMenu->play();

    if (ventanaNivel1) {
        ventanaNivel1->disconnect();
        ventanaNivel1->hide();
        VentanaNivel1* viejaN1 = ventanaNivel1;
        ventanaNivel1 = nullptr;
        QTimer::singleShot(0, viejaN1, &QObject::deleteLater);
    }

    if (ventanaNivel2) {
        ventanaNivel2->disconnect();
        ventanaNivel2->hide();
        ventanaNivel2->deleteLater();
        ventanaNivel2 = nullptr;
    }

    ventanaNivel2 = new VentanaNivel2();
    ventanaNivel2->setAttribute(Qt::WA_DeleteOnClose, false);
    ventanaNivel2->setWindowTitle("Capitán América - Nivel 2: Batalla Final en la Costa del Caribe");
    connect(ventanaNivel2, &VentanaNivel2::regresarAlMenu, this, &MainWindow::volverAlMenu);

    ventanaNivel2->showMaximized();
    ventanaNivel2->raise();
    ventanaNivel2->activateWindow();
    this->hide();
}

void MainWindow::volverAlMenu() {
    if (ventanaNivel1) {
        ventanaNivel1->disconnect();
        ventanaNivel1->hide();
        VentanaNivel1* viejaN1 = ventanaNivel1;
        ventanaNivel1 = nullptr;
        QTimer::singleShot(0, viejaN1, &QObject::deleteLater);
    }
    if (ventanaNivel2) {
        ventanaNivel2->disconnect();
        ventanaNivel2->hide();
        VentanaNivel2* viejaN2 = ventanaNivel2;
        ventanaNivel2 = nullptr;
        QTimer::singleShot(0, viejaN2, &QObject::deleteLater);
    }
    btnIniciar->setEnabled(true);
    btnIniciarDificil->setEnabled(true);
    btnNivel2Directo->setEnabled(true);
    this->show();
    this->raise();
    this->activateWindow();
}