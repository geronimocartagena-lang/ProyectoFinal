#include "VentanaNivel1.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QMessageBox>
#include <QImage>
#include <QTransform>
#include <QDebug>
#include <stdexcept>
#include <algorithm>
#include <cmath>

VentanaNivel1::VentanaNivel1(QWidget* parent)
    : QGraphicsView(parent),
    frameHeroeActual(0),
    frameEscudoActual(0),
    frameCivilActual(0),
    contadorTicks(0),
    mirandoDerecha(true),
    teclaIzquierda(false),
    teclaDerecha(false) {

    setMinimumSize(960, 540);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::SmoothPixmapTransform);

    escena = new QGraphicsScene(0, 0, 4500, 540, this);
    setScene(escena);

    controlador = new ControladorNivel1(this);

    construirEscenario();
    cargarSprites();

    connect(controlador, &ControladorNivel1::nivelFinalizado, this, &VentanaNivel1::manejarFinNivel);

    gameLoopTimer = new QTimer(this);
    connect(gameLoopTimer, &QTimer::timeout, this, &VentanaNivel1::actualizarFrame);

    controlador->iniciarReloj();
    gameLoopTimer->start(16);
}

VentanaNivel1::~VentanaNivel1() {
    delete escena;
}

void VentanaNivel1::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (viewport()->height() <= 0 || viewport()->width() <= 0) return;

    double altoVirtual = 700.0;
    double proporcion = static_cast<double>(viewport()->width()) / static_cast<double>(viewport()->height());
    QRectF rectVisible(0, -100, altoVirtual * proporcion, altoVirtual);
    fitInView(rectVisible, Qt::KeepAspectRatio);
}

QPixmap VentanaNivel1::limpiarFondoAjedrez(const QPixmap& original) {
    QImage img = original.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        QRgb* linea = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            QColor c(linea[x]);
            int maxDiff = std::max({std::abs(c.red() - c.green()),
                                    std::abs(c.green() - c.blue()),
                                    std::abs(c.red() - c.blue())});
            if (maxDiff < 18 && c.red() > 110) {
                linea[x] = qRgba(0, 0, 0, 0);
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap VentanaNivel1::recortarBordesTransparentes(const QPixmap& pix) {
    QImage img = pix.toImage().convertToFormat(QImage::Format_ARGB32);
    int minX = img.width(), minY = img.height(), maxX = 0, maxY = 0;
    bool tienePixeles = false;

    for (int y = 0; y < img.height(); ++y) {
        const QRgb* linea = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            if (qAlpha(linea[x]) > 20) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
                tienePixeles = true;
            }
        }
    }

    if (!tienePixeles || maxX <= minX || maxY <= minY) {
        return pix;
    }
    return pix.copy(minX, minY, (maxX - minX) + 1, (maxY - minY) + 1);
}

