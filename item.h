#ifndef ITEM_H
#define ITEM_H

#include "player.h"
#include "game.h"
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QGraphicsItem>

class Game;
class Player;

class Item: public QObject,public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    Item(int);
    void collide(Player*);
    int type;
    int getType();
    bool used = false;
    int frame = 1;
public slots:
    void bounce();
};

#endif
