#include "ControladorNivel1.h"

ControladorNivel1::ControladorNivel1(QObject* parent)
    : QObject(parent),
    tiempoRestanteSegundos(180),
    nivelActual(1),
    civilesEvacuados(0),
    juegoTerminado(false),
    misionCumplida(false) {

    // Posición inicial sobre el suelo (465 - 105 = 360)
    heroe = new CapitanAmerica(100.0f, 360.0f);

    // 5 campesinos distribuidos en el pueblo (465 - 95 = 370)
    listaCiviles.push_back(new Civil(500.0f, 370.0f));
    listaCiviles.push_back(new Civil(950.0f, 370.0f));
    listaCiviles.push_back(new Civil(1400.0f, 370.0f));
    listaCiviles.push_back(new Civil(1850.0f, 370.0f));
    listaCiviles.push_back(new Civil(2250.0f, 370.0f));

    // Plataformas flotantes (donde se dibujará Suelo.png) y zonas de fango
    mapaTerreno.push_back({QRectF(320.0f, 330.0f, 200.0f, 32.0f), false});
    mapaTerreno.push_back({QRectF(680.0f, 455.0f, 240.0f, 18.0f), true});
    mapaTerreno.push_back({QRectF(1120.0f, 310.0f, 220.0f, 32.0f), false});
    mapaTerreno.push_back({QRectF(1550.0f, 455.0f, 280.0f, 18.0f), true});

    // Zona segura más amplia para facilitar la entrega de campesinos
    zonaEvacuacion = QRectF(2380.0f, 310.0f, 180.0f, 155.0f);

    timerReloj = new QTimer(this);
    connect(timerReloj, &QTimer::timeout, this, &ControladorNivel1::descontarSegundo);
}

ControladorNivel1::~ControladorNivel1() {
    delete heroe;
    for (Civil* c : listaCiviles) {
        delete c;
    }
    listaCiviles.clear();
}

void ControladorNivel1::iniciarReloj() {
    timerReloj->start(1000);
}

void ControladorNivel1::descontarSegundo() {
    if (juegoTerminado) return;

    tiempoRestanteSegundos--;
    emit tiempoActualizado(tiempoRestanteSegundos);

    if (tiempoRestanteSegundos == 30) {
        heroe->activarModoVeloz();
    }

    if (tiempoRestanteSegundos <= 0) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = false;
        emit nivelFinalizado(false);
    }
}

float ControladorNivel1::obtenerFriccionActualHeroe() const {
    QRectF boxHeroe = heroe->getHitBox();
    for (const auto& obs : mapaTerreno) {
        if (obs.esFango && boxHeroe.intersects(obs.area)) {
            return 0.55f;
        }
    }
    return 0.0f;
}

void ControladorNivel1::verificarColisionesAABB() {
    QRectF boxHeroe = heroe->getHitBox();
    QRectF boxEscudo = heroe->getEscudo()->getHitBox();

    // A. Colisión del héroe sobre plataformas sólidas
    for (const auto& obs : mapaTerreno) {
        if (!obs.esFango && boxHeroe.intersects(obs.area)) {
            if (heroe->getVelY() >= 0.0f && boxHeroe.bottom() <= obs.area.top() + 18.0f) {
                heroe->setPosY(obs.area.top() - boxHeroe.height());
                heroe->setVelY(0.0f);
                heroe->setEnSuelo(true);
            }
        }
        // B. Rebote del escudo en plataformas
        if (!obs.esFango && heroe->getEscudo()->estaActivo() && boxEscudo.intersects(obs.area)) {
            heroe->getEscudo()->rebotarVectorial(0.0f, -1.0f);
        }
    }

    // C. Rescate y evacuación de civiles
    for (Civil* civil : listaCiviles) {
        if (!civil->isRescatado() && boxHeroe.intersects(civil->getHitBox())) {
            heroe->rescatarCivil(civil);
        }

        if (civil->isRescatado() && !civil->isEvacuado() && civil->getHitBox().intersects(zonaEvacuacion)) {
            civil->evacuar();
            civilesEvacuados++;
        }
    }
}

void ControladorNivel1::actualizarReglas(float dt) {
    if (juegoTerminado) return;

    heroe->actualizarEstado(dt);
    heroe->actualizarFisica(dt);
    heroe->getEscudo()->actualizarMovimiento(dt);

    for (Civil* civil : listaCiviles) {
        civil->seguirHeroe(heroe->getPosX(), dt);
    }

    verificarColisionesAABB();

    if (civilesEvacuados >= static_cast<int>(listaCiviles.size())) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = true;
        emit nivelFinalizado(true);
    }
}