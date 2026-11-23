#include "VentanaNivel2.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QImage>
#include <QPainter>
#include <QTransform>
#include <QUrl>
#include <QDebug>
#include <stdexcept>
#include <algorithm>
#include <cmath>

VentanaNivel2::VentanaNivel2(QWidget* parent)
    : QGraphicsView(parent),
    ultimaVidaHeroeRender(-1),
    ultimaVidaJefeRender(-1),
    vidaPreviaHeroe(-1),
    overlayFinNivel(nullptr),
    conteoDisparosPrevio(0),
    frameHeroeActual(0),
    frameGolpeActual(0),
    frameDefensaActual(0),
    frameEscudoActual(0),
    frameTrumpActual(0),
    contadorTicks(0),
    mirandoDerecha(true),
    teclaIzquierda(false),
    teclaDerecha(false),
    teclaAbajo(false) {

    setMinimumSize(960, 540);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
    setFocusPolicy(Qt::StrongFocus);

    escena = new QGraphicsScene(0, -100, 1280, 640, this);
    escena->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(escena);

    controlador = new ControladorNivel2(this);
    vidaPreviaHeroe = controlador->getHeroe()->getVidaActual();

    inicializarAudio();
    construirEscenario();
    cargarSprites();

    connect(controlador, &ControladorNivel2::nivelFinalizado, this, &VentanaNivel2::manejarFinNivel);

    gameLoopTimer = new QTimer(this);
    connect(gameLoopTimer, &QTimer::timeout, this, &VentanaNivel2::actualizarFrame);

    controlador->iniciarReloj();
    gameLoopTimer->start(16);
}

VentanaNivel2::~VentanaNivel2() {
    delete escena;
}

void VentanaNivel2::showEvent(QShowEvent* event) {
    QGraphicsView::showEvent(event);
    fitInView(QRectF(0, -100, 1280, 640), Qt::KeepAspectRatio);
    setFocus();
}

void VentanaNivel2::closeEvent(QCloseEvent* event) {
    gameLoopTimer->stop();
    musicaFondo->stop();
    emit regresarAlMenu();
    event->accept();
}

void VentanaNivel2::inicializarAudio() {
    musicaFondo = new QMediaPlayer(this);
    salidaMusicaFondo = new QAudioOutput(this);
    musicaFondo->setAudioOutput(salidaMusicaFondo);
    salidaMusicaFondo->setVolume(0.18f);
    musicaFondo->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/MusicaFondoN1.mp3"));
    musicaFondo->setLoops(QMediaPlayer::Infinite);
    musicaFondo->play();

    sfxDisparo = new QMediaPlayer(this);
    salidaSfxDisparo = new QAudioOutput(this);
    sfxDisparo->setAudioOutput(salidaSfxDisparo);
    salidaSfxDisparo->setVolume(0.65f);
    sfxDisparo->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/DisparosSoldados.mp3"));

    sfxGameOver = new QMediaPlayer(this);
    salidaSfxGameOver = new QAudioOutput(this);
    sfxGameOver->setAudioOutput(salidaSfxGameOver);
    salidaSfxGameOver->setVolume(0.85f);
    sfxGameOver->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/GameOver.mp3"));

    sfxSaltar = new QMediaPlayer(this);
    salidaSfxSaltar = new QAudioOutput(this);
    sfxSaltar->setAudioOutput(salidaSfxSaltar);
    salidaSfxSaltar->setVolume(0.80f);
    sfxSaltar->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/Saltar.mp3"));

    sfxRecibeDano = new QMediaPlayer(this);
    salidaSfxRecibeDano = new QAudioOutput(this);
    sfxRecibeDano->setAudioOutput(salidaSfxRecibeDano);
    salidaSfxRecibeDano->setVolume(0.90f);
    sfxRecibeDano->setSource(QUrl("qrc:/new/prefix1/EfectosSonido/RecibeDaño.mp3"));
}

