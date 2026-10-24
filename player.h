#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QImage>
#include <QGraphicsItem>
#include <QPixmap>
#include <QLabel>
#include <QList>
#include <vector>
#include "platform.h"
#include "score.h"
#include "item.h"
#include <QGraphicsScene>
#include <QKeyEvent>
#include "bullet.h"
#include <QPointF>
#include <QTimer>
#include <QtGlobal>
#include <QMediaPlayer>

#include <QDebug>

class Platform;
class Item;

class Player:public QObject, public QGraphicsPixmapItem{
    Q_OBJECT
public:
    Player(QGraphicsItem * parent=0);
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent *event);
    double vel;
    double getVel();
    void setVel();
    void Camera();
    void startBoost(int itemType);
    bool isFlying() const { return touchHat || touchJet; }
    double feet() const { return pos().y() + 80; }   // bottom of the 80 x 80 body
    bool leftface = true;
    bool middle = false;
    int yCounter = 0;
    bool touchSpring = false;
    bool touchTrampoline = false;
    bool touchHat = false;
    bool touchJet = false;
    int hcount = 1;
    int jcount = 1;
    int dx = 0;
    int best = 0;             // highest point reached so far = the score
    QTimer *timerx;
    QMediaPlayer *shoot, *jump;
private:
    void setSprite(const QString &path, bool alignBottom = true);
    void faceSprite();
    void handleCollisions();
    void endBoost();
    void animateFlight();
public slots:
    void moveX();
    void move();
    void highest();
};

#endif // PLAYER_H
