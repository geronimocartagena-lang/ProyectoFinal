#include "CapitanAmerica.h"

CapitanAmerica::CapitanAmerica(float x, float y)
    : Personaje(x, y, 70.0f, 95.0f, 270.0f),
    civilesRescatados(0),
    modoVeloz(false),
    bloqueando(false),
    atacandoCuerpoACuerpo(false),
    mirandoDerecha(true),
    timerGolpe(0.0f),
    golpeYaImpacto(false),
    vidaMax(100),
    vidaActual(100),
    invulnerableTimer(0.0f) {
    escudo = new EscudoVibranium();
}

CapitanAmerica::~CapitanAmerica() {
    delete escudo;
}

void CapitanAmerica::moverHorizontal(int direccion, float friccionTerreno) {
    if (direccion > 0) mirandoDerecha = true;
    else if (direccion < 0) mirandoDerecha = false;

    float multiplicador = modoVeloz ? 1.45f : 1.0f;
    if (bloqueando || atacandoCuerpoACuerpo) {
        multiplicador *= 0.45f;
    }

    float factorFriccion = 1.0f - friccionTerreno;
    if (factorFriccion < 0.25f) factorFriccion = 0.25f;

    velX = direccion * velocidadBase * multiplicador * factorFriccion;
}

void CapitanAmerica::saltar() {
    if (enSuelo) {
        velY = -560.0f;
        enSuelo = false;
    }
}

void CapitanAmerica::lanzarEscudo(float anguloGrados) {
    if (!escudo->estaActivo() && !bloqueando && !atacandoCuerpoACuerpo) {
        float origenX = posX + (ancho * 0.5f);
        float origenY = posY + (alto * 0.35f);
        escudo->lanzar(origenX, origenY, anguloGrados, 680.0f);
    }
}

void CapitanAmerica::iniciarGolpeCuerpoACuerpo() {
    if (!atacandoCuerpoACuerpo && !bloqueando) {
        atacandoCuerpoACuerpo = true;
        timerGolpe = 0.36f;
        golpeYaImpacto = false;
    }
}

void CapitanAmerica::rescatarCivil(Civil* civil) {
    if (civil && !civil->isRescatado()) {
        civil->rescatar(civilesRescatados);
        civilesRescatados++;
    }
}

void CapitanAmerica::activarModoVeloz() {
    modoVeloz = true;
}

void CapitanAmerica::actualizarEstado(float dt) {
    if (invulnerableTimer > 0.0f) {
        invulnerableTimer -= dt;
    }

    if (atacandoCuerpoACuerpo) {
        timerGolpe -= dt;
        if (timerGolpe <= 0.0f) {
            atacandoCuerpoACuerpo = false;
            timerGolpe = 0.0f;
        }
    }
}

void CapitanAmerica::recibirDanio(int danio) {
    if (invulnerableTimer > 0.0f) return;
    if (bloqueando) {
        danio = danio / 4;
    }
    vidaActual -= danio;
    if (vidaActual < 0) vidaActual = 0;
    invulnerableTimer = 0.65f;
}

QRectF CapitanAmerica::getHitBoxGolpe() const {
    if (!atacandoCuerpoACuerpo) return QRectF();
    float alcance = 55.0f;
    if (mirandoDerecha) {
        return QRectF(posX + ancho - 10.0f, posY + 15.0f, alcance, alto - 30.0f);
    } else {
        return QRectF(posX - alcance + 10.0f, posY + 15.0f, alcance, alto - 30.0f);
    }
}

QString CapitanAmerica::getEstadoEspecial() const {
    if (atacandoCuerpoACuerpo) return "Golpe Cuerpo a Cuerpo";
    if (bloqueando) return "Escudo en Guardia";
    if (modoVeloz) return "MODO VELOZ (+45%)";
    return "Normal";
}