void VentanaNivel2::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (overlayFinNivel) {
        overlayFinNivel->setGeometry(viewport()->rect());
    }
    if (viewport()->height() <= 0 || viewport()->width() <= 0) return;

    fitInView(QRectF(0, -100, 1280, 640), Qt::KeepAspectRatio);
}

QPixmap VentanaNivel2::limpiarFondoAjedrez(const QPixmap& original) {
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

QPixmap VentanaNivel2::limpiarVerdeChroma(const QPixmap& original) {
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

QPixmap VentanaNivel2::limpiarFondoOscuro(const QPixmap& original) {
    QImage img = original.toImage().convertToFormat(QImage::Format_ARGB32);
    if (img.isNull()) return original;

    QColor bg(img.pixel(0, 0));
    for (int y = 0; y < img.height(); ++y) {
        QRgb* linea = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            QColor c(linea[x]);
            int dist = std::max({std::abs(c.red() - bg.red()),
                                 std::abs(c.green() - bg.green()),
                                 std::abs(c.blue() - bg.blue())});
            if (dist < 26) {
                linea[x] = qRgba(0, 0, 0, 0);
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap VentanaNivel2::recortarBordesTransparentes(const QPixmap& pix, bool recortarAnchoExtra) {
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

    if (!tienePixeles || maxX <= minX || maxY <= minY) return pix;

    int anchoRecorte = (maxX - minX) + 1;
    int altoRecorte  = (maxY - minY) + 1;
    if (recortarAnchoExtra && anchoRecorte > altoRecorte * 2) {
        minX = maxX - static_cast<int>(altoRecorte * 1.15);
        if (minX < 0) minX = 0;
        anchoRecorte = (maxX - minX) + 1;
    }

    return pix.copy(minX, minY, anchoRecorte, altoRecorte);
}

void VentanaNivel2::extraerSpritesPorBlob(const QPixmap& sheet, int yInicio, int yFin, int altoReferenciaMax, int altoObjetivo, std::vector<QPixmap>& destino) {
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

void VentanaNivel2::actualizarBarraVida(QGraphicsPixmapItem* target, int& cacheVida, int vidaActual, int vidaMax, const QString& prefijo, bool esJefe) {
    if (vidaActual == cacheVida) return;
    cacheVida = vidaActual;

    const int w = esJefe ? 340 : 250;
    const int h = 28;
    QPixmap pix(w, h);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, false);

    p.fillRect(4, 0, w - 8, h, QColor(15, 15, 20));
    p.fillRect(0, 4, w, h - 8, QColor(15, 15, 20));
    p.fillRect(2, 2, w - 4, h - 4, QColor(15, 15, 20));

    QColor bordeClaro = esJefe ? QColor(241, 196, 15) : QColor(235, 215, 140);
    QColor bordeOsc   = esJefe ? QColor(183, 149, 11) : QColor(150, 125, 65);
    p.fillRect(4, 2, w - 8, 2, bordeClaro);
    p.fillRect(4, h - 4, w - 8, 2, bordeOsc);
    p.fillRect(2, 4, 2, h - 8, bordeClaro);
    p.fillRect(w - 4, 4, 2, h - 8, bordeOsc);

    int innerX = 6;
    int innerY = 6;
    int innerW = w - 12;
    int innerH = h - 12;
    p.fillRect(innerX, innerY, innerW, innerH, QColor(55, 18, 22));

    float pct = (vidaMax > 0) ? std::clamp(static_cast<float>(vidaActual) / static_cast<float>(vidaMax), 0.0f, 1.0f) : 0.0f;
    int fillW = static_cast<int>(std::round((innerW * pct) / 2.0)) * 2;

    if (fillW > 0) {
        QColor colorBase, colorBrillo, colorSombra;
        if (esJefe) {
            if (pct > 0.5f) {
                colorBase   = QColor(192, 57, 43);
                colorBrillo = QColor(241, 148, 138);
                colorSombra = QColor(120, 40, 31);
            } else {
                colorBase   = QColor(142, 68, 173);
                colorBrillo = QColor(210, 180, 222);
                colorSombra = QColor(91, 44, 111);
            }
        } else {
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
        }

        p.fillRect(innerX, innerY, fillW, innerH, colorBase);
        p.fillRect(innerX, innerY, fillW, 3, colorBrillo);
        p.fillRect(innerX, innerY + innerH - 3, fillW, 3, colorSombra);
    }

    QFont fontPixel("Courier New", 10, QFont::Bold);
    fontPixel.setStyleStrategy(QFont::NoAntialias);
    p.setFont(fontPixel);
    QString etiqueta = QString("%1 %2/%3").arg(prefijo).arg(vidaActual).arg(vidaMax);

    p.setPen(QColor(10, 10, 15));
    p.drawText(QRect(innerX + 2, innerY + 1, innerW, innerH), Qt::AlignCenter, etiqueta);
    p.setPen(Qt::white);
    p.drawText(QRect(innerX, innerY - 1, innerW, innerH), Qt::AlignCenter, etiqueta);

    p.end();
    target->setPixmap(pix);
}

void VentanaNivel2::construirEscenario() {
    escena->setBackgroundBrush(QBrush(QColor(22, 34, 48)));
    QPixmap fondoN2;
    try {
        if (!fondoN2.load(":/new/prefix1/FondoN2.png")) {
            throw std::runtime_error("No se pudo cargar FondoN2.png");
        }
        QPixmap fondoEscalado = fondoN2.scaled(1280, 640, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QGraphicsPixmapItem* bgItem = escena->addPixmap(fondoEscalado);
        bgItem->setPos(0, -100);
        bgItem->setZValue(-3);
    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    QPixmap texSuelo;
    bool tieneSuelo = false;
    try {
        if (!texSuelo.load(":/new/prefix1/SueloN2.png")) {
            throw std::runtime_error("No se pudo cargar :/new/prefix1/SueloN2.png");
        }
        texSuelo = limpiarFondoAjedrez(texSuelo);
        texSuelo = recortarBordesTransparentes(texSuelo);
        tieneSuelo = !texSuelo.isNull();
    } catch (const std::exception& e) {
        qWarning() << e.what();
    }

    if (tieneSuelo) {
        int anchoTile = 96;
        int altoTile = 75;
        QPixmap tileSuelo = texSuelo.scaled(anchoTile, altoTile, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (int x = 0; x < 1280; x += (anchoTile - 2)) {
            QGraphicsPixmapItem* sueloItem = escena->addPixmap(tileSuelo);
            sueloItem->setPos(x, 465);
            sueloItem->setZValue(0);
        }
    } else {
        escena->addRect(0, 465, 1280, 15, QPen(Qt::NoPen), QBrush(QColor(212, 172, 110)));
        escena->addRect(0, 480, 1280, 60, QPen(Qt::NoPen), QBrush(QColor(135, 100, 58)));
    }

    {
        const int cw = 355;
        const int ch = 104;
        QPixmap cartelPix(cw, ch);
        cartelPix.fill(Qt::transparent);

        QPainter p(&cartelPix);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.fillRect(2, 2, cw - 4, ch - 4, QColor(15, 20, 28, 205));
        p.setPen(QPen(QColor(210, 180, 110), 2));
        p.drawRect(2, 2, cw - 4, ch - 4);

        QFont fTitulo("Courier New", 9, QFont::Bold);
        fTitulo.setStyleStrategy(QFont::NoAntialias);
        p.setFont(fTitulo);
        p.setPen(QColor(255, 220, 95));
        p.drawText(12, 20, "DUELO FINAL • CONTROLES DE ARENA:");

        p.setPen(QColor(240, 240, 240));
        p.drawText(12, 40, "[A/D] Mover | [W/Espacio] Saltar | [S] Agacharse");
        p.drawText(12, 58, "[H] Golpe   | [J] Escudo Rebote  | [K] Bloquear");
        p.drawText(12, 78, "TIP: Bloquea disparos y aturde a Trump con [J]");
        p.end();

        QGraphicsPixmapItem* itemCartel = escena->addPixmap(cartelPix);
        itemCartel->setPos(462, -22);
        itemCartel->setZValue(8);
    }

    hudTexto = escena->addText("Tiempo: 03:00 | Nivel 2: Costa del Caribe | FASE 1");
    hudTexto->setDefaultTextColor(Qt::white);
    hudTexto->setFont(QFont("Arial", 15, QFont::Bold));
    hudTexto->setPos(25, -88);
    hudTexto->setZValue(10);

    itemBarraVidaHeroe = escena->addPixmap(QPixmap());
    itemBarraVidaHeroe->setPos(25, -48);
    itemBarraVidaHeroe->setZValue(11);
    actualizarBarraVida(itemBarraVidaHeroe, ultimaVidaHeroeRender,
                        controlador->getHeroe()->getVidaActual(),
                        controlador->getHeroe()->getVidaMax(), "CAPITÁN", false);

    itemBarraVidaJefe = escena->addPixmap(QPixmap());
    itemBarraVidaJefe->setPos(915, -48);
    itemBarraVidaJefe->setZValue(11);
    actualizarBarraVida(itemBarraVidaJefe, ultimaVidaJefeRender,
                        controlador->getJefe()->getVidaActual(),
                        controlador->getJefe()->getVidaMax(), "DONALD TRUMP", true);
}

void VentanaNivel2::cargarSprites() {
    const int ALTURA_CORRER   = 76;
    const int ALTURA_ESTANDAR = 88;
    const int ALTURA_TRUMP    = 112;

    QPixmap sheetCorrer;
    if (sheetCorrer.load(":/new/prefix1/Sprite_Correr.png")) {
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
                            bool esAzul = (c.blue() > c.red() + 35 && c.blue() > 90);
                            bool esRojo = (c.red() > c.blue() + 45 && c.red() > 110 && c.green() < 90);
                            if (esAzul || esRojo) pixelesHeroe++;
                        }
                    }
                }
                if (pixelesHeroe > 35) {
                    QPixmap recorte = recortarBordesTransparentes(celda);
                    framesCorrer.push_back(recorte.scaledToHeight(ALTURA_CORRER, Qt::SmoothTransformation));
                }
            }
        }
    }
    if (framesCorrer.empty()) {
        QPixmap fb(60, ALTURA_CORRER);
        fb.fill(Qt::blue);
        framesCorrer.push_back(fb);
    }
    pixQuieto = framesCorrer[0];

    QPixmap sheetSaltar;
    if (sheetSaltar.load(":/new/prefix1/Sprite_Saltar.png")) {
        QPixmap saltoLimpio = limpiarVerdeChroma(sheetSaltar);
        extraerSpritesPorBlob(saltoLimpio, 0, saltoLimpio.height(), 145, ALTURA_ESTANDAR, framesSaltar);
    }
    if (framesSaltar.empty()) framesSaltar.push_back(pixQuieto);

    QPixmap sheetLucha;
    if (sheetLucha.load(":/new/prefix1/Lucha_cuerpoACuerpo.png")) {
        int mitadY = sheetLucha.height() / 2;
        extraerSpritesPorBlob(sheetLucha, mitadY, sheetLucha.height(), 105, ALTURA_ESTANDAR, framesGolpear);
        extraerSpritesPorBlob(sheetLucha, 0, mitadY, 105, ALTURA_ESTANDAR, framesGolpear);
    }
    if (framesGolpear.empty()) framesGolpear.push_back(pixQuieto);

    QPixmap sheetDefensa;
    if (sheetDefensa.load(":/new/prefix1/DefensaC.png")) {
        extraerSpritesPorBlob(sheetDefensa, 0, sheetDefensa.height(), 0, ALTURA_ESTANDAR, framesDefensa);
    }
    if (framesDefensa.empty()) framesDefensa.push_back(pixQuieto);

    QPixmap sheetEscudo;
    if (sheetEscudo.load(":/new/prefix1/SpritesEscudoAmerica.jpg")) {
        QPixmap escudoSinFondo = limpiarFondoAjedrez(sheetEscudo);
        int w = escudoSinFondo.width() / 4;
        int h = escudoSinFondo.height() / 4;
        for (int fila = 0; fila < 4; ++fila) {
            for (int col = 0; col < 4; ++col) {
                framesEscudo.push_back(
                    escudoSinFondo.copy(col * w, fila * h, w, h).scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                    );
            }
        }
    }
    if (framesEscudo.empty()) {
        QPixmap fb(28, 28);
        fb.fill(Qt::red);
        framesEscudo.push_back(fb);
    }

    QPixmap sheetTrump;
    if (sheetTrump.load(":/new/prefix1/sprites_ Trump.png")) {
        QPixmap trumpLimpio = limpiarFondoOscuro(sheetTrump);
        extraerSpritesPorBlob(trumpLimpio, 60, 213, 0, ALTURA_TRUMP, framesTrumpIdle);
        extraerSpritesPorBlob(trumpLimpio, 304, 442, 0, ALTURA_TRUMP, framesTrumpDisparo);
        extraerSpritesPorBlob(trumpLimpio, 510, 710, 0, ALTURA_TRUMP, framesTrumpEmbestida);
        extraerSpritesPorBlob(trumpLimpio, 770, 984, 0, ALTURA_TRUMP, framesTrumpAturdido);
    }
    if (framesTrumpIdle.empty()) {
        QPixmap fb(80, ALTURA_TRUMP);
        fb.fill(Qt::darkRed);
        framesTrumpIdle.push_back(fb);
    }
    if (framesTrumpDisparo.empty()) framesTrumpDisparo = framesTrumpIdle;
    if (framesTrumpEmbestida.empty()) framesTrumpEmbestida = framesTrumpIdle;
    if (framesTrumpAturdido.empty()) framesTrumpAturdido = framesTrumpIdle;

    pixBalaJefe = QPixmap(36, 14);
    pixBalaJefe.fill(Qt::transparent);
    {
        QPainter p(&pixBalaJefe);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(231, 76, 60, 200));
        p.drawEllipse(0, 2, 20, 10);
        p.setPen(QPen(Qt::black, 1.5));
        p.setBrush(QColor(255, 215, 0));
        p.drawRoundedRect(12, 2, 22, 10, 5, 5);
    }

    pixBombaJefe = QPixmap(34, 38);
    pixBombaJefe.fill(Qt::transparent);
    {
        QPainter p(&pixBombaJefe);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::black, 2));
        p.setBrush(QColor(44, 62, 80));
        p.drawEllipse(3, 8, 28, 28);
        p.setBrush(QColor(231, 76, 60));
        p.drawRect(13, 3, 8, 6);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(241, 196, 15));
        p.drawEllipse(14, 0, 6, 6);
    }

    itemHeroe = escena->addPixmap(pixQuieto);
    itemHeroe->setZValue(4);

    itemEscudo = escena->addPixmap(framesEscudo[0]);
    itemEscudo->setVisible(false);
    itemEscudo->setZValue(5);

    itemJefe = escena->addPixmap(framesTrumpIdle[0]);
    itemJefe->setZValue(4);

    for (int i = 0; i < 25; ++i) {
        QGraphicsPixmapItem* bItem = escena->addPixmap(pixBalaJefe);
        bItem->setVisible(false);
        bItem->setZValue(6);
        poolDisparosJefe.push_back(bItem);
    }

    for (int i = 0; i < 15; ++i) {
        QGraphicsEllipseItem* sombra = escena->addEllipse(0, 0, 44, 12, QPen(Qt::NoPen), QBrush(QColor(15, 15, 15, 150)));
        sombra->setVisible(false);
        sombra->setZValue(2);
        poolSombrasBombas.push_back(sombra);

        QGraphicsPixmapItem* bombaItem = escena->addPixmap(pixBombaJefe);
        bombaItem->setVisible(false);
        bombaItem->setZValue(7);
        poolBombasJefe.push_back(bombaItem);
    }
}

