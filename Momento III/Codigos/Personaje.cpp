#include "Personaje.h"

Personaje::Personaje(float x, float y, int salud, float w, float h)
    : posX(x), posY(y), velX(0.0f), velY(0.0f), puntosSalud(salud), ancho(w), alto(h), enSuelo(false) {}

void Personaje::actualizarFisica(float dt, float gravedad) {
    // Modelo Físico 1: Gravedad y Caída Libre (MRUA) en escala de píxeles
    velY += gravedad * dt;
    posX += velX * dt;
    posY += velY * dt;

    // Evitar salir por el borde izquierdo del mapa
    if (posX < 0.0f) {
        posX = 0.0f;
    }

    // Suelo base del nivel (Y = 450 es el piso, menos 'alto' del personaje)
    float limiteSuelo = 450.0f - alto;
    if (posY >= limiteSuelo) {
        posY = limiteSuelo;
        velY = 0.0f;
        enSuelo = true;
    } else {
        enSuelo = false;
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