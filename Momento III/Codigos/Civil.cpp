#include "Civil.h"
#include <cmath>

Civil::Civil(float x, float y)
    : Personaje(x, y, 50, 35.0f, 55.0f), rescatado(false), enZonaEvacuacion(false) {}

void Civil::seguirHeroe(float heroeX, float dt) {
    if (!rescatado || enZonaEvacuacion) {
        velX = 0.0f;
        actualizarFisica(dt);
        return;
    }

    float distancia = heroeX - posX;
    if (std::abs(distancia) > 55.0f) {
        velX = (distancia > 0) ? 160.0f : -160.0f;
    } else {
        velX = 0.0f;
    }
    actualizarFisica(dt);
}

void Civil::evacuar() {
    enZonaEvacuacion = true;
    velX = 0.0f;
}