#ifndef VENTANANIVEL2_H
#define VENTANANIVEL2_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QTimer>
#include <QPixmap>
#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <vector>
#include "ControladorNivel2.h"

class VentanaNivel2 : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* escena;
    ControladorNivel2* controlador;
    QTimer* gameLoopTimer;

    QGraphicsPixmapItem* itemHeroe;
    QGraphicsPixmapItem* itemEscudo;
    QGraphicsPixmapItem* itemJefe;
    std::vector<QGraphicsPixmapItem*> poolDisparosJefe;
    std::vector<QGraphicsPixmapItem*> poolBombasJefe;
    std::vector<QGraphicsEllipseItem*> poolSombrasBombas;

    QGraphicsTextItem* hudTexto;
    QGraphicsPixmapItem* itemBarraVidaHeroe;
    QGraphicsPixmapItem* itemBarraVidaJefe;
    int ultimaVidaHeroeRender;
    int ultimaVidaJefeRender;
    int vidaPreviaHeroe;
    QWidget* overlayFinNivel;

    // Audio
    QMediaPlayer* musicaFondo;
    QAudioOutput* salidaMusicaFondo;
    QMediaPlayer* sfxDisparo;
    QAudioOutput* salidaSfxDisparo;
    QMediaPlayer* sfxGameOver;
    QAudioOutput* salidaSfxGameOver;
    QMediaPlayer* sfxSaltar;
    QAudioOutput* salidaSfxSaltar;
    QMediaPlayer* sfxRecibeDano;
    QAudioOutput* salidaSfxRecibeDano;
    size_t conteoDisparosPrevio;

    // Sprites Capitán América
    QPixmap pixQuieto;
    std::vector<QPixmap> framesCorrer;
    std::vector<QPixmap> framesSaltar;
    std::vector<QPixmap> framesGolpear;
    std::vector<QPixmap> framesDefensa;
    std::vector<QPixmap> framesEscudo;

    // Sprites Donald Trump (Fila 1: Idle, Fila 2: Caminar, Fila 3: Correr/Embestida, Fila 4: Saltar)
    std::vector<QPixmap> framesTrumpIdle;
    std::vector<QPixmap> framesTrumpCaminar;
    std::vector<QPixmap> framesTrumpCorrerEmbestida;
    std::vector<QPixmap> framesTrumpSaltar;

    QPixmap pixBalaJefe;
    QPixmap pixBombaJefe;

    int frameHeroeActual;
    int frameGolpeActual;
    int frameDefensaActual;
    int frameEscudoActual;
    int frameTrumpActual;
    int contadorTicks;
    bool mirandoDerecha;

    bool teclaIzquierda;
    bool teclaDerecha;
    bool teclaAbajo;

    void inicializarAudio();
    void construirEscenario();
    void cargarSprites();
    QPixmap limpiarFondoAjedrez(const QPixmap& original);
    QPixmap limpiarVerdeChroma(const QPixmap& original);
    QPixmap limpiarFondoPorEsquina(const QPixmap& original);
    QPixmap recortarBordesTransparentes(const QPixmap& pix, bool recortarAnchoExtra = true);
    void extraerSpritesPorBlob(const QPixmap& sheet, int yInicio, int yFin, int altoReferenciaMax, int altoObjetivo, std::vector<QPixmap>& destino);
    void actualizarAnimaciones(int direccion);
    void refrescarTextoHUD();
    void actualizarBarraVida(QGraphicsPixmapItem* target, int& cacheVida, int vidaActual, int vidaMax, const QString& prefijo, bool esJefe);
    void mostrarPantallaFinNivel(bool victoria);

public:
    explicit VentanaNivel2(QWidget* parent = nullptr);
    ~VentanaNivel2() override;

signals:
    void regresarAlMenu();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void actualizarFrame();
    void manejarFinNivel(bool victoria);
};

#endif // VENTANANIVEL2_H