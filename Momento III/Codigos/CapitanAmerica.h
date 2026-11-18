#ifndef CAPITANAMERICA_H
#define CAPITANAMERICA_H

#include "Personaje.h"
#include "Civil.h"
#include "EscudoVibranium.h"
#include <QString>

class CapitanAmerica : public Personaje {
private:
    QString estadoEspecial; // "Normal", "Fortachon", "Veloz"
    bool tieneEscudo;
    bool bloqueando;
    float tiempoEstadoEspecial;
    float multiplicadorVelocidad;
    int fuerzaEscudo;
    EscudoVibranium* escudo; // Asociación 1 a 1

public:
    CapitanAmerica(float x, float y);
    ~CapitanAmerica() override;

    // Métodos del diagrama
    void rescatarCivil(Civil* civil);
    void activarModoFortachon();
    void activarModoVeloz();
    void lanzarEscudo(float anguloGrados);
    void bloquear(bool estado);

    // Actualización y control
    void actualizarEstado(float dt);
    void moverHorizontal(int direccion, float friccionTerreno);
    void saltar();

    // Getters
    bool isBloqueando() const { return bloqueando; }
    QString getEstadoEspecial() const { return estadoEspecial; }
    EscudoVibranium* getEscudo() const { return escudo; }
    int getFuerzaEscudo() const { return fuerzaEscudo; }
};

#endif // CAPITANAMERICA_H