void VentanaNivel1::construirEscenario() {
    // 1. Fondo completo del Nivel 1 en dos tramos para cubrir los 4500 px
    QPixmap fondoN1;
    try {
        if (!fondoN1.load(":/new/prefix1/FondoN1.jpg")) {
            throw std::runtime_error("No se pudo cargar FondoN1.jpg");
        }
        QPixmap tramoFondo = fondoN1.scaled(2250, 540, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (int x = 0; x < 4500; x += 2250) {
            QGraphicsPixmapItem* bgItem = escena->addPixmap(tramoFondo);
            bgItem->setPos(x, 0);
            bgItem->setZValue(-3);
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        escena->setBackgroundBrush(QBrush(QColor(135, 175, 205)));
    }

    // 2. Cargar :/new/prefix1/Suelo.png y recortar márgenes transparentes
    QPixmap texSuelo;
    bool tieneSuelo = false;
    try {
        if (!texSuelo.load(":/new/prefix1/Suelo.png")) {
            throw std::runtime_error("No se pudo cargar :/new/prefix1/Suelo.png");
        }
        texSuelo = limpiarFondoAjedrez(texSuelo);
        texSuelo = recortarBordesTransparentes(texSuelo);
        tieneSuelo = true;
    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    // 3. Piso continuo con Suelo.png desde Y = 465 hasta Y = 540 (sin separaciones)
    if (tieneSuelo) {
        int anchoTile = 90;
        int altoTile = 75;
        QPixmap tileSuelo = texSuelo.scaled(anchoTile, altoTile, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (int x = 0; x < 4500; x += (anchoTile - 2)) {
            QGraphicsPixmapItem* sueloItem = escena->addPixmap(tileSuelo);
            sueloItem->setPos(x, 465);
            sueloItem->setZValue(0);
        }
    } else {
        escena->addRect(0, 465, 4500, 12, QPen(Qt::NoPen), QBrush(QColor(56, 142, 60)));
        escena->addRect(0, 477, 4500, 63, QPen(Qt::NoPen), QBrush(QColor(92, 64, 51)));
    }

    // 4. Plataformas flotantes (continuas con Suelo.png) y zonas de fango
    for (const auto& obs : controlador->getMapaTerreno()) {
        if (obs.esFango) {
            escena->addRect(obs.area, QPen(Qt::NoPen), QBrush(QColor(65, 42, 20, 220)));
        } else {
            if (tieneSuelo) {
                int tileW = 55;
                QPixmap tilePlat = texSuelo.scaled(tileW, static_cast<int>(obs.area.height()),
                                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                for (double px = obs.area.left(); px < obs.area.right(); px += (tileW - 2)) {
                    QGraphicsPixmapItem* plat = escena->addPixmap(tilePlat);
                    plat->setPos(px, obs.area.y());
                    plat->setZValue(0);
                }
            } else {
                escena->addRect(obs.area, QPen(Qt::black, 2), QBrush(QColor(56, 142, 60)));
            }
        }
    }

    // 5. Zona Segura de Evacuación
    QRectF zona = controlador->getZonaEvacuacion();
    escena->addRect(zona, QPen(QPen(Qt::darkGreen, 3)), QBrush(QColor(46, 204, 113, 110)));
    QGraphicsTextItem* txtZona = escena->addText("ZONA SEGURA");
    txtZona->setDefaultTextColor(Qt::white);
    txtZona->setFont(QFont("Arial", 13, QFont::Bold));
    txtZona->setPos(zona.x() + 30, zona.y() - 28);

    // 6. HUD
    hudTexto = escena->addText("Tiempo: 03:00 | Rescatados: 0/5 | Estado: Normal");
    hudTexto->setDefaultTextColor(Qt::white);
    hudTexto->setFont(QFont("Arial", 16, QFont::Bold));
    hudTexto->setPos(20, 15);
    hudTexto->setZValue(10);
}

void VentanaNivel1::cargarSprites() {
    // --- A. CAPITÁN CORRIENDO (Matriz 4 cols x 3 filas) ---
    QPixmap sheetCorrer;
    try {
        if (!sheetCorrer.load(":/new/prefix1/Sprite_Correr.png")) {
            throw std::runtime_error("Error cargando Sprite_Correr.png");
        }
        int w = sheetCorrer.width() / 4;
        int h = sheetCorrer.height() / 3;
        for (int fila = 0; fila < 3; ++fila) {
            for (int col = 0; col < 4; ++col) {
                QPixmap frame = sheetCorrer.copy(col * w, fila * h, w, h)
                .scaled(95, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                framesCorrer.push_back(frame);
            }
        }
        pixQuieto = framesCorrer[0];
    } catch (const std::exception& e) {
        qWarning() << e.what();
        pixQuieto = QPixmap(75, 105);
        pixQuieto.fill(Qt::blue);
        framesCorrer.push_back(pixQuieto);
    }

    // --- B. CAPITÁN SALTANDO (7 fotogramas exactos con recorte interno limpio) ---
    QPixmap sheetSaltar;
    try {
        if (!sheetSaltar.load(":/new/prefix1/Sprite_Saltar.png")) {
            throw std::runtime_error("Error cargando Sprite_Saltar.png");
        }
        int cols = 7;
        double anchoCelda = static_cast<double>(sheetSaltar.width()) / cols;
        int h = sheetSaltar.height();
        int margen = static_cast<int>(anchoCelda * 0.05);

        for (int col = 0; col < cols; ++col) {
            int xInicio = static_cast<int>(col * anchoCelda) + margen;
            int anchoUtil = static_cast<int>(anchoCelda) - (margen * 2);
            QPixmap frame = sheetSaltar.copy(xInicio, 0, anchoUtil, h)
                                .scaled(95, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            framesSaltar.push_back(frame);
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        framesSaltar.push_back(pixQuieto);
    }

    // --- C. ESCUDO DE VIBRANIUM (4 cols x 4 filas) ---
    QPixmap sheetEscudo;
    try {
        if (!sheetEscudo.load(":/new/prefix1/SpritesEscudoAmerica.jpg")) {
            throw std::runtime_error("Error cargando SpritesEscudoAmerica.jpg");
        }
        QPixmap escudoSinFondo = limpiarFondoAjedrez(sheetEscudo);
        int w = escudoSinFondo.width() / 4;
        int h = escudoSinFondo.height() / 4;
        for (int fila = 0; fila < 4; ++fila) {
            for (int col = 0; col < 4; ++col) {
                QPixmap frame = escudoSinFondo.copy(col * w, fila * h, w, h)
                .scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                framesEscudo.push_back(frame);
            }
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        QPixmap fallback(32, 32);
        fallback.fill(Qt::red);
        framesEscudo.push_back(fallback);
    }

    // --- D. CAMPESINOS (4 cols x 3 filas) ---
    QPixmap sheetCampesinos;
    try {
        if (!sheetCampesinos.load(":/new/prefix1/spritesCampesinos.jpg")) {
            throw std::runtime_error("Error cargando spritesCampesinos.jpg");
        }
        QPixmap campSinFondo = limpiarFondoAjedrez(sheetCampesinos);
        int w = campSinFondo.width() / 4;
        int h = campSinFondo.height() / 3;
        for (int fila = 0; fila < 3; ++fila) {
            std::vector<QPixmap> animTipo;
            for (int col = 0; col < 4; ++col) {
                QPixmap frame = campSinFondo.copy(col * w, fila * h, w, h)
                .scaled(80, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                animTipo.push_back(frame);
            }
            framesCampesinos.push_back(animTipo);
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        QPixmap fallback(65, 95);
        fallback.fill(Qt::yellow);
        framesCampesinos.push_back({fallback});
    }

    itemHeroe = escena->addPixmap(pixQuieto);
    itemHeroe->setZValue(2);

    itemEscudo = escena->addPixmap(framesEscudo[0]);
    itemEscudo->setVisible(false);
    itemEscudo->setZValue(3);

    for (size_t i = 0; i < controlador->getCiviles().size(); ++i) {
        size_t tipo = i % framesCampesinos.size();
        QGraphicsPixmapItem* itemCivil = escena->addPixmap(framesCampesinos[tipo][0]);
        itemCivil->setZValue(1);
        itemsCiviles.push_back(itemCivil);
    }
}

void VentanaNivel1::actualizarAnimaciones(int direccion) {
    contadorTicks++;

    QPixmap frameHeroe = pixQuieto;
    CapitanAmerica* heroe = controlador->getHeroe();

    if (!heroe->isEnSuelo() && !framesSaltar.empty()) {
        float vy = heroe->getVelY();
        size_t idxSalto = 0;
        if (vy < -320.0f) idxSalto = 0;
        else if (vy < -180.0f) idxSalto = 1;
        else if (vy < -50.0f)  idxSalto = 2;
        else if (vy < 80.0f)   idxSalto = 3;
        else if (vy < 220.0f)  idxSalto = 4;
        else if (vy < 360.0f)  idxSalto = 5;
        else                   idxSalto = 6;

        frameHeroe = framesSaltar[idxSalto % framesSaltar.size()];
    } else if (direccion != 0 && !framesCorrer.empty()) {
        if (contadorTicks % 3 == 0) {
            frameHeroeActual = (frameHeroeActual + 1) % static_cast<int>(framesCorrer.size());
        }
        frameHeroe = framesCorrer[frameHeroeActual % framesCorrer.size()];
    } else {
        frameHeroeActual = 0;
        frameHeroe = pixQuieto;
    }

    if (!mirandoDerecha && !frameHeroe.isNull()) {
        frameHeroe = frameHeroe.transformed(QTransform().scale(-1, 1));
    }
    if (!frameHeroe.isNull()) {
        itemHeroe->setPixmap(frameHeroe);
    }

    if (heroe->getEscudo()->estaActivo() && !framesEscudo.empty()) {
        if (contadorTicks % 2 == 0) {
            frameEscudoActual = (frameEscudoActual + 1) % static_cast<int>(framesEscudo.size());
        }
        itemEscudo->setPixmap(framesEscudo[frameEscudoActual % framesEscudo.size()]);
    }

    if (contadorTicks % 6 == 0) {
        frameCivilActual++;
    }
    const auto& civiles = controlador->getCiviles();
    if (!framesCampesinos.empty()) {
        for (size_t i = 0; i < civiles.size() && i < itemsCiviles.size(); ++i) {
            if (!civiles[i]->isEvacuado()) {
                size_t tipo = i % framesCampesinos.size();
                if (!framesCampesinos[tipo].empty()) {
                    size_t f = civiles[i]->isRescatado()
                    ? (frameCivilActual % framesCampesinos[tipo].size())
                    : 0;
                    QPixmap pixC = framesCampesinos[tipo][f];
                    if (civiles[i]->isRescatado() && heroe->getPosX() < civiles[i]->getPosX()) {
                        pixC = pixC.transformed(QTransform().scale(-1, 1));
                    }
                    itemsCiviles[i]->setPixmap(pixC);
                }
            }
        }
    }
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

    actualizarAnimaciones(dir);

    CapitanAmerica* heroe = controlador->getHeroe();
    itemHeroe->setPos(heroe->getPosX(), heroe->getPosY());

    EscudoVibranium* escudo = heroe->getEscudo();
    itemEscudo->setVisible(escudo->estaActivo());
    if (escudo->estaActivo()) {
        itemEscudo->setPos(escudo->getPosX() - 18.0f, escudo->getPosY() - 18.0f);
    }

    const auto& civiles = controlador->getCiviles();
    for (size_t i = 0; i < civiles.size(); ++i) {
        itemsCiviles[i]->setVisible(!civiles[i]->isEvacuado());
        itemsCiviles[i]->setPos(civiles[i]->getPosX(), civiles[i]->getPosY());
    }

    centerOn(heroe->getPosX() + 260.0f, 290.0f);

    refrescarTextoHUD();
    hudTexto->setPos(mapToScene(25, 20));
}

void VentanaNivel1::refrescarTextoHUD() {
    int segundosRestantes = controlador->getTiempoRestante();
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