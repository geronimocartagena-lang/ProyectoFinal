#include "Enemigo.h"
#include <cmath>

Enemigo::Enemigo(float x, float y, float rangoPatrulla)
    : posX(x),
    posY(y),
    ancho(85.0f),
    alto(105.0f),
    minPatrullaX(x - rangoPatrulla),
    maxPatrullaX(x + rangoPatrulla),
    velocidad(75.0f),
    direccion(-1),
    vida(2),
    vivo(true),
    disparando(false),
    cooldownDisparo(1.2f) {}

void Enemigo::actualizar(float dt, float heroeX, std::vector<BalaEnemiga>& balas) {
    if (!vivo) return;

    if (cooldownDisparo > 0.0f) {
        cooldownDisparo -= dt;
    }

    float distanciaX = heroeX - posX;
    float absDist = std::abs(distanciaX);

    if (absDist < 460.0f) {
        disparando = true;
        direccion = (distanciaX >= 0.0f) ? 1 : -1;

        if (cooldownDisparo <= 0.0f) {
            float salidaX = (direccion > 0) ? (posX + ancho - 5.0f) : (posX + 5.0f);
            float salidaY = posY + 42.0f;
            balas.push_back({salidaX, salidaY, 430.0f * direccion, true});
            cooldownDisparo = 2.1f;
        }
    } else {
        disparando = false;
        posX += velocidad * direccion * dt;
        if (posX <= minPatrullaX) {
            posX = minPatrullaX;
            direccion = 1;
        } else if (posX >= maxPatrullaX) {
            posX = maxPatrullaX;
            direccion = -1;
        }
    }
}

void Enemigo::recibirDanio(int cantidad) {
    if (!vivo) return;
    vida -= cantidad;
    if (vida <= 0) {
        vida = 0;
        vivo = false;
    }
}