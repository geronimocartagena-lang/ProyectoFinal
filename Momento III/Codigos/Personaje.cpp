#include "Personaje.h"

Personaje::Personaje(float x, float y, float w, float h, float vel)
    : posX(x),
    posY(y),
    ancho(w),
    alto(h),
    velX(0.0f),
    velY(0.0f),
    velocidadBase(vel),
    enSuelo(true) {}

void Personaje::actualizarFisica(float dt) {
    const float gravedad = 980.0f;
    const float limiteSuelo = 465.0f - alto;

    posX += velX * dt;
    if (posX < 0.0f) posX = 0.0f;
    if (posX > 4420.0f) posX = 4420.0f;

    velY += gravedad * dt;
    posY += velY * dt;

    if (posY >= limiteSuelo) {
        posY = limiteSuelo;
        velY = 0.0f;
        enSuelo = true;
    } else {
        enSuelo = false;
    }
}