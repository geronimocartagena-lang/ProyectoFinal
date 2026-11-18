#include "VentanaNivel1.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QMessageBox>

VentanaNivel1::VentanaNivel1(QWidget* parent)
    : QGraphicsView(parent),
    mirandoDerecha(true),
    teclaIzquierda(false),
    teclaDerecha(false) {

    setFixedSize(960, 540);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    escena = new QGraphicsScene(0, 0, 2600, 540, this);
    setScene(escena);

    controlador = new ControladorNivel1(this);

    construirEscenario();

    connect(controlador, &ControladorNivel1::tiempoActualizado, this, &VentanaNivel1::actualizarHUD);
    connect(controlador, &ControladorNivel1::nivelFinalizado, this, &VentanaNivel1::manejarFinNivel);

    gameLoopTimer = new QTimer(this);
    connect(gameLoopTimer, &QTimer::timeout, this, &VentanaNivel1::actualizarFrame);

    controlador->iniciarReloj();
    gameLoopTimer->start(16);
}

VentanaNivel1::~VentanaNivel1() {
    delete escena;
}

void VentanaNivel1::construirEscenario() {
    // Fondo cielo
    escena->setBackgroundBrush(QBrush(QColor(135, 175, 205)));

    // Suelo principal
    escena->addRect(0, 450, 2600, 90, QPen(Qt::NoPen), QBrush(QColor(92, 64, 51)));

    // Obstáculos y fango
    for (const auto& obs : controlador->getMapaTerreno()) {
        if (obs.esFango) {
            escena->addRect(obs.area, QPen(Qt::NoPen), QBrush(QColor(60, 40, 20)));
        } else {
            escena->addRect(obs.area, QPen(Qt::black), QBrush(QColor(160, 110, 60)));
        }
    }

    // Zona de evacuación
    escena->addRect(controlador->getZonaEvacuacion(), QPen(QPen(Qt::darkGreen, 3)), QBrush(QColor(46, 204, 113, 120)));

    // Representación visual provisional (antes de sprites)
    itemHeroe = escena->addRect(0, 0, 45, 65, QPen(Qt::black), QBrush(Qt::blue));
    itemEscudo = escena->addEllipse(0, 0, 24, 24, QPen(Qt::white, 2), QBrush(Qt::red));
    itemEscudo->setVisible(false);

    for (size_t i = 0; i < controlador->getCiviles().size(); ++i) {
        itemsCiviles.push_back(escena->addRect(0, 0, 35, 55, QPen(Qt::black), QBrush(Qt::yellow)));
    }

    // HUD
    hudTexto = escena->addText("Tiempo: 03:00 | Rescatados: 0/5 | Estado: Normal");
    hudTexto->setDefaultTextColor(Qt::white);
    hudTexto->setFont(QFont("Arial", 13, QFont::Bold));
    hudTexto->setPos(20, 15);
}

void VentanaNivel1::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        teclaIzquierda = true;
        mirandoDerecha = false;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        teclaDerecha = true;
        mirandoDerecha = true;
        break;
    case Qt::Key_W:
    case Qt::Key_Space:
        controlador->getHeroe()->saltar();
        break;
    case Qt::Key_J:
        controlador->getHeroe()->lanzarEscudo(mirandoDerecha ? 25.0f : 155.0f);
        break;
    case Qt::Key_K:
        controlador->getHeroe()->bloquear(true);
        break;
    }
}

void VentanaNivel1::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        teclaIzquierda = false;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        teclaDerecha = false;
        break;
    case Qt::Key_K:
        controlador->getHeroe()->bloquear(false);
        break;
    }
}

void VentanaNivel1::actualizarFrame() {
    float dt = 0.016f;
    float friccion = controlador->obtenerFriccionActualHeroe();

    int dir = 0;
    if (teclaIzquierda) dir -= 1;
    if (teclaDerecha) dir += 1;

    controlador->getHeroe()->moverHorizontal(dir, friccion);
    controlador->actualizarReglas(dt);

    CapitanAmerica* heroe = controlador->getHeroe();
    itemHeroe->setPos(heroe->getPosX(), heroe->getPosY());

    // Cambio de color visual para confirmar estado especial (Fortachón = morado, Veloz = cian, Bloqueando = verde)
    if (heroe->isBloqueando()) {
        itemHeroe->setBrush(Qt::darkGreen);
    } else if (heroe->getEstadoEspecial() == "Fortachon") {
        itemHeroe->setBrush(Qt::darkMagenta);
    } else if (heroe->getEstadoEspecial() == "Veloz") {
        itemHeroe->setBrush(Qt::cyan);
    } else {
        itemHeroe->setBrush(Qt::blue);
    }

    EscudoVibranium* escudo = heroe->getEscudo();
    itemEscudo->setVisible(escudo->estaActivo());
    if (escudo->estaActivo()) {
        itemEscudo->setPos(escudo->getPosX() - 12.0f, escudo->getPosY() - 12.0f);
    }

    const auto& civiles = controlador->getCiviles();
    for (size_t i = 0; i < civiles.size(); ++i) {
        itemsCiviles[i]->setVisible(!civiles[i]->isEvacuado());
        itemsCiviles[i]->setPos(civiles[i]->getPosX(), civiles[i]->getPosY());
        if (civiles[i]->isRescatado()) {
            itemsCiviles[i]->setBrush(Qt::green); // Cambia a verde cuando te sigue
        }
    }

    centerOn(heroe->getPosX() + 150.0f, 270.0f);
    hudTexto->setPos(mapToScene(20, 15));
}

void VentanaNivel1::actualizarHUD(int segundosRestantes) {
    int min = segundosRestantes / 60;
    int seg = segundosRestantes % 60;
    QString texto = QString("Tiempo: %1:%2 | Rescatados: %3/5 | Estado: %4")
                        .arg(min, 2, 10, QChar('0'))
                        .arg(seg, 2, 10, QChar('0'))
                        .arg(controlador->getCivilesEvacuados())
                        .arg(controlador->getHeroe()->getEstadoEspecial());
    hudTexto->setPlainText(texto);
}

void VentanaNivel1::manejarFinNivel(bool victoria) {
    gameLoopTimer->stop();
    if (victoria) {
        QMessageBox::information(this, "¡Pueblo a Salvo!",
                                 "Has evacuado a los 5 grupos de civiles.\n\n"
                                 "Cinemática: En el futuro, Donald Trump observa el mapa de Colombia y Panamá:\n"
                                 "«Que Colombia pelee entre ellos. Me quedaré con Panamá».");
    } else {
        QMessageBox::warning(this, "Tiempo Agotado", "No lograste evacuar el pueblo a tiempo.");
    }
}