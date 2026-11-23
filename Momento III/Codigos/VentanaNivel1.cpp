#include "VentanaNivel1.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QMessageBox>
#include <QImage>
#include <QPainter>
#include <QTransform>
#include <QUrl>
#include <QDebug>
#include <stdexcept>
#include <algorithm>
#include <cmath>

VentanaNivel1::VentanaNivel1(QWidget* parent)
    : QGraphicsView(parent),
    ultimaVidaRenderizada(-1),
    conteoBalasPrevio(0),
    frameHeroeActual(0),
    frameGolpeActual(0),
    frameDefensaActual(0),
    frameEscudoActual(0),
    frameCivilActual(0),
    frameEnemigoActual(0),
    contadorTicks(0),
    mirandoDerecha(true),
    teclaIzquierda(false),
    teclaDerecha(false) {

    setMinimumSize(960, 540);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontSavePainterState, true);

    escena = new QGraphicsScene(0, 0, 6500, 540, this);
    escena->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(escena);

    controlador = new ControladorNivel1(this);

    inicializarAudio();
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

void VentanaNivel1::inicializarAudio() {
    // 1. Música de fondo del Nivel 1 (volumen reducido)
    musicaFondo = new QMediaPlayer(this);
    salidaMusicaFondo = new QAudioOutput(this);
    musicaFondo->setAudioOutput(salidaMusicaFondo);
    salidaMusicaFondo->setVolume(0.15f);
    musicaFondo->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/MusicaFondoN1.mp3"));
    musicaFondo->setLoops(QMediaPlayer::Infinite);
    musicaFondo->play();

    // 2. Efecto al caminar en lodo/fango (volumen aumentado al máximo)
    sfxLodo = new QMediaPlayer(this);
    salidaSfxLodo = new QAudioOutput(this);
    sfxLodo->setAudioOutput(salidaSfxLodo);
    salidaSfxLodo->setVolume(1.0f);
    sfxLodo->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/CaminoLodo.mp3"));
    sfxLodo->setLoops(QMediaPlayer::Infinite);

    // 3. Efecto de disparos enemigos
    sfxDisparo = new QMediaPlayer(this);
    salidaSfxDisparo = new QAudioOutput(this);
    sfxDisparo->setAudioOutput(salidaSfxDisparo);
    salidaSfxDisparo->setVolume(0.65f);
    sfxDisparo->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/DisparosSoldados.mp3"));

    // 4. Efecto de Game Over
    sfxGameOver = new QMediaPlayer(this);
    salidaSfxGameOver = new QAudioOutput(this);
    sfxGameOver->setAudioOutput(salidaSfxGameOver);
    salidaSfxGameOver->setVolume(0.85f);
    sfxGameOver->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/GameOver.mp3"));
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
            if ((maxDiff < 18 && c.red() > 110) || (maxDiff < 10 && c.red() < 28)) {
                linea[x] = qRgba(0, 0, 0, 0);
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap VentanaNivel1::limpiarVerdeChroma(const QPixmap& original) {
    QImage img = original.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        QRgb* linea = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            QColor c(linea[x]);
            if (c.alpha() > 0 && c.green() > 140 && c.green() > c.red() + 60 && c.green() > c.blue() + 60) {
                linea[x] = qRgba(0, 0, 0, 0);
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap VentanaNivel1::recortarBordesTransparentes(const QPixmap& pix, bool recortarAnchoExtra) {
    QImage img = pix.toImage().convertToFormat(QImage::Format_ARGB32);
    int w = img.width();
    int h = img.height();
    int minX = w, minY = h, maxX = 0, maxY = 0;
    bool tienePixeles = false;

    for (int y = 0; y < h; ++y) {
        const QRgb* linea = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            if (qAlpha(linea[x]) > 35) {
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

    int anchoRecorte = (maxX - minX) + 1;
    int altoRecorte  = (maxY - minY) + 1;
    if (recortarAnchoExtra && anchoRecorte > altoRecorte * 2) {
        minX = maxX - static_cast<int>(altoRecorte * 1.15);
        if (minX < 0) minX = 0;
        anchoRecorte = (maxX - minX) + 1;
    }

    return pix.copy(minX, minY, anchoRecorte, altoRecorte);
}

void VentanaNivel1::extraerSpritesPorBlob(const QPixmap& sheet, int yInicio, int yFin, int altoReferenciaMax, int altoObjetivo, std::vector<QPixmap>& destino) {
    QImage img = sheet.toImage().convertToFormat(QImage::Format_ARGB32);
    int w = img.width();
    bool enSprite = false;
    int inicioX = 0;

    std::vector<QPixmap> recortesCrudos;
    int maxAltoDetectado = 1;

    for (int x = 0; x < w; ++x) {
        int pixelesOpacos = 0;
        for (int y = yInicio; y < yFin; ++y) {
            if (qAlpha(img.pixel(x, y)) > 35) {
                pixelesOpacos++;
            }
        }
        if (pixelesOpacos > 2 && !enSprite) {
            enSprite = true;
            inicioX = x;
        } else if (pixelesOpacos <= 2 && enSprite) {
            enSprite = false;
            int anchoSprite = x - inicioX;
            if (anchoSprite > 22) {
                QPixmap recorte = recortarBordesTransparentes(sheet.copy(inicioX, yInicio, anchoSprite, yFin - yInicio));
                if (!recorte.isNull()) {
                    maxAltoDetectado = std::max(maxAltoDetectado, recorte.height());
                    recortesCrudos.push_back(recorte);
                }
            }
        }
    }

    int baseAltura = (altoReferenciaMax > 0) ? altoReferenciaMax : maxAltoDetectado;
    double factorEscala = static_cast<double>(altoObjetivo) / static_cast<double>(baseAltura);

    for (const QPixmap& rec : recortesCrudos) {
        int nuevoAncho = std::max(1, static_cast<int>(std::round(rec.width() * factorEscala)));
        int nuevoAlto  = std::max(1, static_cast<int>(std::round(rec.height() * factorEscala)));
        destino.push_back(rec.scaled(nuevoAncho, nuevoAlto, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}

void VentanaNivel1::actualizarBarraVidaPixelArt(int vidaActual, int vidaMax) {
    if (vidaActual == ultimaVidaRenderizada) return;
    ultimaVidaRenderizada = vidaActual;

    const int w = 244;
    const int h = 28;
    QPixmap pix(w, h);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, false);

    p.fillRect(4, 0, w - 8, h, QColor(15, 15, 20));
    p.fillRect(0, 4, w, h - 8, QColor(15, 15, 20));
    p.fillRect(2, 2, w - 4, h - 4, QColor(15, 15, 20));

    p.fillRect(4, 2, w - 8, 2, QColor(235, 215, 140));
    p.fillRect(4, h - 4, w - 8, 2, QColor(150, 125, 65));
    p.fillRect(2, 4, 2, h - 8, QColor(235, 215, 140));
    p.fillRect(w - 4, 4, 2, h - 8, QColor(150, 125, 65));

    int innerX = 6;
    int innerY = 6;
    int innerW = w - 12;
    int innerH = h - 12;
    p.fillRect(innerX, innerY, innerW, innerH, QColor(55, 18, 22));

    float pct = (vidaMax > 0) ? std::clamp(static_cast<float>(vidaActual) / static_cast<float>(vidaMax), 0.0f, 1.0f) : 0.0f;
    int fillW = static_cast<int>(std::round((innerW * pct) / 2.0)) * 2;

    if (fillW > 0) {
        QColor colorBase, colorBrillo, colorSombra;
        if (pct > 0.5f) {
            colorBase   = QColor(46, 204, 113);
            colorBrillo = QColor(130, 245, 175);
            colorSombra = QColor(24, 135, 70);
        } else if (pct > 0.25f) {
            colorBase   = QColor(241, 196, 15);
            colorBrillo = QColor(255, 238, 120);
            colorSombra = QColor(175, 130, 10);
        } else {
            colorBase   = QColor(231, 76, 60);
            colorBrillo = QColor(255, 145, 130);
            colorSombra = QColor(155, 35, 25);
        }

        p.fillRect(innerX, innerY, fillW, innerH, colorBase);
        p.fillRect(innerX, innerY, fillW, 3, colorBrillo);
        p.fillRect(innerX, innerY + innerH - 3, fillW, 3, colorSombra);
    }

    int segmentos = 10;
    for (int i = 1; i < segmentos; ++i) {
        int sx = innerX + (innerW * i) / segmentos;
        p.fillRect(sx, innerY, 2, innerH, QColor(15, 15, 20, 190));
    }

    QFont fontPixel("Courier New", 10, QFont::Bold);
    fontPixel.setStyleStrategy(QFont::NoAntialias);
    p.setFont(fontPixel);
    QString etiqueta = QString("HP %1/%2").arg(vidaActual).arg(vidaMax);

    p.setPen(QColor(10, 10, 15));
    p.drawText(QRect(innerX + 2, innerY + 1, innerW, innerH), Qt::AlignCenter, etiqueta);
    p.setPen(Qt::white);
    p.drawText(QRect(innerX, innerY - 1, innerW, innerH), Qt::AlignCenter, etiqueta);

    p.end();
    itemBarraVida->setPixmap(pix);
}

void VentanaNivel1::construirEscenario() {
    // 1. Fondo del Nivel 1 en 3 tramos para cubrir 6500 px
    QPixmap fondoN1;
    try {
        if (!fondoN1.load(":/new/prefix1/FondoN1.jpg")) {
            throw std::runtime_error("No se pudo cargar FondoN1.jpg");
        }
        QPixmap tramoFondo = fondoN1.scaled(2200, 540, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (int x = 0; x < 6600; x += 2200) {
            QGraphicsPixmapItem* bgItem = escena->addPixmap(tramoFondo);
            bgItem->setPos(x, 0);
            bgItem->setZValue(-3);
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        escena->setBackgroundBrush(QBrush(QColor(135, 175, 205)));
    }

    // 2. Suelo continuo con :/new/prefix1/Suelo.png hasta 6500 px
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

    if (tieneSuelo) {
        int anchoTile = 90;
        int altoTile = 75;
        QPixmap tileSuelo = texSuelo.scaled(anchoTile, altoTile, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (int x = 0; x < 6500; x += (anchoTile - 2)) {
            QGraphicsPixmapItem* sueloItem = escena->addPixmap(tileSuelo);
            sueloItem->setPos(x, 465);
            sueloItem->setZValue(0);
        }
    } else {
        escena->addRect(0, 465, 6500, 12, QPen(Qt::NoPen), QBrush(QColor(56, 142, 60)));
        escena->addRect(0, 477, 6500, 63, QPen(Qt::NoPen), QBrush(QColor(92, 64, 51)));
    }

    // 3. Cargar textura completa de Obstaculo.png (sin recortar su ancho)
    QPixmap texObstaculo;
    bool tieneObstaculo = false;
    try {
        if (!texObstaculo.load(":/new/prefix1/Obstaculo.png")) {
            throw std::runtime_error("No se pudo cargar :/new/prefix1/Obstaculo.png");
        }
        texObstaculo = limpiarFondoAjedrez(texObstaculo);
        texObstaculo = recortarBordesTransparentes(texObstaculo, false);
        tieneObstaculo = !texObstaculo.isNull();
    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    // 4. Plataformas flotantes (Suelo.png), zonas de fango y obstáculos sólidos (Obstaculo.png)
    for (const auto& obs : controlador->getMapaTerreno()) {
        if (obs.esFango) {
            escena->addRect(obs.area, QPen(Qt::NoPen), QBrush(QColor(65, 42, 20, 220)));
        } else if (obs.esBloqueSalto) {
            if (tieneObstaculo) {
                QPixmap pixObs = texObstaculo.scaled(static_cast<int>(obs.area.width()),
                                                     static_cast<int>(obs.area.height()),
                                                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                QGraphicsPixmapItem* itemObs = escena->addPixmap(pixObs);
                itemObs->setPos(obs.area.x(), obs.area.y());
                itemObs->setZValue(1);
            } else {
                escena->addRect(obs.area, QPen(Qt::black, 2), QBrush(QColor(139, 90, 43)));
            }
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

    // 5. Casa.png detrás de la posición inicial de cada campesino
    QPixmap texCasa;
    try {
        if (!texCasa.load(":/new/prefix1/Casa.png")) {
            throw std::runtime_error("No se pudo cargar :/new/prefix1/Casa.png");
        }
        texCasa = limpiarFondoAjedrez(texCasa);
        texCasa = recortarBordesTransparentes(texCasa, false);
        if (!texCasa.isNull()) {
            QPixmap casaEscalada = texCasa.scaledToHeight(135, Qt::SmoothTransformation);
            for (const Civil* civil : controlador->getCiviles()) {
                QGraphicsPixmapItem* itemCasa = escena->addPixmap(casaEscalada);
                double baseSueloCivil = civil->getPosY() + 95.0;
                double casaX = civil->getPosX() - (casaEscalada.width() * 0.35);
                double casaY = baseSueloCivil - casaEscalada.height() + 4.0;
                itemCasa->setPos(casaX, casaY);
                itemCasa->setZValue(0.2);
            }
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    // 6. Cartel de indicaciones de controles en el escenario
    {
        const int cw = 310;
        const int ch = 122;
        QPixmap cartelPix(cw, ch);
        cartelPix.fill(Qt::transparent);

        QPainter p(&cartelPix);
        p.setRenderHint(QPainter::Antialiasing, false);

        p.fillRect(4, 0, cw - 8, ch, QColor(20, 18, 15, 215));
        p.fillRect(0, 4, cw, ch - 8, QColor(20, 18, 15, 215));
        p.fillRect(4, 2, cw - 8, 2, QColor(210, 180, 110));
        p.fillRect(4, ch - 4, cw - 8, 2, QColor(130, 105, 55));
        p.fillRect(2, 4, 2, ch - 8, QColor(210, 180, 110));
        p.fillRect(cw - 4, 4, 2, ch - 8, QColor(130, 105, 55));

        QFont fTitulo("Courier New", 10, QFont::Bold);
        fTitulo.setStyleStrategy(QFont::NoAntialias);
        p.setFont(fTitulo);
        p.setPen(QColor(255, 220, 95));
        p.drawText(QRect(12, 8, cw - 24, 18), Qt::AlignLeft | Qt::AlignVCenter, "INDICACIONES DE COMBATE:");

        QFont fTexto("Courier New", 9, QFont::Bold);
        fTexto.setStyleStrategy(QFont::NoAntialias);
        p.setFont(fTexto);
        p.setPen(QColor(240, 240, 240));
        p.drawText(14, 40, "[A / D] o Flechas : Moverse");
        p.drawText(14, 58, "[W / Espacio]     : Saltar");
        p.drawText(14, 76, "[H]               : Golpe cuerpo a cuerpo");
        p.drawText(14, 94, "[J] Lanzar Escudo | [K] Cubrirse");
        p.end();

        QGraphicsPixmapItem* itemCartel = escena->addPixmap(cartelPix);
        itemCartel->setPos(45, 195);
        itemCartel->setZValue(-1);
    }

    // 7. Zona Segura
    QRectF zona = controlador->getZonaEvacuacion();
    escena->addRect(zona, QPen(QPen(Qt::darkGreen, 3)), QBrush(QColor(46, 204, 113, 110)));
    QGraphicsTextItem* txtZona = escena->addText("ZONA SEGURA");
    txtZona->setDefaultTextColor(Qt::white);
    txtZona->setFont(QFont("Arial", 13, QFont::Bold));
    txtZona->setPos(zona.x() + 45, zona.y() - 28);

    // 8. HUD superior y Barra de Vida Pixel Art
    hudTexto = escena->addText("Tiempo: 03:30 | Rescatados: 0/5 | Estado: Normal");
    hudTexto->setDefaultTextColor(Qt::white);
    hudTexto->setFont(QFont("Arial", 16, QFont::Bold));
    hudTexto->setZValue(10);

    itemBarraVida = escena->addPixmap(QPixmap());
    itemBarraVida->setZValue(11);
    actualizarBarraVidaPixelArt(controlador->getHeroe()->getVidaActual(), controlador->getHeroe()->getVidaMax());
}

void VentanaNivel1::cargarSprites() {
    const int ALTURA_CORRER   = 76;
    const int ALTURA_ESTANDAR = 88;

    // --- A. CAPITÁN CORRIENDO ---
    QPixmap sheetCorrer;
    try {
        if (!sheetCorrer.load(":/new/prefix1/Sprite_Correr.png")) {
            throw std::runtime_error("Error cargando :/new/prefix1/Sprite_Correr.png");
        }
        int w = sheetCorrer.width() / 4;
        int h = sheetCorrer.height() / 3;
        for (int fila = 0; fila < 3; ++fila) {
            for (int col = 0; col < 4; ++col) {
                QPixmap celda = sheetCorrer.copy(col * w, fila * h, w, h);
                QImage imgCelda = celda.toImage().convertToFormat(QImage::Format_ARGB32);

                int pixelesHeroe = 0;
                for (int y = 0; y < imgCelda.height(); y += 2) {
                    const QRgb* line = reinterpret_cast<const QRgb*>(imgCelda.constScanLine(y));
                    for (int x = 0; x < imgCelda.width(); x += 2) {
                        QColor c(line[x]);
                        if (c.alpha() > 80) {
                            bool esAzulTraje = (c.blue() > c.red() + 35 && c.blue() > 90);
                            bool esRojoTraje = (c.red() > c.blue() + 45 && c.red() > 110 && c.green() < 90);
                            if (esAzulTraje || esRojoTraje) {
                                pixelesHeroe++;
                            }
                        }
                    }
                }

                if (pixelesHeroe > 35) {
                    QPixmap recorte = recortarBordesTransparentes(celda);
                    framesCorrer.push_back(recorte.scaledToHeight(ALTURA_CORRER, Qt::SmoothTransformation));
                }
            }
        }
        if (framesCorrer.empty()) {
            throw std::runtime_error("No se detectaron frames validos en Sprite_Correr.png");
        }
        pixQuieto = framesCorrer[0];
    } catch (const std::exception& e) {
        qWarning() << e.what();
        pixQuieto = QPixmap(60, ALTURA_CORRER);
        pixQuieto.fill(Qt::blue);
        framesCorrer.push_back(pixQuieto);
    }

    // --- B. CAPITÁN SALTANDO ---
    QPixmap sheetSaltar;
    try {
        if (!sheetSaltar.load(":/new/prefix1/Sprite_Saltar.png")) {
            throw std::runtime_error("Error cargando :/new/prefix1/Sprite_Saltar.png");
        }
        QPixmap saltoLimpio = limpiarVerdeChroma(sheetSaltar);
        extraerSpritesPorBlob(saltoLimpio, 0, saltoLimpio.height(), 145, ALTURA_ESTANDAR, framesSaltar);

        if (framesSaltar.empty()) {
            framesSaltar.push_back(pixQuieto);
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        framesSaltar.push_back(pixQuieto);
    }

    // --- C. CAPITÁN PELEA CUERPO A CUERPO ---
    QPixmap sheetLucha;
    try {
        if (!sheetLucha.load(":/new/prefix1/Lucha_cuerpoACuerpo.png")) {
            throw std::runtime_error("Error cargando Lucha_cuerpoACuerpo.png");
        }
        int mitadY = sheetLucha.height() / 2;
        extraerSpritesPorBlob(sheetLucha, mitadY, sheetLucha.height(), 105, ALTURA_ESTANDAR, framesGolpear);
        extraerSpritesPorBlob(sheetLucha, 0, mitadY, 105, ALTURA_ESTANDAR, framesGolpear);
        if (framesGolpear.empty()) framesGolpear.push_back(pixQuieto);
    } catch (const std::exception& e) {
        qWarning() << e.what();
        framesGolpear.push_back(pixQuieto);
    }

    // --- D. CAPITÁN DEFENSA / CUBRIRSE ---
    QPixmap sheetDefensa;
    try {
        if (!sheetDefensa.load(":/new/prefix1/DefensaC.png")) {
            throw std::runtime_error("Error cargando DefensaC.png");
        }
        extraerSpritesPorBlob(sheetDefensa, 0, sheetDefensa.height(), 0, ALTURA_ESTANDAR, framesDefensa);
        if (framesDefensa.empty()) framesDefensa.push_back(pixQuieto);
    } catch (const std::exception& e) {
        qWarning() << e.what();
        framesDefensa.push_back(pixQuieto);
    }

    // --- E. ESCUDO DE VIBRANIUM ---
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
                framesEscudo.push_back(
                    escudoSinFondo.copy(col * w, fila * h, w, h).scaled(34, 34, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                    );
            }
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        QPixmap fallback(28, 28);
        fallback.fill(Qt::red);
        framesEscudo.push_back(fallback);
    }

    // --- F. CAMPESINOS ---
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
                QPixmap celda = campSinFondo.copy(col * w + 4, fila * h + 4, w - 8, h - 8);
                QPixmap recorte = recortarBordesTransparentes(celda);
                animTipo.push_back(recorte.scaledToHeight(ALTURA_ESTANDAR, Qt::SmoothTransformation));
            }
            framesCampesinos.push_back(animTipo);
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        QPixmap fallback(55, ALTURA_ESTANDAR);
        fallback.fill(Qt::yellow);
        framesCampesinos.push_back({fallback});
    }

    // --- G. SOLDADOS ENEMIGOS ---
    QPixmap sheetEnemigo;
    try {
        if (!sheetEnemigo.load(":/new/prefix1/Sprite_Enemigo.png")) {
            throw std::runtime_error("Error cargando :/new/prefix1/Sprite_Enemigo.png");
        }
        QPixmap enemigoSinFondo = limpiarFondoAjedrez(sheetEnemigo);
        int w = enemigoSinFondo.width() / 6;
        int h = enemigoSinFondo.height() / 3;

        for (int col = 0; col < 6; ++col) {
            QPixmap recorte = recortarBordesTransparentes(enemigoSinFondo.copy(col * w, 0, w, h));
            framesEnemigoCaminar.push_back(recorte.scaledToHeight(ALTURA_ESTANDAR, Qt::SmoothTransformation));
        }
        for (int col = 0; col < 6; ++col) {
            QPixmap recorte = recortarBordesTransparentes(enemigoSinFondo.copy(col * w, h, w, h));
            framesEnemigoDisparar.push_back(recorte.scaledToHeight(ALTURA_ESTANDAR, Qt::SmoothTransformation));
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
        QPixmap fallback(55, ALTURA_ESTANDAR);
        fallback.fill(Qt::darkRed);
        framesEnemigoCaminar.push_back(fallback);
        framesEnemigoDisparar.push_back(fallback);
    }

    pixBalaEnemiga = QPixmap(36, 14);
    pixBalaEnemiga.fill(Qt::transparent);
    {
        QPainter p(&pixBalaEnemiga);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 87, 34, 180));
        p.drawEllipse(0, 3, 18, 8);
        p.setPen(QPen(Qt::black, 1.5));
        p.setBrush(QColor(255, 235, 59));
        p.drawRoundedRect(12, 2, 22, 10, 5, 5);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawEllipse(26, 4, 6, 6);
    }

    itemHeroe = escena->addPixmap(pixQuieto);
    itemHeroe->setZValue(4);

    itemEscudo = escena->addPixmap(framesEscudo[0]);
    itemEscudo->setVisible(false);
    itemEscudo->setZValue(5);

    for (size_t i = 0; i < controlador->getCiviles().size(); ++i) {
        size_t tipo = i % framesCampesinos.size();
        QGraphicsPixmapItem* itemCivil = escena->addPixmap(framesCampesinos[tipo][0]);
        itemCivil->setZValue(1.0 + (i * 0.1));
        itemsCiviles.push_back(itemCivil);
    }

    for (size_t i = 0; i < controlador->getEnemigos().size(); ++i) {
        QGraphicsPixmapItem* itemEnem = escena->addPixmap(framesEnemigoCaminar[0]);
        itemEnem->setZValue(3);
        itemsEnemigos.push_back(itemEnem);
    }

    for (int i = 0; i < 40; ++i) {
        QGraphicsPixmapItem* bItem = escena->addPixmap(pixBalaEnemiga);
        bItem->setVisible(false);
        bItem->setZValue(6);
        poolBalasEnemigas.push_back(bItem);
    }
}

void VentanaNivel1::actualizarAnimaciones(int direccion) {
    contadorTicks++;

    QPixmap frameHeroe = pixQuieto;
    CapitanAmerica* heroe = controlador->getHeroe();

    if (heroe->isBloqueando() && !framesDefensa.empty()) {
        int ultimoFrame = static_cast<int>(framesDefensa.size()) - 1;
        if (contadorTicks % 14 == 0 && frameDefensaActual < ultimoFrame) {
            frameDefensaActual++;
        }
        frameHeroe = framesDefensa[frameDefensaActual];
    } else if (heroe->isAtacandoCuerpoACuerpo() && !framesGolpear.empty()) {
        frameDefensaActual = 0;
        if (contadorTicks % 3 == 0) {
            frameGolpeActual = (frameGolpeActual + 1) % static_cast<int>(framesGolpear.size());
        }
        frameHeroe = framesGolpear[frameGolpeActual % framesGolpear.size()];
    } else if (!heroe->isEnSuelo() && !framesSaltar.empty()) {
        frameDefensaActual = 0;
        frameGolpeActual = 0;
        float vy = heroe->getVelY();
        int total = static_cast<int>(framesSaltar.size());

        float progreso = (vy + 560.0f) / 1120.0f;
        if (progreso < 0.0f) progreso = 0.0f;
        if (progreso > 0.999f) progreso = 0.999f;
        int idxSalto = static_cast<int>(progreso * total);

        frameHeroe = framesSaltar[idxSalto % total];
    } else if (direccion != 0 && !framesCorrer.empty()) {
        frameDefensaActual = 0;
        frameGolpeActual = 0;
        if (contadorTicks % 3 == 0) {
            frameHeroeActual = (frameHeroeActual + 1) % static_cast<int>(framesCorrer.size());
        }
        frameHeroe = framesCorrer[frameHeroeActual % framesCorrer.size()];
    } else {
        frameDefensaActual = 0;
        frameGolpeActual = 0;
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

    if (contadorTicks % 7 == 0) frameCivilActual++;
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
                    if (civiles[i]->isRescatado() && heroe->getPosX() < civiles[i]->getPosX() && heroe->getPosX() < 5950.0f) {
                        pixC = pixC.transformed(QTransform().scale(-1, 1));
                    }
                    itemsCiviles[i]->setPixmap(pixC);
                }
            }
        }
    }

    if (contadorTicks % 5 == 0) frameEnemigoActual++;
    const auto& enemigos = controlador->getEnemigos();
    for (size_t i = 0; i < enemigos.size() && i < itemsEnemigos.size(); ++i) {
        if (!enemigos[i]->isVivo()) {
            itemsEnemigos[i]->setVisible(false);
            continue;
        }
        itemsEnemigos[i]->setVisible(true);
        const auto& listaFrames = enemigos[i]->isDisparando() ? framesEnemigoDisparar : framesEnemigoCaminar;
        if (!listaFrames.empty()) {
            QPixmap pixE = listaFrames[frameEnemigoActual % listaFrames.size()];
            if (enemigos[i]->getDireccion() < 0) {
                pixE = pixE.transformed(QTransform().scale(-1, 1));
            }
            itemsEnemigos[i]->setPixmap(pixE);
        }
        double hEnem = itemsEnemigos[i]->pixmap().height();
        itemsEnemigos[i]->setPos(enemigos[i]->getPosX(), (enemigos[i]->getPosY() + 95.0) - hEnem + 4.0);
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
    case Qt::Key_H:
        controlador->getHeroe()->iniciarGolpeCuerpoACuerpo();
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

    CapitanAmerica* heroe = controlador->getHeroe();

    // Sonido CaminoLodo.mp3 cuando el héroe camina sobre fango
    if (friccion > 0.0f && dir != 0 && heroe->isEnSuelo()) {
        if (sfxLodo->playbackState() != QMediaPlayer::PlayingState) {
            sfxLodo->play();
        }
    } else if (sfxLodo->playbackState() == QMediaPlayer::PlayingState) {
        sfxLodo->stop();
    }

    heroe->moverHorizontal(dir, friccion);
    controlador->actualizarReglas(dt);

    actualizarAnimaciones(dir);

    double anchoHeroePix = itemHeroe->pixmap().width();
    double altoHeroePix  = itemHeroe->pixmap().height();
    double xCentradaHeroe = heroe->getPosX() + (70.0 - anchoHeroePix) * 0.5;
    double yAlineadaHeroe = (heroe->getPosY() + 95.0) - altoHeroePix + 4.0;
    itemHeroe->setPos(xCentradaHeroe, yAlineadaHeroe);

    EscudoVibranium* escudo = heroe->getEscudo();
    itemEscudo->setVisible(escudo->estaActivo());
    if (escudo->estaActivo()) {
        itemEscudo->setPos(escudo->getPosX() - 16.0f, escudo->getPosY() - 16.0f);
    }

    const auto& civiles = controlador->getCiviles();
    for (size_t i = 0; i < civiles.size(); ++i) {
        itemsCiviles[i]->setVisible(!civiles[i]->isEvacuado());
        double altoCivilPix = itemsCiviles[i]->pixmap().height();
        double yAlineadaCivil = (civiles[i]->getPosY() + 95.0) - altoCivilPix + 4.0;
        itemsCiviles[i]->setPos(civiles[i]->getPosX(), yAlineadaCivil);
    }

    // Reproducir DisparosSoldados.mp3 cuando se dispara una bala nueva
    const auto& balas = controlador->getBalasEnemigas();
    if (balas.size() > conteoBalasPrevio) {
        sfxDisparo->setPosition(0);
        sfxDisparo->play();
    }
    conteoBalasPrevio = balas.size();

    for (size_t i = 0; i < poolBalasEnemigas.size(); ++i) {
        if (i < balas.size() && balas[i].activa) {
            poolBalasEnemigas[i]->setVisible(true);
            QPixmap pBala = pixBalaEnemiga;
            if (balas[i].vx < 0) {
                pBala = pBala.transformed(QTransform().scale(-1, 1));
            }
            poolBalasEnemigas[i]->setPixmap(pBala);
            poolBalasEnemigas[i]->setPos(balas[i].x, balas[i].y);
        } else {
            poolBalasEnemigas[i]->setVisible(false);
        }
    }

    centerOn(heroe->getPosX() + 260.0f, 290.0f);

    refrescarTextoHUD();
    hudTexto->setPos(mapToScene(25, 16));

    actualizarBarraVidaPixelArt(heroe->getVidaActual(), heroe->getVidaMax());
    itemBarraVida->setPos(mapToScene(25, 66));
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
    musicaFondo->stop();
    sfxLodo->stop();

    if (victoria) {
        QMessageBox::information(this, "¡Pueblo a Salvo!",
                                 "Has evacuado a los 5 grupos de civiles.\n\n"
                                 "Cinemática: En el futuro, Donald Trump observa el mapa de Colombia y Panamá:\n"
                                 "«Que Colombia pelee entre ellos. Me quedaré con Panamá».");
    } else {
        sfxGameOver->play();
        QMessageBox::warning(this, "Misión Fallida", "Has perdido toda tu salud o se agotó el tiempo.");
    }
}