#include "JefeTrump.h"
#include <cmath>
#include <algorithm>

JefeTrump::JefeTrump(float x, float y)
    : Personaje(x, y, 88.0f, 112.0f, 210.0f),
    vidaMax(300),
    vidaActual(300),
    faseActual(1),
    direccion(-1),
    estadoIA(EstadoIATrump::Patrullando),
    timerEstado(0.8f),
    cooldownAccion(1.1f),
    timerAturdido(0.0f),
    invulnerableTimer(0.0f),
    impulsoX(0.0f),
    memSaltosJugador(0),
    memAgachadasJugador(0),
    memBloqueosJugador(0),
    memEscudosJugador(0) {}

void JefeTrump::percibirYAprender(bool jugadorSaltando, bool jugadorAgachado, bool jugadorBloqueando, bool escudoActivo) {
    if (jugadorSaltando) memSaltosJugador++;
    if (jugadorAgachado) memAgachadasJugador++;
    if (jugadorBloqueando) memBloqueosJugador++;
    if (escudoActivo) memEscudosJugador++;
}

void JefeTrump::decidirSiguienteAccion(float heroeX) {
    float distancia = std::abs(heroeX - posX);
    direccion = (heroeX >= posX) ? 1 : -1;

    int pesoEmbestida = 45;
    int pesoDisparo   = 55;

    if (memBloqueosJugador > memSaltosJugador + 20) {
        pesoEmbestida += 30;
    }
    if (distancia < 340.0f) {
        pesoEmbestida += 20;
    } else {
        pesoDisparo += 25;
    }

    int total = pesoEmbestida + pesoDisparo;
    int eleccion = (static_cast<int>(posX + vidaActual * 7 + memBloqueosJugador) % total);

    if (eleccion < pesoEmbestida) {
        estadoIA = EstadoIATrump::PreparandoEmbestida;
        timerEstado = (faseActual == 2) ? 0.35f : 0.55f;
    } else {
        estadoIA = EstadoIATrump::PreparandoDisparo;
        timerEstado = (faseActual == 2) ? 0.28f : 0.45f;
    }
}

