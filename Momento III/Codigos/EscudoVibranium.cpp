#include "EscudoVibranium.h"
#include <cmath>

EscudoVibranium::EscudoVibranium()
    : posX(0.0f), posY(0.0f), velX(0.0f), velY(0.0f),
    radio(18.0f), activo(false), tiempoVuelo(0.0f),
    rebotes(0), modoRebote(false) {}

void EscudoVibranium::lanzar(float x, float y, float anguloGrados, float rapidez) {
    posX = x;
    posY = y;
    float rad = anguloGrados * 3.14159265f / 180.0f;
    velX = std::cos(rad) * rapidez;
    velY = modoRebote ? (-std::sin(rad) * rapidez * 0.25f) : (-std::sin(rad) * rapidez * 0.45f);
    activo = true;
    tiempoVuelo = 0.0f;
    rebotes = 0;
}

void EscudoVibranium::actualizarMovimiento(float dt) {
    if (!activo) return;

    tiempoVuelo += dt;

    if (modoRebote) {
        // Física de rebote para la arena del Nivel 2 (sin caída brusca por gravedad)
        posX += velX * dt;
        posY += velY * dt;

        // Rebote en suelo y techo de la arena
        if (posY >= 465.0f - radio) {
            posY = 465.0f - radio;
            rebotarVectorial(0.0f, -1.0f);
        } else if (posY <= -60.0f + radio) {
            posY = -60.0f + radio;
            rebotarVectorial(0.0f, 1.0f);
        }

        // Desactivar tras 4 segundos o 6 rebotes si el Capitán no lo atrapó antes
        if (tiempoVuelo >= 4.0f || rebotes >= 6) {
            activo = false;
        }
    } else {
        // Física parabólica original del Nivel 1
        const float gravedadEscudo = 420.0f;
        velY += gravedadEscudo * dt;
        posX += velX * dt;
        posY += velY * dt;

        if (posY >= 465.0f - radio) {
            posY = 465.0f - radio;
            rebotarVectorial(0.0f, -1.0f);
        }

        if (tiempoVuelo >= 1.15f || rebotes >= 3 || posX < 0.0f || posX > 8800.0f) {
            activo = false;
        }
    }
}

void EscudoVibranium::rebotarVectorial(float normalX, float normalY) {
    float factorConservar = modoRebote ? 0.95f : 0.85f;
    if (normalX != 0.0f) velX = -velX * factorConservar;
    if (normalY != 0.0f) velY = -velY * (modoRebote ? 0.90f : 0.75f);
    rebotes++;
}