void VentanaNivel2::actualizarAnimaciones(int direccion) {
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
        float progreso = std::clamp((vy + 560.0f) / 1120.0f, 0.0f, 0.999f);
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

    if (controlador->isAgachado() && heroe->isEnSuelo() && !frameHeroe.isNull()) {
        frameHeroe = frameHeroe.scaled(frameHeroe.width(), static_cast<int>(frameHeroe.height() * 0.56),
                                       Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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

    JefeTrump* jefe = controlador->getJefe();
    if (!jefe->isVivo()) {
        itemJefe->setVisible(false);
        return;
    }

    if (contadorTicks % 5 == 0) frameTrumpActual++;

    const std::vector<QPixmap>* setTrump = &framesTrumpIdle;
    if (jefe->isAturdido()) {
        setTrump = &framesTrumpAturdido;
    } else if (jefe->isEmbistiendo()) {
        setTrump = &framesTrumpEmbestida;
    } else if (jefe->isDisparando()) {
        setTrump = &framesTrumpDisparo;
    }

    if (!setTrump->empty()) {
        QPixmap pixT = (*setTrump)[frameTrumpActual % setTrump->size()];
        if (jefe->getDireccion() < 0) {
            pixT = pixT.transformed(QTransform().scale(-1, 1));
        }
        itemJefe->setPixmap(pixT);
    }
}

void VentanaNivel2::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat() || overlayFinNivel != nullptr) return;

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
    case Qt::Key_S:
    case Qt::Key_Down:
        teclaAbajo = true;
        controlador->setAgachado(true);
        break;
    case Qt::Key_W:
    case Qt::Key_Space:
        controlador->setAgachado(false);
        if (controlador->getHeroe()->isEnSuelo()) {
            sfxSaltar->setPosition(0);
            sfxSaltar->play();
        }
        controlador->getHeroe()->saltar();
        break;
    case Qt::Key_H:
        controlador->setAgachado(false);
        controlador->getHeroe()->iniciarGolpeCuerpoACuerpo();
        break;
    case Qt::Key_J:
        controlador->setAgachado(false);
        controlador->getHeroe()->lanzarEscudo(mirandoDerecha ? 15.0f : 165.0f);
        break;
    case Qt::Key_K:
        controlador->getHeroe()->bloquear(true);
        break;
    }
}

