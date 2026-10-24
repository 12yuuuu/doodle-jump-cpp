#include "monster.h"

Monster::Monster(int t)
{
    switch (t)
    {
    case 0:
        break;
    case 1:
        setPixmap(QPixmap("./dataset/images/monster1.png"));
        break;
    case 2:
        setPixmap(QPixmap("./dataset/images/monster2.png"));
        break;
    case 3:
        setPixmap(QPixmap("./dataset/images/monster3.png"));
        break;
    case 4:
        setPixmap(QPixmap("./dataset/images/monster4.png"));
        break;
    default:
        break;
    }
    setZValue(3);

    jumponmonster = new QMediaPlayer(this);
    jumponmonster->setMedia(QUrl::fromLocalFile("./dataset/sounds/jumponmonster.wav"));
        // 設定音量
    jumponmonster->setVolume(50);
}

void Monster::collide(Player * player)
{
    // a flying doodle (propeller hat / jetpack) can't be hurt
    if (dead || player->isFlying())
        return;

    // coming down onto the top part of the monster: stomp it
    if (player->getVel() < 0 && player->feet() <= pos().y() + pixmap().height() * 0.4)
    {
        dead = true;
        jumponmonster->play();
        QTimer* timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(move()));
        timer->start(50);
        player->setVel();
        player->yCounter = 0;
    }
    // touching its sides or bottom: lose one heart, once per monster
    else if (!hasCollidedWithPlayer)
    {
        hasCollidedWithPlayer = true;
        game->decreaseHp();
    }
}

void Monster::hit(Game * game)
{
    if (dead)
        return;
    dead = true;
    game->discardMonster(this);
}

void Monster::move()
{
    setY(pos().y() + 10);
}
