#include "item.h"
#include <QTimer>
#include <QGraphicsScene>
#include <QList>
#include <stdlib.h>

extern Game * game;

enum items {
    noitem,
    spring,
    trampoline,
    propellerhat,
    jetpack,
};

Item::Item(int t): QObject(), QGraphicsPixmapItem(nullptr)
{
    type = t;
    switch (t)
    {
    case 0:
        break;
    case 1:
        setPixmap(QPixmap("./dataset/images/item-spring1.png"));
        break;
    case 2:
        setPixmap(QPixmap("./dataset/images/item-trampoline1.png"));
        break;
    case 3:
        setPixmap(QPixmap("./dataset/images/item-propeller-hat.png"));
        break;
    case 4:
        setPixmap(QPixmap("./dataset/images/item-jetpack.png"));
        break;
    default:
        break;
    }
    setZValue(2);
}

void Item::collide(Player * player)
{
    switch (type)
    {
        case spring:
        case trampoline:
            // only works when the doodle lands on it from above
            if (player->vel < 0 && player->feet() <= pos().y() + 15)
            {
                if (type == spring && !used)
                {
                    // the extended spring is taller; keep its base on the platform
                    QPixmap extended("./dataset/images/item-spring2.png");
                    setY(pos().y() - (extended.height() - pixmap().height()));
                    setPixmap(extended);
                }
                else if (type == trampoline)
                {
                    frame = 1;
                    QTimer::singleShot(0, this, &Item::bounce);
                }
                used = true;
                player->startBoost(type);
            }
            break;
        case propellerhat:
        case jetpack:
            // picked up on any touch, and used up
            if (!player->isFlying())
            {
                player->startBoost(type);
                game->discardItem(this);
            }
            break;
    }
}

void Item::bounce()
{
    // trampoline animation: frame 2, 3, back to 1
    frame = frame % 3 + 1;
    setPixmap(QPixmap(QString("./dataset/images/item-trampoline%1.png").arg(frame)));
    if (frame != 1)
        QTimer::singleShot(70, this, &Item::bounce);
}

int Item::getType()
{
    return type;
}
