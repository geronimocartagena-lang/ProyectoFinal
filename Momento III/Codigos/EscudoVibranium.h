#ifndef ESCUDOVIBRANIUM_H
#define ESCUDOVIBRANIUM_H

#include <QRectF>

class EscudoVibranium {
private:
    float posX;
    float posY;
    float velX;
    float velY;
    float radio;
    bool activo;
    float tiempoVuelo;
    int rebotes;
    bool modoRebote;

public:
    EscudoVibranium();

    void lanzar(float x, float y, float anguloGrados, float rapidez);
    void actualizarMovimiento(float dt);
    void rebotarVectorial(float normalX, float normalY);
    void desactivar() { activo = false; }

    void setModoRebote(bool activar) { modoRebote = activar; }
    void setPosX(float x) { posX = x; }
    void setPosY(float y) { posY = y; }
    void reiniciarTiempo() { tiempoVuelo = 0.0f; }

    bool estaActivo() const { return activo; }
    float getTiempoActivo() const { return tiempoVuelo; }
    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    QRectF getHitBox() const { return QRectF(posX - radio, posY - radio, radio * 2.0f, radio * 2.0f); }
};

#endif // ESCUDOVIBRANIUM_H