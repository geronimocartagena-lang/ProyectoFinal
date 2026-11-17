#ifndef CONTROLADORNIVEL1_H
#define CONTROLADORNIVEL1_H

#include <QObject>
#include <QTimer>
#include <QRectF>
#include <vector>
#include "CapitanAmerica.h"
#include "Civil.h"

struct Obstaculo {
    QRectF area;
    bool esFango; // true = aplica rozamiento dinámico, false = plataforma sólida
};

class ControladorNivel1 : public QObject {
    Q_OBJECT

private:
    int tiempoRestanteSegundos; // Requisito: Nivel controlado por tiempo (180s)
    int nivelActual;
    int civilesEvacuados;
    bool juegoTerminado;
    bool misionCumplida;

    CapitanAmerica* heroe;
    std::vector<Civil*> listaCiviles; // Contenedor STL con memoria dinámica
    std::vector<Obstaculo> mapaTerreno;
    QRectF zonaEvacuacion;

    QTimer* timerReloj; // Temporizador de 1 segundo para el cronómetro

public:
    explicit ControladorNivel1(QObject* parent = nullptr);
    ~ControladorNivel1() override;

    // Métodos definidos en tu diagrama de clases
    void iniciarReloj();
    void verificarColisionesAABB();
    void actualizarReglas(float dt);

    // Getters para la Capa Gráfica (GUI)
    CapitanAmerica* getHeroe() const { return heroe; }
    const std::vector<Civil*>& getCiviles() const { return listaCiviles; }
    const std::vector<Obstaculo>& getMapaTerreno() const { return mapaTerreno; }
    QRectF getZonaEvacuacion() const { return zonaEvacuacion; }
    int getTiempoRestante() const { return tiempoRestanteSegundos; }
    int getCivilesEvacuados() const { return civilesEvacuados; }
    bool isJuegoTerminado() const { return juegoTerminado; }
    bool isMisionCumplida() const { return misionCumplida; }
    float obtenerFriccionActualHeroe() const;

signals:
    void tiempoActualizado(int segundos);
    void nivelFinalizado(bool victoria);

private slots:
    void descontarSegundo();
};

#endif // CONTROLADORNIVEL1_H