#ifndef JEFETRUMP_H
#define JEFETRUMP_H

#include <QRectF>
#include <QString>
#include <vector>
#include "Personaje.h"

struct ProyectilJefe {
    float x;
    float y;
    float vx;
    float vy;
    bool esBomba;
    bool activa;
};

enum class EstadoIATrump {
    Patrullando,
    PreparandoDisparo,
    Disparando,
    PreparandoEmbestida,
    Embistiendo,
    Aturdido
};

class JefeTrump : public Personaje {
private:
    int vidaMax;
    int vidaActual;
    int faseActual; // 1 = Disparos y Embestidas | 2 = Velocidad + Bombardeo Aéreo
    int direccion;  // -1 izquierda, 1 derecha

    EstadoIATrump estadoIA;
    float timerEstado;
    float cooldownAccion;
    float timerAturdido;
    float invulnerableTimer;
    float impulsoX;

    // Memoria de aprendizaje dinámico sobre los hábitos del jugador
    int memSaltosJugador;
    int memAgachadasJugador;
    int memBloqueosJugador;
    int memEscudosJugador;

    void percibirYAprender(bool jugadorSaltando, bool jugadorAgachado, bool jugadorBloqueando, bool escudoActivo);
    void decidirSiguienteAccion(float heroeX);

public:
    JefeTrump(float x, float y);

    void actualizarIA(float dt, float heroeX, bool jugadorSaltando, bool jugadorAgachado,
                      bool jugadorBloqueando, bool escudoActivo, std::vector<ProyectilJefe>& proyectiles);

    void recibirDanio(int danio, float impulsoVectorialX, bool rompePostura);
    void aplicarImpulso(float vx, float vy);

    int getVidaActual() const { return vidaActual; }
    int getVidaMax() const { return vidaMax; }
    int getFaseActual() const { return faseActual; }
    int getDireccion() const { return direccion; }
    EstadoIATrump getEstadoIA() const { return estadoIA; }
    bool isAturdido() const { return estadoIA == EstadoIATrump::Aturdido; }
    bool isEmbistiendo() const { return estadoIA == EstadoIATrump::Embistiendo; }
    bool isDisparando() const { return estadoIA == EstadoIATrump::Disparando || estadoIA == EstadoIATrump::PreparandoDisparo; }
    bool isVivo() const { return vidaActual > 0; }
    QString getNombreEstadoIA() const;
};

#endif // JEFETRUMP_H