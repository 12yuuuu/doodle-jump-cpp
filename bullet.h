#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>
#include <QObject>
#include "game.h"
#include <QMouseEvent>
#include "player.h"

class Player;
extern Game * game;

class Bullet: public QObject,public QGraphicsPixmapItem 
{
    Q_OBJECT
public:
    Bullet(QGraphicsItem * parent=0, double startX=0.0, double startY=0.0, double targetX =0.0, double targetY=0.0);
    double dx ,dy;
public slots:
    void move();
};

#endif // BULLET_H