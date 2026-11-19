#include "EscudoVibranium.h"
#include <cmath>

EscudoVibranium::EscudoVibranium()
    : posX(0.0f), posY(0.0f), velX(0.0f), velY(0.0f), radio(18.0f), activo(false), tiempoVuelo(0.0f) {}

void EscudoVibranium::lanzar(float x, float y, float anguloGrados, float rapidez) {
    posX = x;
    posY = y;
    float rad = anguloGrados * 3.14159265f / 180.0f;
    velX = std::cos(rad) * rapidez;
    velY = -std::sin(rad) * rapidez * 0.45f;
    activo = true;
    tiempoVuelo = 0.0f;
}

void EscudoVibranium::actualizarMovimiento(float dt) {
    if (!activo) return;

    const float gravedadEscudo = 420.0f;
    velY += gravedadEscudo * dt;
    posX += velX * dt;
    posY += velY * dt;
    tiempoVuelo += dt;

    if (posY >= 465.0f - radio) {
        posY = 465.0f - radio;
        rebotarVectorial(0.0f, -1.0f);
    }

    if (tiempoVuelo >= 1.35f || posX < 0.0f || posX > 4500.0f) {
        activo = false;
    }
}

void EscudoVibranium::rebotarVectorial(float normalX, float normalY) {
    if (normalX != 0.0f) velX = -velX * 0.85f;
    if (normalY != 0.0f) velY = -velY * 0.75f;
}