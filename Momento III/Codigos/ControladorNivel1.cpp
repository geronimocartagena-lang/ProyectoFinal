#include "ControladorNivel1.h"
#include <algorithm>

ControladorNivel1::ControladorNivel1(QObject* parent)
    : QObject(parent),
    tiempoRestanteSegundos(180),
    nivelActual(1),
    civilesEvacuados(0),
    juegoTerminado(false),
    misionCumplida(false) {

    heroe = new CapitanAmerica(100.0f, 360.0f);

    listaCiviles.push_back(new Civil(650.0f, 370.0f));
    listaCiviles.push_back(new Civil(1450.0f, 370.0f));
    listaCiviles.push_back(new Civil(2250.0f, 370.0f));
    listaCiviles.push_back(new Civil(3050.0f, 370.0f));
    listaCiviles.push_back(new Civil(3800.0f, 370.0f));

    listaEnemigos.push_back(new Enemigo(820.0f, 360.0f, 120.0f));
    listaEnemigos.push_back(new Enemigo(1550.0f, 360.0f, 140.0f));
    listaEnemigos.push_back(new Enemigo(2150.0f, 360.0f, 130.0f));
    listaEnemigos.push_back(new Enemigo(2750.0f, 360.0f, 150.0f));
    listaEnemigos.push_back(new Enemigo(3350.0f, 360.0f, 140.0f));
    listaEnemigos.push_back(new Enemigo(3980.0f, 360.0f, 120.0f));

    mapaTerreno.push_back({QRectF(420.0f, 330.0f, 220.0f, 40.0f), false});
    mapaTerreno.push_back({QRectF(880.0f, 455.0f, 280.0f, 18.0f), true});
    mapaTerreno.push_back({QRectF(1320.0f, 310.0f, 240.0f, 40.0f), false});
    mapaTerreno.push_back({QRectF(1800.0f, 455.0f, 320.0f, 18.0f), true});
    mapaTerreno.push_back({QRectF(2450.0f, 320.0f, 240.0f, 40.0f), false});
    mapaTerreno.push_back({QRectF(2850.0f, 455.0f, 300.0f, 18.0f), true});
    mapaTerreno.push_back({QRectF(3350.0f, 300.0f, 260.0f, 40.0f), false});

    zonaEvacuacion = QRectF(4220.0f, 310.0f, 200.0f, 155.0f);

    timerReloj = new QTimer(this);
    connect(timerReloj, &QTimer::timeout, this, &ControladorNivel1::descontarSegundo);
}

ControladorNivel1::~ControladorNivel1() {
    delete heroe;
    for (Civil* c : listaCiviles) delete c;
    for (Enemigo* e : listaEnemigos) delete e;
    listaCiviles.clear();
    listaEnemigos.clear();
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
    QRectF boxGolpe = heroe->getHitBoxGolpe();

    for (const auto& obs : mapaTerreno) {
        if (!obs.esFango && boxHeroe.intersects(obs.area)) {
            if (heroe->getVelY() >= 0.0f && boxHeroe.bottom() <= obs.area.top() + 18.0f) {
                heroe->setPosY(obs.area.top() - boxHeroe.height());
                heroe->setVelY(0.0f);
                heroe->setEnSuelo(true);
            }
        }
        if (!obs.esFango && heroe->getEscudo()->estaActivo() && boxEscudo.intersects(obs.area)) {
            heroe->getEscudo()->rebotarVectorial(0.0f, -1.0f);
        }
    }

    if (heroe->isAtacandoCuerpoACuerpo() && !heroe->isGolpeYaImpacto()) {
        for (Enemigo* enemigo : listaEnemigos) {
            if (enemigo->isVivo() && boxGolpe.intersects(enemigo->getHitBox())) {
                enemigo->recibirDanio(2);
                heroe->marcarGolpeImpactado();
                break;
            }
        }
    }

    if (heroe->getEscudo()->estaActivo()) {
        for (Enemigo* enemigo : listaEnemigos) {
            if (enemigo->isVivo() && boxEscudo.intersects(enemigo->getHitBox())) {
                enemigo->recibirDanio(1);
                heroe->getEscudo()->rebotarVectorial(-1.0f, -0.5f);
            }
        }
    }

    for (auto& bala : balasEnemigas) {
        if (!bala.activa) continue;
        QRectF boxBala(bala.x, bala.y, 18.0f, 8.0f);
        if (boxBala.intersects(boxHeroe)) {
            bala.activa = false;
            heroe->recibirDanio(15);
        }
    }

    for (Enemigo* enemigo : listaEnemigos) {
        if (enemigo->isVivo() && !heroe->isAtacandoCuerpoACuerpo() && boxHeroe.intersects(enemigo->getHitBox())) {
            heroe->recibirDanio(10);
        }
    }

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

    for (Enemigo* enemigo : listaEnemigos) {
        enemigo->actualizar(dt, heroe->getPosX(), balasEnemigas);
    }

    for (auto& bala : balasEnemigas) {
        if (!bala.activa) continue;
        bala.x += bala.vx * dt;
        if (bala.x < 0.0f || bala.x > 4500.0f) {
            bala.activa = false;
        }
    }
    balasEnemigas.erase(
        std::remove_if(balasEnemigas.begin(), balasEnemigas.end(),
                       [](const BalaEnemiga& b) { return !b.activa; }),
        balasEnemigas.end()
        );

    for (Civil* civil : listaCiviles) {
        civil->seguirHeroe(heroe->getPosX(), dt);
    }

    verificarColisionesAABB();

    if (heroe->getVidaActual() <= 0) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = false;
        emit nivelFinalizado(false);
        return;
    }

    if (civilesEvacuados >= static_cast<int>(listaCiviles.size())) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = true;
        emit nivelFinalizado(true);
    }
}