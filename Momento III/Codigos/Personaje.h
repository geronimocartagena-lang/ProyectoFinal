#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QRectF>

class Personaje {
protected:
    float posX;
    float posY;
    float velX;
    float velY;
    int puntosSalud;
    float ancho;
    float alto;

public:
    Personaje(float x, float y, int salud, float w = 40.0f, float h = 60.0f);
    virtual ~Personaje() = default;

    virtual void actualizarFisica(float dt, float gravedad = 9.8f);
    virtual void recibirDanio(int cantidad);

    QRectF getHitBox() const;
    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    int getPuntosSalud() const { return puntosSalud; }
    void setVelocidad(float vx, float vy) { velX = vx; velY = vy; }
};

#endif // PERSONAJE_H