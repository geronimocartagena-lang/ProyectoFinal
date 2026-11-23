#include "ControladorNivel1.h"
#include <algorithm>

ControladorNivel1::ControladorNivel1(QObject* parent)
    : QObject(parent),
    tiempoRestanteSegundos(240),
    nivelActual(1),
    civilesEvacuados(0),
    juegoTerminado(false),
    misionCumplida(false) {

    heroe = new CapitanAmerica(100.0f, 370.0f);

    // 13 Plataformas elevadas (Suelo.png) y 6 zonas de fango a lo largo de 8800 px
    mapaTerreno.push_back({QRectF(420.0f, 340.0f, 250.0f, 36.0f), false, false});   // Plat 1 (Y = 340)
    mapaTerreno.push_back({QRectF(860.0f, 455.0f, 270.0f, 18.0f), true, false});    // Fango 1
    mapaTerreno.push_back({QRectF(1260.0f, 320.0f, 280.0f, 36.0f), false, false});  // Plat 2 (Y = 320)
    mapaTerreno.push_back({QRectF(1680.0f, 240.0f, 260.0f, 36.0f), false, false});  // Plat 3 alta (Y = 240)
    mapaTerreno.push_back({QRectF(2050.0f, 455.0f, 300.0f, 18.0f), true, false});   // Fango 2
    mapaTerreno.push_back({QRectF(2480.0f, 330.0f, 280.0f, 36.0f), false, false});  // Plat 4 (Y = 330)
    mapaTerreno.push_back({QRectF(2950.0f, 250.0f, 260.0f, 36.0f), false, false});  // Plat 5 alta (Y = 250)
    mapaTerreno.push_back({QRectF(3350.0f, 455.0f, 300.0f, 18.0f), true, false});   // Fango 3
    mapaTerreno.push_back({QRectF(3820.0f, 320.0f, 280.0f, 36.0f), false, false});  // Plat 6 (Y = 320)
    mapaTerreno.push_back({QRectF(4280.0f, 240.0f, 270.0f, 36.0f), false, false});  // Plat 7 alta (Y = 240)
    mapaTerreno.push_back({QRectF(4720.0f, 455.0f, 300.0f, 18.0f), true, false});   // Fango 4
    mapaTerreno.push_back({QRectF(5180.0f, 330.0f, 280.0f, 36.0f), false, false});  // Plat 8 (Y = 330)
    mapaTerreno.push_back({QRectF(5620.0f, 250.0f, 270.0f, 36.0f), false, false});  // Plat 9 alta (Y = 250)
    mapaTerreno.push_back({QRectF(6080.0f, 455.0f, 310.0f, 18.0f), true, false});   // Fango 5
    mapaTerreno.push_back({QRectF(6540.0f, 325.0f, 290.0f, 36.0f), false, false});  // Plat 10 (Y = 325)
    mapaTerreno.push_back({QRectF(6980.0f, 240.0f, 280.0f, 36.0f), false, false});  // Plat 11 alta (Y = 240)
    mapaTerreno.push_back({QRectF(7420.0f, 455.0f, 300.0f, 18.0f), true, false});   // Fango 6
    mapaTerreno.push_back({QRectF(7860.0f, 320.0f, 290.0f, 36.0f), false, false});  // Plat 12 (Y = 320)
    mapaTerreno.push_back({QRectF(8050.0f, 230.0f, 240.0f, 36.0f), false, false});  // Plat 13 alta final (Y = 230)

    // 9 Obstáculos sólidos (Obstaculo.png, 96 x 78 px apoyados en Y = 465)
    mapaTerreno.push_back({QRectF(690.0f, 387.0f, 96.0f, 78.0f), false, true});     // Obstáculo 1
    mapaTerreno.push_back({QRectF(1560.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 2
    mapaTerreno.push_back({QRectF(2370.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 3
    mapaTerreno.push_back({QRectF(3220.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 4
    mapaTerreno.push_back({QRectF(4140.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 5
    mapaTerreno.push_back({QRectF(5060.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 6
    mapaTerreno.push_back({QRectF(5950.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 7
    mapaTerreno.push_back({QRectF(6850.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 8
    mapaTerreno.push_back({QRectF(7740.0f, 387.0f, 96.0f, 78.0f), false, true});    // Obstáculo 9

    // 5 Campesinos distribuidos a lo largo del mapa extendido
    listaCiviles.push_back(new Civil(510.0f, 340.0f - 95.0f));
    listaCiviles.push_back(new Civil(2180.0f, 465.0f - 95.0f));
    listaCiviles.push_back(new Civil(4380.0f, 240.0f - 95.0f));
    listaCiviles.push_back(new Civil(6640.0f, 325.0f - 95.0f));
    listaCiviles.push_back(new Civil(8120.0f, 230.0f - 95.0f));

    // 15 Soldados enemigos
    listaEnemigos.push_back(new Enemigo(805.0f, 370.0f, 65.0f));
    listaEnemigos.push_back(new Enemigo(1380.0f, 320.0f - 95.0f, 80.0f));
    listaEnemigos.push_back(new Enemigo(1980.0f, 370.0f, 120.0f));
    listaEnemigos.push_back(new Enemigo(2600.0f, 330.0f - 95.0f, 80.0f));
    listaEnemigos.push_back(new Enemigo(3060.0f, 250.0f - 95.0f, 75.0f));
    listaEnemigos.push_back(new Enemigo(3680.0f, 370.0f, 130.0f));
    listaEnemigos.push_back(new Enemigo(3940.0f, 320.0f - 95.0f, 80.0f));
    listaEnemigos.push_back(new Enemigo(4620.0f, 370.0f, 120.0f));
    listaEnemigos.push_back(new Enemigo(5300.0f, 330.0f - 95.0f, 80.0f));
    listaEnemigos.push_back(new Enemigo(5750.0f, 250.0f - 95.0f, 75.0f));
    listaEnemigos.push_back(new Enemigo(6410.0f, 370.0f, 110.0f));
    listaEnemigos.push_back(new Enemigo(7090.0f, 240.0f - 95.0f, 85.0f));
    listaEnemigos.push_back(new Enemigo(7320.0f, 370.0f, 120.0f));
    listaEnemigos.push_back(new Enemigo(7980.0f, 320.0f - 95.0f, 85.0f));
    listaEnemigos.push_back(new Enemigo(8220.0f, 370.0f, 110.0f));

    zonaEvacuacion = QRectF(8420.0f, 310.0f, 260.0f, 155.0f);

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
        if (obs.esFango) continue;

        if (boxHeroe.intersects(obs.area)) {
            if (heroe->getVelY() >= 0.0f && boxHeroe.bottom() <= obs.area.top() + 22.0f) {
                heroe->setPosY(obs.area.top() - 95.0f);
                heroe->setVelY(0.0f);
                heroe->setEnSuelo(true);
            } else if (obs.esBloqueSalto) {
                if (boxHeroe.center().x() < obs.area.center().x()) {
                    heroe->setPosX(obs.area.left() - 58.0f);
                } else {
                    heroe->setPosX(obs.area.right() - 12.0f);
                }
                heroe->setVelX(0.0f);
            }
        }

        if (heroe->getEscudo()->estaActivo() && boxEscudo.intersects(obs.area)) {
            if (obs.esBloqueSalto) {
                heroe->getEscudo()->rebotarVectorial(-1.0f, -0.3f);
            } else {
                heroe->getEscudo()->rebotarVectorial(0.0f, -1.0f);
            }
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
        QRectF boxBala(bala.x, bala.y, 32.0f, 12.0f);
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
        if (bala.x < 0.0f || bala.x > 8800.0f) {
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