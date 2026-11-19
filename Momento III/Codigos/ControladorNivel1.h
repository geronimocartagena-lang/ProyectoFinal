#ifndef CONTROLADORNIVEL1_H
#define CONTROLADORNIVEL1_H

#include <QObject>
#include <QTimer>
#include <QRectF>
#include <vector>
#include "CapitanAmerica.h"
#include "Civil.h"
#include "Enemigo.h"

struct Obstaculo {
    QRectF area;
    bool esFango;
};

class ControladorNivel1 : public QObject {
    Q_OBJECT

private:
    int tiempoRestanteSegundos;
    int nivelActual;
    int civilesEvacuados;
    bool juegoTerminado;
    bool misionCumplida;

    CapitanAmerica* heroe;
    std::vector<Civil*> listaCiviles;
    std::vector<Enemigo*> listaEnemigos;
    std::vector<BalaEnemiga> balasEnemigas;
    std::vector<Obstaculo> mapaTerreno;
    QRectF zonaEvacuacion;

    QTimer* timerReloj;

public:
    explicit ControladorNivel1(QObject* parent = nullptr);
    ~ControladorNivel1() override;

    void iniciarReloj();
    void verificarColisionesAABB();
    void actualizarReglas(float dt);

    CapitanAmerica* getHeroe() const { return heroe; }
    const std::vector<Civil*>& getCiviles() const { return listaCiviles; }
    const std::vector<Enemigo*>& getEnemigos() const { return listaEnemigos; }
    const std::vector<BalaEnemiga>& getBalasEnemigas() const { return balasEnemigas; }
    const std::vector<Obstaculo>& getMapaTerreno() const { return mapaTerreno; }
    QRectF getZonaEvacuacion() const { return zonaEvacuacion; }
    int getTiempoRestante() const { return tiempoRestanteSegundos; }
    int getCivilesEvacuados() const { return civilesEvacuados; }
    float obtenerFriccionActualHeroe() const;

signals:
    void tiempoActualizado(int segundos);
    void nivelFinalizado(bool victoria);

private slots:
    void descontarSegundo();
};

#endif // CONTROLADORNIVEL1_H