#ifndef MONSTER_H
#define MONSTER_H

#include "player.h"
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QGraphicsItem>
#include <QTimer>
#include <QGraphicsScene>
#include <QList>
#include <stdlib.h>
#include "game.h"
#include <QMediaPlayer>

class Game;
class Player;
extern Game * game;

class Monster: public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    Monster(int);
    void collide(Player *);
    void hit(Game*);
    bool hasCollidedWithPlayer = false;
    bool dead = false;
    QMediaPlayer  *jumponmonster;
public slots:
    void move();

};

#endif
