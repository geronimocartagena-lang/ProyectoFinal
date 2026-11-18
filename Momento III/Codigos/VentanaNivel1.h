#ifndef VENTANANIVEL1_H
#define VENTANANIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
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
    QGraphicsTextItem* hudTexto;

    QPixmap pixQuieto;
    std::vector<QPixmap> framesCorrer;
    std::vector<QPixmap> framesSaltar;
    std::vector<QPixmap> framesEscudo;
    std::vector<std::vector<QPixmap>> framesCampesinos;

    int frameHeroeActual;
    int frameEscudoActual;
    int frameCivilActual;
    int contadorTicks;
    bool mirandoDerecha;

    bool teclaIzquierda;
    bool teclaDerecha;

    void construirEscenario();
    void cargarSprites();
    QPixmap limpiarFondoAjedrez(const QPixmap& original);
    QPixmap recortarBordesTransparentes(const QPixmap& pix);
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