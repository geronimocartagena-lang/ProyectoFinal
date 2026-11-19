#ifndef CIVIL_H
#define CIVIL_H

#include <QRectF>

class Civil {
private:
    float posX;
    float posY;
    float ancho;
    float alto;
    bool rescatado;
    bool evacuado;
    int indiceFila;

public:
    Civil(float x, float y);

    void rescatar(int ordenEnFila);
    void evacuar();
    void seguirHeroe(float heroeX, float dt);

    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    bool isRescatado() const { return rescatado; }
    bool isEvacuado() const { return evacuado; }
    QRectF getHitBox() const { return QRectF(posX + 10.0f, posY + 5.0f, ancho - 20.0f, alto - 5.0f); }
};

#endif // CIVIL_H