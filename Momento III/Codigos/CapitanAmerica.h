#ifndef CAPITANAMERICA_H
#define CAPITANAMERICA_H

#include <QString>
#include <QRectF>
#include "Personaje.h"
#include "EscudoVibranium.h"
#include "Civil.h"

class CapitanAmerica : public Personaje {
private:
    EscudoVibranium* escudo; //clase padre
    int civilesRescatados;
    bool modoVeloz;
    bool bloqueando;
    bool atacandoCuerpoACuerpo;
    bool mirandoDerecha;
    float timerGolpe;
    bool golpeYaImpacto;

    int vidaMax;
    int vidaActual;
    float invulnerableTimer;

public:
    CapitanAmerica(float x, float y);
    ~CapitanAmerica() override;

    void moverHorizontal(int direccion, float friccionTerreno);
    void saltar();
    void lanzarEscudo(float anguloGrados);
    void iniciarGolpeCuerpoACuerpo();
    void bloquear(bool activo) { bloqueando = activo; }
    void rescatarCivil(Civil* civil);
    void activarModoVeloz();
    void actualizarEstado(float dt);

    void recibirDanio(int danio);
    QRectF getHitBoxGolpe() const;

    EscudoVibranium* getEscudo() const { return escudo; }
    int getCivilesRescatados() const { return civilesRescatados; }
    bool isModoVeloz() const { return modoVeloz; }
    bool isBloqueando() const { return bloqueando; }
    bool isAtacandoCuerpoACuerpo() const { return atacandoCuerpoACuerpo; }
    bool isGolpeYaImpacto() const { return golpeYaImpacto; }
    void marcarGolpeImpactado() { golpeYaImpacto = true; }
    int getVidaActual() const { return vidaActual; }
    int getVidaMax() const { return vidaMax; }
    QString getEstadoEspecial() const;
};

#endif // CAPITANAMERICA_H