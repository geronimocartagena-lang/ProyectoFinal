#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QRectF>

class Personaje {
protected:
    float posX;
    float posY;
    float ancho;
    float alto;
    float velX;
    float velY;
    float velocidadBase;
    bool enSuelo;

public:
    Personaje(float x, float y, float w, float h, float vel);
    virtual ~Personaje() = default;

    virtual void actualizarFisica(float dt);

    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    float getVelX() const { return velX; }
    float getVelY() const { return velY; }
    bool isEnSuelo() const { return enSuelo; }

    void setPosX(float x) { posX = x; }
    void setPosY(float y) { posY = y; }
    void setVelX(float vx) { velX = vx; }
    void setVelY(float vy) { velY = vy; }
    void setEnSuelo(bool estado) { enSuelo = estado; }

    QRectF getHitBox() const { return QRectF(posX + 12.0f, posY + 5.0f, ancho - 24.0f, alto - 5.0f); }
};

#endif // PERSONAJE_H