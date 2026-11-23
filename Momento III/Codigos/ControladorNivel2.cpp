#include "ControladorNivel2.h"
#include <algorithm>
#include <cmath>

ControladorNivel2::ControladorNivel2(QObject* parent)
    : QObject(parent),
    tiempoRestanteSegundos(180),
    juegoTerminado(false),
    misionCumplida(false),
    agachado(false),
    impulsoHeroeX(0.0f),
    timerBombardeo(1.4f),
    contadorBombas(0) {

    heroe = new CapitanAmerica(180.0f, 370.0f);
    jefe  = new JefeTrump(980.0f, 353.0f);

    timerReloj = new QTimer(this);
    connect(timerReloj, &QTimer::timeout, this, &ControladorNivel2::descontarSegundo);
}

ControladorNivel2::~ControladorNivel2() {
    delete heroe;
    delete jefe;
}

void ControladorNivel2::iniciarReloj() {
    timerReloj->start(1000);
}

void ControladorNivel2::setAgachado(bool estado) {
    if (heroe->isEnSuelo()) {
        agachado = estado;
    } else {
        agachado = false;
    }
}

QRectF ControladorNivel2::getHitBoxHeroeArena() const {
    QRectF base = heroe->getHitBox();
    if (agachado && heroe->isEnSuelo()) {
        return QRectF(base.x(), base.y() + base.height() * 0.48f, base.width(), base.height() * 0.52f);
    }
    return base;
}

void ControladorNivel2::descontarSegundo() {
    if (juegoTerminado) return;

    tiempoRestanteSegundos--;
    emit tiempoActualizado(tiempoRestanteSegundos);

    if (tiempoRestanteSegundos == 25) {
        heroe->activarModoVeloz();
    }

    if (tiempoRestanteSegundos <= 0) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = false;
        emit nivelFinalizado(false);
    }
}

void ControladorNivel2::verificarColisionesArena() {
    QRectF boxHeroe  = getHitBoxHeroeArena();
    QRectF boxJefe   = jefe->getHitBox();
    QRectF boxEscudo = heroe->getEscudo()->getHitBox();
    QRectF boxGolpe  = heroe->getHitBoxGolpe();

    EscudoVibranium* escudo = heroe->getEscudo();
    if (escudo->estaActivo()) {
        if (escudo->getPosX() <= 24.0f) {
            escudo->rebotarVectorial(-1.0f, 0.0f);
        } else if (escudo->getPosX() >= 1256.0f) {
            escudo->rebotarVectorial(-1.0f, 0.0f);
        }

        if (boxEscudo.intersects(boxHeroe) && std::abs(escudo->getPosX() - (heroe->getPosX() + 35.0f)) < 45.0f) {
            if ((escudo->getPosX() < heroe->getPosX() + 35.0f && heroe->getVelX() >= 0) ||
                (escudo->getPosX() > heroe->getPosX() + 35.0f && heroe->getVelX() <= 0)) {
                escudo->desactivar();
            }
        }
    }

    if (escudo->estaActivo() && jefe->isVivo() && boxEscudo.intersects(boxJefe)) {
        float dirImpulso = (jefe->getPosX() >= escudo->getPosX()) ? 1.0f : -1.0f;
        jefe->recibirDanio(22, dirImpulso * 340.0f, true);
        escudo->rebotarVectorial(-1.0f, -0.45f);
    }

    if (heroe->isAtacandoCuerpoACuerpo() && !heroe->isGolpeYaImpacto() && jefe->isVivo()) {
        if (boxGolpe.intersects(boxJefe)) {
            float dirImpulso = (jefe->getPosX() >= heroe->getPosX()) ? 1.0f : -1.0f;
            jefe->recibirDanio(28, dirImpulso * 420.0f, jefe->isAturdido());
            heroe->marcarGolpeImpactado();
        }
    }

    if (jefe->isVivo() && boxJefe.intersects(boxHeroe)) {
        if (jefe->isEmbistiendo()) {
            float dirEmpuje = (heroe->getPosX() >= jefe->getPosX()) ? 1.0f : -1.0f;
            heroe->recibirDanio(20);
            impulsoHeroeX = dirEmpuje * 560.0f;
            heroe->setVelY(-240.0f);
            heroe->setEnSuelo(false);
        } else if (!heroe->isAtacandoCuerpoACuerpo()) {
            float dirEmpuje = (heroe->getPosX() >= jefe->getPosX()) ? 1.0f : -1.0f;
            heroe->recibirDanio(10);
            impulsoHeroeX = dirEmpuje * 280.0f;
        }
    }

    for (auto& proj : proyectilesJefe) {
        if (!proj.activa) continue;

        QRectF boxProj = proj.esBomba
                             ? QRectF(proj.x - 16.0f, proj.y - 16.0f, 32.0f, 32.0f)
                             : QRectF(proj.x, proj.y, 32.0f, 12.0f);

        if (boxProj.intersects(boxHeroe)) {
            proj.activa = false;
            int danioBase = proj.esBomba ? 24 : 16;
            heroe->recibirDanio(danioBase);
            float dirEmpuje = (proj.vx >= 0.0f) ? 1.0f : -1.0f;
            impulsoHeroeX = dirEmpuje * (heroe->isBloqueando() ? 140.0f : 320.0f);
        }
    }
}