void VentanaNivel2::keyReleaseEvent(QKeyEvent* event) {
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
    case Qt::Key_S:
    case Qt::Key_Down:
        teclaAbajo = false;
        controlador->setAgachado(false);
        break;
    case Qt::Key_K:
        controlador->getHeroe()->bloquear(false);
        break;
    }
}

void VentanaNivel2::actualizarFrame() {
    float dt = 0.016f;

    int dir = 0;
    if (teclaIzquierda) dir -= 1;
    if (teclaDerecha) dir += 1;

    CapitanAmerica* heroe = controlador->getHeroe();
    JefeTrump* jefe = controlador->getJefe();

    controlador->setAgachado(teclaAbajo && dir == 0);
    heroe->moverHorizontal(controlador->isAgachado() ? 0 : dir, 0.0f);
    controlador->actualizarReglas(dt);

    int vidaActual = heroe->getVidaActual();
    if (vidaPreviaHeroe != -1 && vidaActual < vidaPreviaHeroe) {
        sfxRecibeDano->setPosition(0);
        sfxRecibeDano->play();
    }
    vidaPreviaHeroe = vidaActual;

    actualizarAnimaciones(dir);

    double anchoHeroePix = itemHeroe->pixmap().width();
    double altoHeroePix  = itemHeroe->pixmap().height();
    double xCentradaHeroe = heroe->getPosX() + (70.0 - anchoHeroePix) * 0.5;
    double yAlineadaHeroe = (heroe->getPosY() + 95.0) - altoHeroePix + 4.0;
    itemHeroe->setPos(xCentradaHeroe, yAlineadaHeroe);

    double hJefe = itemJefe->pixmap().height();
    itemJefe->setPos(jefe->getPosX(), (jefe->getPosY() + 112.0) - hJefe + 4.0);

    EscudoVibranium* escudo = heroe->getEscudo();
    itemEscudo->setVisible(escudo->estaActivo());
    if (escudo->estaActivo()) {
        itemEscudo->setPos(escudo->getPosX() - 16.0f, escudo->getPosY() - 16.0f);
    }

    const auto& proyectiles = controlador->getProyectilesJefe();
    size_t disparosActivos = 0;
    size_t idxDisparo = 0;
    size_t idxBomba   = 0;

    for (const auto& proj : proyectiles) {
        if (!proj.activa) continue;
        if (proj.esBomba) {
            if (idxBomba < poolBombasJefe.size()) {
                poolBombasJefe[idxBomba]->setVisible(true);
                poolBombasJefe[idxBomba]->setPos(proj.x - 17.0, proj.y - 19.0);

                float progresoCaida = std::clamp((proj.y + 70.0f) / 535.0f, 0.15f, 1.0f);
                double anchoSombra = 56.0 * progresoCaida;
                double altoSombra  = 14.0 * progresoCaida;
                poolSombrasBombas[idxBomba]->setVisible(true);
                poolSombrasBombas[idxBomba]->setRect(proj.x - anchoSombra * 0.5, 462.0 - altoSombra * 0.5, anchoSombra, altoSombra);
                idxBomba++;
            }
        } else {
            disparosActivos++;
            if (idxDisparo < poolDisparosJefe.size()) {
                poolDisparosJefe[idxDisparo]->setVisible(true);
                QPixmap pBala = pixBalaJefe;
                if (proj.vx < 0) pBala = pBala.transformed(QTransform().scale(-1, 1));
                poolDisparosJefe[idxDisparo]->setPixmap(pBala);
                poolDisparosJefe[idxDisparo]->setPos(proj.x, proj.y);
                idxDisparo++;
            }
        }
    }

    if (disparosActivos > conteoDisparosPrevio) {
        sfxDisparo->setPosition(0);
        sfxDisparo->play();
    }
    conteoDisparosPrevio = disparosActivos;

    for (size_t i = idxDisparo; i < poolDisparosJefe.size(); ++i) {
        poolDisparosJefe[i]->setVisible(false);
    }
    for (size_t i = idxBomba; i < poolBombasJefe.size(); ++i) {
        poolBombasJefe[i]->setVisible(false);
        poolSombrasBombas[i]->setVisible(false);
    }

    refrescarTextoHUD();
    actualizarBarraVida(itemBarraVidaHeroe, ultimaVidaHeroeRender,
                        heroe->getVidaActual(), heroe->getVidaMax(), "CAPITÁN", false);
    actualizarBarraVida(itemBarraVidaJefe, ultimaVidaJefeRender,
                        jefe->getVidaActual(), jefe->getVidaMax(), "DONALD TRUMP", true);
}

