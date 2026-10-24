#include "bullet.h"
#include <QTimer>
#include <QGraphicsScene>
#include <QList>
#include <qmath.h>
#include <QDebug>

Bullet::Bullet(QGraphicsItem *parent, double startX, double startY, double targetX, double targetY): QObject(), QGraphicsPixmapItem(parent)
{
    // draw graphics, centred on the muzzle
    setPixmap(QPixmap("./dataset/images/bullet.png"));
    setPos(startX - 10, startY - 10);
    setZValue(5);
    double dx = targetX - startX;
    double dy = targetY - startY;

    // 根据移動向量计算移動單位向量
    double length = std::sqrt(dx * dx + dy * dy);
    if (length < 1)
    {
        // cursor right on the doodle: shoot straight up
        dx = 0;
        dy = -1;
        length = 1;
    }
    double unitX = dx / length;
    double unitY = dy / length;

    // 設置子彈的移動速度 (20 px every 20 ms = 1000 px/s)
    double speed = 20;
    dx = unitX * speed;
    dy = unitY * speed;

    // 保存移動向量
    this->dx = dx;
    this->dy = dy;

    QTimer * timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(move()));
    timer->start(20);
}

void Bullet::move()
{
    if (!scene())
        return;
    setPos(x()+dx, y()+dy);

    // one shot, one kill
    QList<QGraphicsItem*> colliding_bullet = collidingItems();
    for(int i = 0, n = colliding_bullet.size();i<n; ++i)
    {
        Monster * monster = dynamic_cast<Monster *>(colliding_bullet[i]);
        if(monster && !monster->dead)
        {
            monster->hit(game);
            scene()->removeItem(this);
            deleteLater();
            return;
        }
    }

    // left the screen
    if (!scene()->sceneRect().adjusted(-40, -40, 40, 40).contains(pos()))
    {
        scene()->removeItem(this);
        deleteLater();
    }
}
