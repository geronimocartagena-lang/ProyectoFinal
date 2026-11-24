#ifndef VENTANANIVEL1_H
#define VENTANANIVEL1_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QTimer>
#include <QPixmap>
#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <vector>
#include "ControladorNivel1.h"

class VentanaNivel1 : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* escena;
    ControladorNivel1* controlador;
    QTimer* gameLoopTimer;

    QGraphicsPixmapItem* itemHeroe;
    QGraphicsPixmapItem* itemEscudo;
    std::vector<QGraphicsPixmapItem*> itemsCiviles;
    std::vector<QGraphicsPixmapItem*> itemsEnemigos;
    std::vector<QGraphicsPixmapItem*> poolBalasEnemigas;

    QGraphicsTextItem* hudTexto;
    QGraphicsPixmapItem* itemBarraVida;
    int ultimaVidaRenderizada;
    int vidaPreviaHeroe;
    int vidaTotalEnemigosPrevia;
    QWidget* overlayFinNivel;

    // Audio
    QMediaPlayer* musicaFondo;
    QAudioOutput* salidaMusicaFondo;
    QMediaPlayer* sfxLodo;
    QAudioOutput* salidaSfxLodo;
    QMediaPlayer* sfxDisparo;
    QAudioOutput* salidaSfxDisparo;
    QMediaPlayer* sfxGameOver;
    QAudioOutput* salidaSfxGameOver;
    QMediaPlayer* sfxSaltar;
    QAudioOutput* salidaSfxSaltar;
    QMediaPlayer* sfxRecibeDano;
    QAudioOutput* salidaSfxRecibeDano;
    QMediaPlayer* sfxDolorEnemigo;
    QAudioOutput* salidaSfxDolorEnemigo;
    size_t conteoBalasPrevio;

    QPixmap pixQuieto;
    QPixmap pixBalaEnemiga;
    std::vector<QPixmap> framesCorrer;
    std::vector<QPixmap> framesSaltar;
    std::vector<QPixmap> framesGolpear;
    std::vector<QPixmap> framesDefensa;
    std::vector<QPixmap> framesEscudo;
    std::vector<std::vector<QPixmap>> framesCampesinos;
    std::vector<QPixmap> framesEnemigoCaminar;
    std::vector<QPixmap> framesEnemigoDisparar;

    int frameHeroeActual;
    int frameGolpeActual;
    int frameDefensaActual;
    int frameEscudoActual;
    int frameCivilActual;
    int frameEnemigoActual;
    int contadorTicks;
    bool mirandoDerecha;

    bool teclaIzquierda;
    bool teclaDerecha;

    void inicializarAudio();
    void construirEscenario();
    void cargarSprites();
    QPixmap limpiarFondoAjedrez(const QPixmap& original);
    QPixmap limpiarVerdeChroma(const QPixmap& original);
    QPixmap recortarBordesTransparentes(const QPixmap& pix, bool recortarAnchoExtra = true);
    void extraerSpritesPorBlob(const QPixmap& sheet, int yInicio, int yFin, int altoReferenciaMax, int altoObjetivo, std::vector<QPixmap>& destino);
    void actualizarAnimaciones(int direccion);
    void refrescarTextoHUD();
    void actualizarBarraVidaPixelArt(int vidaActual, int vidaMax);
    void mostrarPantallaFinNivel(bool victoria);

public:
    explicit VentanaNivel1(QWidget* parent = nullptr);
    ~VentanaNivel1() override;

signals:
    void regresarAlMenu();
    void iniciarNivel2();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void actualizarFrame();
    void manejarFinNivel(bool victoria);
};

#endif // VENTANANIVEL1_H