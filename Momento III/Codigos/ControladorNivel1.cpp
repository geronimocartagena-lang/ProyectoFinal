#include "ControladorNivel1.h"

ControladorNivel1::ControladorNivel1(QObject* parent)
    : QObject(parent),
    tiempoRestanteSegundos(180), // 3:00 minutos exactos
    nivelActual(1),
    civilesEvacuados(0),
    juegoTerminado(false),
    misionCumplida(false) {

    // 1. Instanciar al protagonista en memoria dinámica
    heroe = new CapitanAmerica(100.0f, 385.0f);

    // 2. Poblar los 5 grupos de civiles a rescatar en el pueblo (memoria dinámica)
    listaCiviles.push_back(new Civil(450.0f, 395.0f));
    listaCiviles.push_back(new Civil(850.0f, 395.0f));
    listaCiviles.push_back(new Civil(1300.0f, 395.0f));
    listaCiviles.push_back(new Civil(1750.0f, 395.0f));
    listaCiviles.push_back(new Civil(2150.0f, 395.0f));

    // 3. Configurar plataformas (escombros) y zonas de fango (Física de rozamiento)
    mapaTerreno.push_back({QRectF(300.0f, 340.0f, 150.0f, 20.0f), false});  // Plataforma madera
    mapaTerreno.push_back({QRectF(600.0f, 445.0f, 220.0f, 15.0f), true});   // Zona de fango 1
    mapaTerreno.push_back({QRectF(1050.0f, 320.0f, 180.0f, 20.0f), false}); // Plataforma techo
    mapaTerreno.push_back({QRectF(1450.0f, 445.0f, 260.0f, 15.0f), true});  // Zona de fango 2

    // 4. Zona segura al inicio/extremo del pueblo
    zonaEvacuacion = QRectF(2400.0f, 330.0f, 140.0f, 120.0f);

    // 5. Configurar el reloj con sintaxis moderna de Qt
    timerReloj = new QTimer(this);
    connect(timerReloj, &QTimer::timeout, this, &ControladorNivel1::descontarSegundo);
}

ControladorNivel1::~ControladorNivel1() {
    delete heroe;
    for (Civil* c : listaCiviles) {
        delete c;
    }
    listaCiviles.clear();
}

void ControladorNivel1::iniciarReloj() {
    timerReloj->start(1000); // Dispara cada 1000 ms (1 segundo)
}

void ControladorNivel1::descontarSegundo() {
    if (juegoTerminado) return;

    tiempoRestanteSegundos--;
    emit tiempoActualizado(tiempoRestanteSegundos);

    // Activar personalidad "Veloz / Salterín" bajo presión en los últimos 30 segundos
    if (tiempoRestanteSegundos == 30) {
        heroe->activarModoVeloz();
    }

    if (tiempoRestanteSegundos <= 0) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = false;
        emit nivelFinalizado(false);
    }
}

float ControladorNivel1::obtenerFriccionActualHeroe() const {
    QRectF boxHeroe = heroe->getHitBox();
    for (const auto& obs : mapaTerreno) {
        if (obs.esFango && boxHeroe.intersects(obs.area)) {
            return 0.55f; // Reduce velocidad al 45% en el fango
        }
    }
    return 0.0f; // Terreno seco normal
}

void ControladorNivel1::verificarColisionesAABB() {
    QRectF boxHeroe = heroe->getHitBox();
    QRectF boxEscudo = heroe->getEscudo()->getHitBox();

    // A. Colisión del Héroe con Plataformas Sólidas (AABB)
    for (const auto& obs : mapaTerreno) {
        if (!obs.esFango && boxHeroe.intersects(obs.area)) {
            // Si cae desde arriba sobre la plataforma
            if (boxHeroe.bottom() <= obs.area.top() + 15.0f) {
                heroe->setVelocidad(0.0f, 0.0f);
            }
        }
        // B. Rebote vectorial del escudo contra escombros sólidos
        if (!obs.esFango && heroe->getEscudo()->estaActivo() && boxEscudo.intersects(obs.area)) {
            heroe->getEscudo()->rebotarVectorial(0.0f, -1.0f);
        }
    }

    // C. Interacción Héroe <-> Civiles (Rescate y Evacuación)
    for (Civil* civil : listaCiviles) {
        if (!civil->isRescatado() && boxHeroe.intersects(civil->getHitBox())) {
            heroe->rescatarCivil(civil); // Activa modo Fortachón
        }

        if (civil->isRescatado() && !civil->isEvacuado() && civil->getHitBox().intersects(zonaEvacuacion)) {
            civil->evacuar();
            civilesEvacuados++;
        }
    }
}

void ControladorNivel1::actualizarReglas(float dt) {
    if (juegoTerminado) return;

    // 1. Actualizar físicas y estados
    heroe->actualizarEstado(dt);
    heroe->actualizarFisica(dt);
    heroe->getEscudo()->actualizarMovimiento(dt);

    // 2. Actualizar movimiento de civiles rescatados
    for (Civil* civil : listaCiviles) {
        civil->seguirHeroe(heroe->getPosX(), dt);
    }

    // 3. Evaluar colisiones AABB
    verificarColisionesAABB();

    // 4. Verificar condición de victoria (5 grupos rescatados)
    if (civilesEvacuados >= static_cast<int>(listaCiviles.size())) {
        timerReloj->stop();
        juegoTerminado = true;
        misionCumplida = true;
        emit nivelFinalizado(true);
    }
}