void ControladorNivel2::actualizarReglas(float dt) {
    if (juegoTerminado) return;

    if (std::abs(impulsoHeroeX) > 1.0f) {
        float nuevaX = heroe->getPosX() + impulsoHeroeX * dt;
        nuevaX = std::clamp(nuevaX, 10.0f, 1200.0f);
        heroe->setPosX(nuevaX);
        impulsoHeroeX *= std::pow(0.05f, dt);
    } else {
        impulsoHeroeX = 0.0f;
    }

    heroe->actualizarEstado(dt);
    heroe->actualizarFisica(dt);
    if (heroe->getPosX() > 1200.0f) heroe->setPosX(1200.0f);
    if (heroe->getPosX() < 10.0f) heroe->setPosX(10.0f);

    heroe->getEscudo()->actualizarMovimiento(dt);

    jefe->actualizarIA(dt,
                       heroe->getPosX(),
                       !heroe->isEnSuelo(),
                       agachado,
                       heroe->isBloqueando(),
                       heroe->getEscudo()->estaActivo(),
                       proyectilesJefe);

    if (jefe->getFaseActual() == 2 && jefe->isVivo()) {
        timerBombardeo -= dt;
        if (timerBombardeo <= 0.0f) {
            timerBombardeo = 1.15f;
            contadorBombas++;

            float offsetX = ((contadorBombas % 3) - 1) * 175.0f;
            float dropX = std::clamp(heroe->getPosX() + offsetX, 80.0f, 1200.0f);
            float vxBomba = ((contadorBombas % 2 == 0) ? 1.0f : -1.0f) * 55.0f;
            proyectilesJefe.push_back({dropX, -70.0f, vxBomba, 40.0f, true, true});
        }
    }

    const float gravedadBombas = 520.0f;
    for (auto& proj : proyectilesJefe) {
        if (!proj.activa) continue;
        if (proj.esBomba) {
            proj.vy += gravedadBombas * dt;
            proj.x  += proj.vx * dt;
            proj.y  += proj.vy * dt;
            if (proj.y >= 455.0f) {
                proj.activa = false;
            }
        } else {
            proj.x += proj.vx * dt;
            if (proj.x < -40.0f || proj.x > 1320.0f) {
                proj.activa = false;
            }
        }
    }

    proyectilesJefe.erase(
        std::remove_if(proyectilesJefe.begin(), proyectilesJefe.end(),
                       [](const ProyectilJefe& p) { return !p.activa; }),
        proyectilesJefe.end()
        );

    verificarColisionesArena();

    if (heroe->getVidaActual() <= 0) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = false;
        emit nivelFinalizado(false);
        return;
    }

    if (!jefe->isVivo()) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = true;
        emit nivelFinalizado(true);
    }
}