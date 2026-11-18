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
    bool enSuelo;

public:
    Personaje(float x, float y, int salud, float w = 40.0f, float h = 60.0f);
    virtual ~Personaje() = default;

    // Métodos principales del diagrama
    virtual void actualizarFisica(float dt, float gravedad = 980.0f);
    virtual void recibirDanio(int cantidad);

    // Getters y Setters para colisiones y estado
    QRectF getHitBox() const;
    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    float getVelY() const { return velY; }
    int getPuntosSalud() const { return puntosSalud; }
    bool isEnSuelo() const { return enSuelo; }

    void setPosY(float y) { posY = y; }
    void setVelY(float vy) { velY = vy; }
    void setEnSuelo(bool estado) { enSuelo = estado; }
    void setVelocidad(float vx, float vy) { velX = vx; velY = vy; }
};

#endif // PERSONAJE_H