#ifndef VENTANANIVEL1_H
#define VENTANANIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QPixmap>
#include <vector>
#include "ControladorNivel1.h"

class VentanaNivel1 : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* escena;
    ControladorNivel1* controlador;
    QTimer* gameLoopTimer;

    QGraphicsPixmapItem* itemHeroe;
    QGraphicsPixmapItem* itemEscudo;
    std::vector<QGraphicsPixmapItem*> itemsCiviles;
    std::vector<QGraphicsPixmapItem*> itemsEnemigos;
    std::vector<QGraphicsPixmapItem*> poolBalasEnemigas;

    QGraphicsTextItem* hudTexto;
    QGraphicsRectItem* barraVidaFondo;
    QGraphicsRectItem* barraVidaRelleno;
    QGraphicsTextItem* textoVida;

    QPixmap pixQuieto;
    std::vector<QPixmap> framesCorrer;
    std::vector<QPixmap> framesSaltar;
    std::vector<QPixmap> framesGolpear;
    std::vector<QPixmap> framesEscudo;
    std::vector<std::vector<QPixmap>> framesCampesinos;
    std::vector<QPixmap> framesEnemigoCaminar;
    std::vector<QPixmap> framesEnemigoDisparar;
    QPixmap pixBalaEnemiga;

    int frameHeroeActual;
    int frameGolpeActual;
    int frameEscudoActual;
    int frameCivilActual;
    int frameEnemigoActual;
    int contadorTicks;
    bool mirandoDerecha;

    bool teclaIzquierda;
    bool teclaDerecha;

    void construirEscenario();
    void cargarSprites();
    QPixmap limpiarFondoAjedrez(const QPixmap& original);
    QPixmap recortarBordesTransparentes(const QPixmap& pix);
    void extraerSpritesPorBlob(const QPixmap& sheet, int yInicio, int yFin, std::vector<QPixmap>& destino);
    void actualizarAnimaciones(int direccion);
    void refrescarTextoHUD();

public:
    explicit VentanaNivel1(QWidget* parent = nullptr);
    ~VentanaNivel1() override;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void actualizarFrame();
    void manejarFinNivel(bool victoria);
};

#endif // VENTANANIVEL1_H