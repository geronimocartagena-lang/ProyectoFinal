#ifndef CONTROLADORNIVEL2_H
#define CONTROLADORNIVEL2_H

#include <QObject>
#include <QTimer>
#include <QRectF>
#include <vector>
#include "CapitanAmerica.h"
#include "JefeTrump.h"

struct PlataformaArena {
    float x;
    float y;
    float ancho;
    float alto;
};

struct ExplosionVisual {
    float x;
    float y;
    float tiempoVida;
    float duracionMax;
};

class ControladorNivel2 : public QObject {
    Q_OBJECT

private:
    CapitanAmerica* heroe;
    JefeTrump* jefe;
    std::vector<ProyectilJefe> proyectilesJefe;
    std::vector<PlataformaArena> plataformas;
    std::vector<ExplosionVisual> explosiones;

    QTimer* timerReloj;
    int tiempoRestanteSegundos;
    bool juegoTerminado;
    bool misionCumplida;
    bool agachado;
    float impulsoHeroeX;

    float timerBombardeo;
    int contadorBombas;

    void verificarColisionesArena();
    void procesarPlataformasHeroe();

public:
    explicit ControladorNivel2(QObject* parent = nullptr);
    ~ControladorNivel2() override;

    void iniciarReloj();
    void actualizarReglas(float dt);
    void setAgachado(bool estado);
    bool isAgachado() const { return agachado; }

    QRectF getHitBoxHeroeArena() const;
    CapitanAmerica* getHeroe() const { return heroe; }
    JefeTrump* getJefe() const { return jefe; }
    const std::vector<ProyectilJefe>& getProyectilesJefe() const { return proyectilesJefe; }
    const std::vector<PlataformaArena>& getPlataformas() const { return plataformas; }
    const std::vector<ExplosionVisual>& getExplosiones() const { return explosiones; }
    int getTiempoRestante() const { return tiempoRestanteSegundos; }
    bool isJuegoTerminado() const { return juegoTerminado; }

signals:
    void tiempoActualizado(int segundosRestantes);
    void nivelFinalizado(bool victoria);

private slots:
    void descontarSegundo();
};

#endif // CONTROLADORNIVEL2_H