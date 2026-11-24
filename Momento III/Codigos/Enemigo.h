#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <QRectF>
#include <vector>

struct BalaEnemiga {
    float x;
    float y;
    float vx;
    bool activa;
};

class Enemigo {
private:
    float posX;
    float posY;
    float ancho;
    float alto;
    float minPatrullaX;
    float maxPatrullaX;
    float velocidad;
    int direccion;
    int vida;
    bool vivo;
    bool disparando;
    float cooldownDisparo;

public:
    Enemigo(float x, float y, float rangoPatrulla = 140.0f);

    void actualizar(float dt, float heroeX, std::vector<BalaEnemiga>& balas);
    void recibirDanio(int cantidad);

    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    int getDireccion() const { return direccion; }
    int getVidaActual() const { return vida; }
    bool isVivo() const { return vivo; }
    bool isDisparando() const { return disparando; }
    QRectF getHitBox() const { return QRectF(posX + 18.0f, posY + 10.0f, ancho - 36.0f, alto - 10.0f); }
};

#endif // ENEMIGO_H