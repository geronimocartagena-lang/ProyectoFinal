#include "Civil.h"
#include <cmath>

Civil::Civil(float x, float y)
    : posX(x), posY(y), ancho(70.0f), alto(95.0f), rescatado(false), evacuado(false), indiceFila(0) {}

void Civil::rescatar(int ordenEnFila) {
    rescatado = true;
    indiceFila = ordenEnFila;
}

void Civil::evacuar() {
    evacuado = true;
}

void Civil::seguirHeroe(float heroeX, float dt) {
    if (!rescatado || evacuado) return;

    const float sueloY = 370.0f;
    if (posY < sueloY) {
        posY += 420.0f * dt;
        if (posY > sueloY) posY = sueloY;
    }

    // Si el héroe está cerca de la Zona Segura (X >= 5950), todos los campesinos corren directo adentro
    float objetivoX = (heroeX >= 5950.0f)
                          ? 6280.0f
                          : (heroeX - (65.0f + indiceFila * 48.0f));

    float diff = objetivoX - posX;
    if (std::abs(diff) > 6.0f) {
        posX += diff * 4.8f * dt;
    }
}