void VentanaNivel2::refrescarTextoHUD() {
    int segs = controlador->getTiempoRestante();
    int min = segs / 60;
    int seg = segs % 60;
    QString texto = QString("Tiempo: %1:%2 | COSTA DEL CARIBE • %3")
                        .arg(min, 2, 10, QChar('0'))
                        .arg(seg, 2, 10, QChar('0'))
                        .arg(controlador->getJefe()->getNombreEstadoIA());
    hudTexto->setPlainText(texto);
}

void VentanaNivel2::mostrarPantallaFinNivel(bool victoria) {
    if (overlayFinNivel) return;

    overlayFinNivel = new QWidget(viewport());
    overlayFinNivel->setGeometry(viewport()->rect());
    overlayFinNivel->setStyleSheet("background-color: rgba(8, 10, 15, 200);");

    QVBoxLayout* layoutFondo = new QVBoxLayout(overlayFinNivel);
    layoutFondo->setAlignment(Qt::AlignCenter);

    QWidget* tarjeta = new QWidget(overlayFinNivel);
    tarjeta->setObjectName("tarjetaFinN2");
    tarjeta->setFixedWidth(700);

    QString colorBorde = victoria ? "#f1c40f" : "#e74c3c";
    tarjeta->setStyleSheet(
        QString("QWidget#tarjetaFinN2 {"
                "   background-color: rgba(18, 22, 32, 245);"
                "   border: 4px solid %1;"
                "   border-radius: 8px;"
                "}").arg(colorBorde)
        );

    QVBoxLayout* layoutTarjeta = new QVBoxLayout(tarjeta);
    layoutTarjeta->setContentsMargins(38, 30, 38, 30);
    layoutTarjeta->setSpacing(14);
    layoutTarjeta->setAlignment(Qt::AlignCenter);

    QLabel* lblBadge = new QLabel(victoria ? "★ CAMPAÑA COMPLETADA ★" : "✖ DERROTA EN EL CARIBE ✖", tarjeta);
    lblBadge->setAlignment(Qt::AlignCenter);
    lblBadge->setStyleSheet(
        QString("color: %1;"
                "font-family: 'Courier New';"
                "font-size: 16px;"
                "font-weight: bold;"
                "letter-spacing: 2px;"
                "background: transparent;").arg(victoria ? "#2ecc71" : "#ff7675")
        );

    QLabel* lblTitulo = new QLabel(victoria ? "¡VICTORIA TOTAL!\n¡SOBERANÍA DEFENDIDA!" : "MISIÓN FALLIDA", tarjeta);
    lblTitulo->setAlignment(Qt::AlignCenter);
    lblTitulo->setStyleSheet(
        QString("color: %1;"
                "font-family: 'Courier New';"
                "font-size: 34px;"
                "font-weight: bold;"
                "background: transparent;").arg(victoria ? "#f1c40f" : "#e74c3c")
        );

    QLabel* lblResumen = new QLabel(
        victoria
            ? "Has derrotado a Donald Trump en la costa del Caribe y neutralizado la amenaza extranjera sobre Panamá."
            : "Donald Trump ha superado tus defensas o se agotó el tiempo del combate.",
        tarjeta
        );
    lblResumen->setAlignment(Qt::AlignCenter);
    lblResumen->setWordWrap(true);
    lblResumen->setStyleSheet(
        "color: #ecf0f1;"
        "font-family: 'Courier New';"
        "font-size: 16px;"
        "font-weight: bold;"
        "background: transparent;"
        );

    QPushButton* btnMenu = new QPushButton("VOLVER AL MENÚ", tarjeta);
    btnMenu->setCursor(Qt::PointingHandCursor);
    btnMenu->setFixedSize(270, 56);
    btnMenu->setStyleSheet(
        "QPushButton {"
        "   background-color: #c0392b;"
        "   color: #ffffff;"
        "   font-family: 'Courier New';"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   border: 3px solid #f5d76e;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e74c3c;"
        "   border: 3px solid #ffffff;"
        "}"
        );

    layoutTarjeta->addWidget(lblBadge);
    layoutTarjeta->addWidget(lblTitulo);
    layoutTarjeta->addWidget(lblResumen);
    layoutTarjeta->addSpacing(10);
    layoutTarjeta->addWidget(btnMenu, 0, Qt::AlignCenter);
    layoutFondo->addWidget(tarjeta);

    connect(btnMenu, &QPushButton::clicked, this, [this]() {
        sfxGameOver->stop();
        emit regresarAlMenu();
    });

    overlayFinNivel->show();
    overlayFinNivel->raise();
}

void VentanaNivel2::manejarFinNivel(bool victoria) {
    gameLoopTimer->stop();
    musicaFondo->stop();
    teclaIzquierda = false;
    teclaDerecha = false;
    teclaAbajo = false;

    if (!victoria) {
        sfxGameOver->play();
    }
    mostrarPantallaFinNivel(victoria);
}