void JefeTrump::actualizarIA(float dt, float heroeX, bool jugadorSaltando, bool jugadorAgachado,
                             bool jugadorBloqueando, bool escudoActivo, std::vector<ProyectilJefe>& proyectiles) {
    if (!isVivo()) return;

    if (faseActual == 1 && vidaActual <= vidaMax / 2) {
        faseActual = 2;
        velocidadBase = 325.0f;
    }

    if (invulnerableTimer > 0.0f) invulnerableTimer -= dt;

    if (std::abs(impulsoX) > 1.0f) {
        posX += impulsoX * dt;
        impulsoX *= std::pow(0.06f, dt);
    } else {
        impulsoX = 0.0f;
    }

    percibirYAprender(jugadorSaltando, jugadorAgachado, jugadorBloqueando, escudoActivo);

    // Reacción de salto táctico si el escudo viene hacia Trump y está en el suelo
    if (escudoActivo && enSuelo && estadoIA == EstadoIATrump::Patrullando && memEscudosJugador > 12 && (static_cast<int>(posX) % 3 == 0)) {
        estadoIA = EstadoIATrump::Saltando;
        velY = -520.0f;
        enSuelo = false;
        velX = direccion * velocidadBase * 1.1f;
    }

    if (estadoIA == EstadoIATrump::Aturdido) {
        velX = 0.0f;
        timerAturdido -= dt;
        if (timerAturdido <= 0.0f) {
            estadoIA = EstadoIATrump::Patrullando;
            cooldownAccion = (faseActual == 2) ? 0.35f : 0.65f;
        }
    } else {
        switch (estadoIA) {
        case EstadoIATrump::Patrullando: {
            direccion = (heroeX >= posX) ? 1 : -1;
            float dist = std::abs(heroeX - posX);
            float factorFase = (faseActual == 2) ? 1.35f : 1.0f;

            if (dist > 460.0f) {
                velX = direccion * velocidadBase * 0.85f * factorFase;
            } else if (dist < 220.0f) {
                velX = -direccion * velocidadBase * 0.65f * factorFase;
            } else {
                velX = direccion * velocidadBase * 0.45f * factorFase;
            }

            cooldownAccion -= dt;
            if (cooldownAccion <= 0.0f) {
                decidirSiguienteAccion(heroeX);
            }
            break;
        }

        case EstadoIATrump::Saltando: {
            if (enSuelo && velY >= 0.0f) {
                estadoIA = EstadoIATrump::Patrullando;
                cooldownAccion = 0.45f;
            }
            break;
        }

        case EstadoIATrump::PreparandoDisparo: {
            velX = 0.0f;
            direccion = (heroeX >= posX) ? 1 : -1;
            timerEstado -= dt;
            if (timerEstado <= 0.0f) {
                estadoIA = EstadoIATrump::Disparando;
                timerEstado = (faseActual == 2) ? 0.25f : 0.35f;

                float rapidezBala = (faseActual == 2) ? 640.0f : 490.0f;
                float origenX = (direccion > 0) ? (posX + ancho) : (posX - 24.0f);
                float alturaDisparo = (memSaltosJugador > memAgachadasJugador + 15)
                                          ? (posY + 32.0f)
                                          : (posY + 38.0f);

                proyectiles.push_back({origenX, alturaDisparo, direccion * rapidezBala, 0.0f, false, true});

                if (faseActual == 2) {
                    proyectiles.push_back({origenX - direccion * 45.0f, posY + 68.0f, direccion * (rapidezBala * 0.92f), 0.0f, false, true});
                }
            }
            break;
        }

        case EstadoIATrump::Disparando: {
            velX = 0.0f;
            timerEstado -= dt;
            if (timerEstado <= 0.0f) {
                estadoIA = EstadoIATrump::Patrullando;
                cooldownAccion = (faseActual == 2) ? 0.55f : 1.05f;
            }
            break;
        }

        case EstadoIATrump::PreparandoEmbestida: {
            velX = -direccion * 45.0f;
            direccion = (heroeX >= posX) ? 1 : -1;
            timerEstado -= dt;
            if (timerEstado <= 0.0f) {
                estadoIA = EstadoIATrump::Embistiendo;
                timerEstado = (faseActual == 2) ? 0.85f : 0.95f;
            }
            break;
        }

        case EstadoIATrump::Embistiendo: {
            float velCarga = (faseActual == 2) ? 690.0f : 520.0f;
            velX = direccion * velCarga;
            timerEstado -= dt;

            if (posX <= 20.0f && direccion < 0) {
                posX = 20.0f;
                estadoIA = EstadoIATrump::Patrullando;
                cooldownAccion = 0.7f;
            } else if (posX >= 1260.0f - ancho && direccion > 0) {
                posX = 1260.0f - ancho;
                estadoIA = EstadoIATrump::Patrullando;
                cooldownAccion = 0.7f;
            } else if (timerEstado <= 0.0f) {
                estadoIA = EstadoIATrump::Patrullando;
                cooldownAccion = (faseActual == 2) ? 0.65f : 1.15f;
            }
            break;
        }

        case EstadoIATrump::Aturdido:
            break;
        }
    }

    const float gravedad = 980.0f;
    const float limiteSuelo = 465.0f - alto;

    posX += velX * dt;
    if (posX < 10.0f) posX = 10.0f;
    if (posX > 1270.0f - ancho) posX = 1270.0f - ancho;

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

void JefeTrump::recibirDanio(int danio, float impulsoVectorialX, bool rompePostura) {
    if (!isVivo() || invulnerableTimer > 0.0f) return;

    vidaActual -= danio;
    if (vidaActual < 0) vidaActual = 0;

    impulsoX = impulsoVectorialX;
    invulnerableTimer = 0.28f;

    if (rompePostura && vidaActual > 0) {
        estadoIA = EstadoIATrump::Aturdido;
        timerAturdido = (faseActual == 2) ? 0.65f : 1.05f;
        velX = 0.0f;
    }
}

void JefeTrump::aplicarImpulso(float vx, float vy) {
    impulsoX = vx;
    velY = vy;
    if (vy < 0.0f) enSuelo = false;
}

QString JefeTrump::getNombreEstadoIA() const {
    if (faseActual == 2) {
        switch (estadoIA) {
        case EstadoIATrump::Aturdido: return "FASE 2 • ATURDIDO";
        case EstadoIATrump::Embistiendo: return "FASE 2 • EMBESTIDA FURIA";
        case EstadoIATrump::PreparandoEmbestida: return "FASE 2 • ¡VA A EMBESTIR!";
        case EstadoIATrump::Saltando: return "FASE 2 • SALTO EVASIVO";
        case EstadoIATrump::PreparandoDisparo:
        case EstadoIATrump::Disparando: return "FASE 2 • FUEGO + BOMBAS";
        default: return "FASE 2 • BOMBARDEO AÉREO";
        }
    }
    switch (estadoIA) {
    case EstadoIATrump::Aturdido: return "FASE 1 • POSTURA ROTA";
    case EstadoIATrump::Embistiendo: return "FASE 1 • EMBESTIDA";
    case EstadoIATrump::PreparandoEmbestida: return "FASE 1 • PREPARANDO CARGA";
    case EstadoIATrump::Saltando: return "FASE 1 • SALTO";
    case EstadoIATrump::PreparandoDisparo:
    case EstadoIATrump::Disparando: return "FASE 1 • DISPARANDO";
    default: return "FASE 1 • ANALIZANDO";
    }
}