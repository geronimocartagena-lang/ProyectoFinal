#include "Civil.h"
#include <cmath>

Civil::Civil(float x, float y)
    : posX(x), posY(y), ancho(70.0f), alto(95.0f), rescatado(false), evacuado(false), indiceFila(0) {}

void Civil::rescatar(int ordenEnFila) {
    rescatado = true;
    indiceFila = ordenEnFila;
}

void Civil::evacuar() {
    evacuado = true;
}

void Civil::seguirHeroe(float heroeX, float dt) {
    if (!rescatado || evacuado) return;

    float distanciaDeseada = 65.0f + (indiceFila * 45.0f);
    float objetivoX = heroeX - distanciaDeseada;
    float diff = objetivoX - posX;

    if (std::abs(diff) > 8.0f) {
        posX += diff * 4.5f * dt;
    }
}