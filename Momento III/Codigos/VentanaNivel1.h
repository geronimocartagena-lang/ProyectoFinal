#ifndef VENTANANIVEL1_H
#define VENTANANIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QTimer>
#include <vector>
#include "ControladorNivel1.h"

class VentanaNivel1 : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* escena;
    ControladorNivel1* controlador;
    QTimer* gameLoopTimer;

    QGraphicsRectItem* itemHeroe;
    QGraphicsEllipseItem* itemEscudo;
    std::vector<QGraphicsRectItem*> itemsCiviles;
    QGraphicsTextItem* hudTexto;

    bool mirandoDerecha;
    bool teclaIzquierda;
    bool teclaDerecha;

    void construirEscenario();

public:
    explicit VentanaNivel1(QWidget* parent = nullptr);
    ~VentanaNivel1() override;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void actualizarFrame();
    void actualizarHUD(int segundosRestantes);
    void manejarFinNivel(bool victoria);
};

#endif // VENTANANIVEL1_H