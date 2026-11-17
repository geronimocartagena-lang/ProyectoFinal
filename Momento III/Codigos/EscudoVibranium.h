#ifndef ESCUDOVIBRANIUM_H
#define ESCUDOVIBRANIUM_H

#include <QRectF>
#include <cmath>

class EscudoVibranium {
private:
    float posX;
    float posY;
    float velX;
    float velY;
    float radio;
    float anguloTiro;
    int rebotesRestantes;
    bool activo;

public:
    EscudoVibranium(float r = 15.0f);

    // Métodos del diagrama
    void calcularTiroBalistico(float origenX, float origenY, float anguloGrados, float velocidadInicial);
    void rebotarVectorial(float normalX, float normalY);
    void actualizarMovimiento(float dt, float gravedad = 4.5f);

    // Utilidades
    bool estaActivo() const { return activo; }
    void desactivar() { activo = false; }
    QRectF getHitBox() const;
    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
};

#endif // ESCUDOVIBRANIUM_H