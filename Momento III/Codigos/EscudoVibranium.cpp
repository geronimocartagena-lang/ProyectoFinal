#include "EscudoVibranium.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

EscudoVibranium::EscudoVibranium(float r)
    : posX(0), posY(0), velX(0), velY(0), radio(r), anguloTiro(0), rebotesRestantes(3), activo(false) {}

void EscudoVibranium::calcularTiroBalistico(float origenX, float origenY, float anguloGrados, float velocidadInicial) {
    posX = origenX;
    posY = origenY;
    anguloTiro = anguloGrados * (M_PI / 180.0f);
    velX = velocidadInicial * std::cos(anguloTiro);
    velY = -velocidadInicial * std::sin(anguloTiro);
    rebotesRestantes = 3;
    activo = true;
}

void EscudoVibranium::rebotarVectorial(float normalX, float normalY) {
    if (rebotesRestantes > 0) {
        // Modelo Físico 2: Rebote Elástico Vectorial -> vr = vi - 2*(vi . n)*n
        float productoPunto = (velX * normalX) + (velY * normalY);
        velX = velX - 2.0f * productoPunto * normalX;
        velY = velY - 2.0f * productoPunto * normalY;
        rebotesRestantes--;
    } else {
        activo = false;
    }
}

void EscudoVibranium::actualizarMovimiento(float dt, float gravedad) {
    if (!activo) return;

    velY += gravedad * dt;
    posX += velX * dt;
    posY += velY * dt;

    // Rebote automático contra el suelo (Y = 450)
    if (posY + radio >= 450.0f) {
        posY = 450.0f - radio;
        rebotarVectorial(0.0f, -1.0f);
    }
}

QRectF EscudoVibranium::getHitBox() const {
    return QRectF(posX - radio, posY - radio, radio * 2, radio * 2);
}