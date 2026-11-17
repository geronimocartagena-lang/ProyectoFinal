#include "Personaje.h"

Personaje::Personaje(float x, float y, int salud, float w, float h)
    : posX(x), posY(y), velX(0.0f), velY(0.0f), puntosSalud(salud), ancho(w), alto(h) {}

void Personaje::actualizarFisica(float dt, float gravedad) {
    // Modelo Físico 1: Gravedad y Caída Libre (MRUA)
    velY += gravedad * dt;
    posX += velX * dt;
    posY += velY * dt;

    if (posY >= 450.0f) {
        posY = 450.0f;
        velY = 0.0f;
    }
}

void Personaje::recibirDanio(int cantidad) {
    puntosSalud -= cantidad;
    if (puntosSalud < 0) {
        puntosSalud = 0;
    }
}

QRectF Personaje::getHitBox() const {
    return QRectF(posX, posY, ancho, alto);
}