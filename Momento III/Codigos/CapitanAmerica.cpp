#include "CapitanAmerica.h"

CapitanAmerica::CapitanAmerica(float x, float y)
    : Personaje(x, y, 100, 45.0f, 65.0f),
    estadoEspecial("Normal"),
    tieneEscudo(true),
    bloqueando(false),
    tiempoEstadoEspecial(0.0f),
    multiplicadorVelocidad(1.0f),
    fuerzaEscudo(25) {
    escudo = new EscudoVibranium();
}

CapitanAmerica::~CapitanAmerica() {
    delete escudo;
}

void CapitanAmerica::rescatarCivil(Civil* civil) {
    if (civil != nullptr && !civil->isRescatado()) {
        civil->liberar();
        activarModoFortachon();
    }
}

void CapitanAmerica::activarModoFortachon() {
    estadoEspecial = "Fortachon";
    tiempoEstadoEspecial = 8.0f;
    fuerzaEscudo = 40;
    multiplicadorVelocidad = 0.85f;
}

void CapitanAmerica::activarModoVeloz() {
    estadoEspecial = "Veloz";
    tiempoEstadoEspecial = 10.0f;
    fuerzaEscudo = 25;
    multiplicadorVelocidad = 1.25f;
}

void CapitanAmerica::lanzarEscudo(float anguloGrados) {
    if (tieneEscudo && !escudo->estaActivo() && !bloqueando) {
        escudo->calcularTiroBalistico(posX + 22.0f, posY + 25.0f, anguloGrados, 520.0f);
        tieneEscudo = false;
    }
}

void CapitanAmerica::bloquear(bool estado) {
    if (tieneEscudo) {
        bloqueando = estado;
    }
}

void CapitanAmerica::actualizarEstado(float dt) {
    if (tiempoEstadoEspecial > 0.0f) {
        tiempoEstadoEspecial -= dt;
        if (tiempoEstadoEspecial <= 0.0f) {
            estadoEspecial = "Normal";
            multiplicadorVelocidad = 1.0f;
            fuerzaEscudo = 25;
        }
    }

    if (!tieneEscudo && !escudo->estaActivo()) {
        tieneEscudo = true;
    }
}

void CapitanAmerica::moverHorizontal(int direccion, float friccionTerreno) {
    // Modelo Físico 3: Rozamiento dinámico en fango
    float velocidadBase = 240.0f * multiplicadorVelocidad;
    velX = direccion * velocidadBase * (1.0f - friccionTerreno);
}

void CapitanAmerica::saltar() {
    if (enSuelo) {
        velY = -490.0f * multiplicadorVelocidad; // Impulso vertical calibrado con g = 980
        enSuelo = false;
    }
}