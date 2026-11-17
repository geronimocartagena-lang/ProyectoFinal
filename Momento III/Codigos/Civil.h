#ifndef CIVIL_H
#define CIVIL_H

#include "Personaje.h"

class Civil : public Personaje {
private:
    bool rescatado;
    bool enZonaEvacuacion;

public:
    Civil(float x, float y);

    // Métodos del diagrama
    void seguirHeroe(float heroeX, float dt);
    void evacuar();

    // Getters y Setters
    void liberar() { rescatado = true; }
    bool isRescatado() const { return rescatado; }
    bool isEvacuado() const { return enZonaEvacuacion; }
};

#endif // CIVIL_H