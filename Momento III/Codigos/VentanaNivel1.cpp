#include "VentanaNivel1.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QMessageBox>
#include <QDebug>
#include <stdexcept>

VentanaNivel1::VentanaNivel1(QWidget* parent)
    : QGraphicsView(parent), teclaIzquierda(false), teclaDerecha(false) {

    // Configurar ventana y escena de 2600px de ancho para el Side-Scroller
    setFixedSize(960, 540);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    escena = new QGraphicsScene(0, 0, 2600, 540, this);
    setScene(escena);

    controlador = new ControladorNivel1(this);

    construirEscenario();
    cargarRecursosGraficos();

    // Conectar señales del controlador con la interfaz gráfica
    connect(controlador, &ControladorNivel1::tiempoActualizado, this, &VentanaNivel1::actualizarHUD);
    connect(controlador, &ControladorNivel1::nivelFinalizado, this, &VentanaNivel1::manejarFinNivel);

    // Bucle principal a ~60 FPS (cada 16 ms)
    gameLoopTimer = new QTimer(this);
    connect(gameLoopTimer, &QTimer::timeout, this, &VentanaNivel1::actualizarFrame);

    controlador->iniciarReloj();
    gameLoopTimer->start(16);
}

VentanaNivel1::~VentanaNivel1() {
    delete escena;
}

void VentanaNivel1::construirEscenario() {
    // Fondo cielo colonial / pueblo andino
    escena->setBackgroundBrush(QBrush(QColor(135, 175, 205)));

    // Suelo base del pueblo
    escena->addRect(0, 450, 2600, 90, QPen(Qt::NoPen), QBrush(QColor(92, 64, 51)));

    // Dibujar obstáculos (plataformas de madera y zonas de fango)
    for (const auto& obs : controlador->getMapaTerreno()) {
        if (obs.esFango) {
            escena->addRect(obs.area, QPen(Qt::NoPen), QBrush(QColor(60, 40, 20))); // Fango oscuro
        } else {
            escena->addRect(obs.area, QPen(Qt::black), QBrush(QColor(160, 110, 60))); // Madera
        }
    }

    // Dibujar Zona Segura de Evacuación
    QRectF zona = controlador->getZonaEvacuacion();
    escena->addRect(zona, QPen(QPen(Qt::darkGreen, 3)), QBrush(QColor(46, 204, 113, 120)));

    // HUD (Texto fijo que seguirá la cámara)
    hudTexto = escena->addText("Tiempo: 03:00 | Rescatados: 0/5");
    hudTexto->setDefaultTextColor(Qt::white);
    hudTexto->setFont(QFont("Arial", 14, QFont::Bold));
    hudTexto->setPos(20, 15);
}

void VentanaNivel1::cargarRecursosGraficos() {
    // Uso obligatorio de excepciones para validación de archivos multimedia
    QPixmap pixHeroe, pixEscudo, pixCivil;

    try {
        if (!pixHeroe.load(":/sprites/capitan.png")) {
            throw std::runtime_error("No se encontró :/sprites/capitan.png. Usando gráfico de respaldo.");
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        pixHeroe = QPixmap(45, 65);
        pixHeroe.fill(Qt::blue); // Fallback visual para evitar que el juego cierre
    }

    try {
        if (!pixEscudo.load(":/sprites/escudo.png")) {
            throw std::runtime_error("No se encontró :/sprites/escudo.png. Usando gráfico de respaldo.");
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        pixEscudo = QPixmap(24, 24);
        pixEscudo.fill(Qt::red);
    }

    try {
        if (!pixCivil.load(":/sprites/civil.png")) {
            throw std::runtime_error("No se encontró :/sprites/civil.png. Usando gráfico de respaldo.");
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        pixCivil = QPixmap(35, 55);
        pixCivil.fill(Qt::yellow);
    }

    spriteHeroe = escena->addPixmap(pixHeroe.scaled(45, 65));
    spriteEscudo = escena->addPixmap(pixEscudo.scaled(24, 24));
    spriteEscudo->setVisible(false);

    for (size_t i = 0; i < controlador->getCiviles().size(); ++i) {
        QGraphicsPixmapItem* itemCivil = escena->addPixmap(pixCivil.scaled(35, 55));
        spritesCiviles.push_back(itemCivil);
    }
}

void VentanaNivel1::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        teclaIzquierda = true;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        teclaDerecha = true;
        break;
    case Qt::Key_W:
    case Qt::Key_Space:
        controlador->getHeroe()->saltar();
        break;
    case Qt::Key_J: // Lanzar escudo en ángulo parabólico de 25 grados
        controlador->getHeroe()->lanzarEscudo(25.0f);
        break;
    case Qt::Key_K: // Bloquear con escudo
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
    float dt = 0.016f; // 16 ms en segundos
    float friccion = controlador->obtenerFriccionActualHeroe();

    // Aplicar movimiento horizontal según teclas activas y fricción del terreno
    int dir = 0;
    if (teclaIzquierda) dir -= 1;
    if (teclaDerecha) dir += 1;
    controlador->getHeroe()->moverHorizontal(dir, friccion);

    // Ejecutar paso lógico
    controlador->actualizarReglas(dt);

    // Sincronizar sprites gráficos con las posiciones físicas de la capa lógica
    CapitanAmerica* heroe = controlador->getHeroe();
    spriteHeroe->setPos(heroe->getPosX(), heroe->getPosY());

    EscudoVibranium* escudo = heroe->getEscudo();
    spriteEscudo->setVisible(escudo->estaActivo());
    if (escudo->estaActivo()) {
        spriteEscudo->setPos(escudo->getPosX(), escudo->getPosY());
    }

    const auto& civiles = controlador->getCiviles();
    for (size_t i = 0; i < civiles.size(); ++i) {
        spritesCiviles[i]->setVisible(!civiles[i]->isEvacuado());
        spritesCiviles[i]->setPos(civiles[i]->getPosX(), civiles[i]->getPosY());
    }

    // Cámara Smooth Scroll siguiendo al Capitán América
    centerOn(heroe->getPosX() + 150.0f, 270.0f);

    // Mantener el HUD visible en la esquina superior izquierda de la vista
    QPointF esquinaVista = mapToScene(20, 15);
    hudTexto->setPos(esquinaVista);
}

void VentanaNivel1::actualizarHUD(int segundosRestantes) {
    int min = segundosRestantes / 60;
    int seg = segundosRestantes % 60;
    QString texto = QString("Tiempo: %1:%2 | Rescatados: %3/5")
                        .arg(min, 2, 10, QChar('0'))
                        .arg(seg, 2, 10, QChar('0'))
                        .arg(controlador->getCivilesEvacuados());
    hudTexto->setPlainText(texto);
}

void VentanaNivel1::manejarFinNivel(bool victoria) {
    gameLoopTimer->stop();
    if (victoria) {
        QMessageBox::information(this, "¡Pueblo a Salvo!",
                                 "Has evacuado a los 5 grupos de civiles.\n\n"
                                 "Cinemática: En el futuro, Donald Trump observa el mapa de Colombia y Panamá:\n"
                                 "«Que Colombia pelee entre ellos. Me quedaré con Panamá».");
        // Aquí se invoca el cambio hacia la ventana del Nivel 2 (Boss Final)
    } else {
        QMessageBox::warning(this, "Tiempo Agotado", "No lograste evacuar el pueblo a tiempo.");
    }
}