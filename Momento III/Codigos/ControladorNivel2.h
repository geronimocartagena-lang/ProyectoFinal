#ifndef CONTROLADORNIVEL2_H
#define CONTROLADORNIVEL2_H

#include <QObject>
#include <QTimer>
#include <QRectF>
#include <vector>
#include "CapitanAmerica.h"
#include "JefeTrump.h"

class ControladorNivel2 : public QObject {
    Q_OBJECT

private:
    int tiempoRestanteSegundos;
    bool juegoTerminado;
    bool misionCumplida;
    bool agachado;
    float impulsoHeroeX;
    float timerBombardeo;
    int contadorBombas;

    CapitanAmerica* heroe;
    JefeTrump* jefe;
    std::vector<ProyectilJefe> proyectilesJefe;

    QTimer* timerReloj;

public:
    explicit ControladorNivel2(QObject* parent = nullptr);
    ~ControladorNivel2() override;

    void iniciarReloj();
    void setAgachado(bool estado);
    bool isAgachado() const { return agachado; }
    QRectF getHitBoxHeroeArena() const;

    void verificarColisionesArena();
    void actualizarReglas(float dt);

    CapitanAmerica* getHeroe() const { return heroe; }
    JefeTrump* getJefe() const { return jefe; }
    const std::vector<ProyectilJefe>& getProyectilesJefe() const { return proyectilesJefe; }
    int getTiempoRestante() const { return tiempoRestanteSegundos; }

signals:
    void tiempoActualizado(int segundos);
    void nivelFinalizado(bool victoria);

private slots:
    void descontarSegundo();
};

#endif // CONTROLADORNIVEL2_H