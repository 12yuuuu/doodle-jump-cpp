#include "platform.h"

enum platformstypes {
    normal = 0,
    broken = 1,
    moving = 2,
    disappear = 3,
};

Platform::Platform(int t)
{
    type = t;
    switch (t)
    {
    case 0:
        setPixmap(QPixmap("./dataset/images/stair-basic.png"));
        break;
    case 1:
        setPixmap(QPixmap("./dataset/images/stair-broken1.png"));
        break;
    case 2:
        setPixmap(QPixmap("./dataset/images/stair-blue.png"));
        break;
    case 3:
        setPixmap(QPixmap("./dataset/images/stair-disapear.png"));
        break;
    default:
        setPixmap(QPixmap("./dataset/images/stair-basic.png"));
        break;
    }
    setZValue(1);

    if (type == moving)
    {
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Platform::move);
        timer->start(50);
    }

    if (type == broken)
    {
        platformbreak = new QMediaPlayer(this);
        platformbreak->setMedia(QUrl::fromLocalFile("./dataset/sounds/break.wav"));
        // 設定音量
        platformbreak->setVolume(50);
    }
    else
    {
        platformbreak = nullptr;
    }
}

bool Platform::collide(Player * player)
{
    // one-way platform: only a falling doodle whose feet reach the top edge lands
    if (used || player->vel >= 0 || player->feet() > pos().y() + 15)
        return false;

    switch (type)
    {
        case broken:
        {
            // it cracks and drops away; the doodle falls straight through
            used = true;
            platformbreak->play();
            QTimer* timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, &Platform::down);
            timer->start(30);
            return false;
        }
        case disappear:
        {
            // good for exactly one jump, then it fades out
            used = true;
            player->setVel();
            QTimer* timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, &Platform::fade);
            timer->start(30);
            return true;
        }
        default:   // normal, moving
            player->setVel();
            return true;
    }
}

int Platform::getType()
{
    return type;
}

void Platform::down()
{
    // play the break animation, then keep falling
    if (frame < 4)
    {
        ++frame;
        setPixmap(QPixmap(QString("./dataset/images/stair-broken%1.png").arg(frame)));
    }
    setY(pos().y() + 10);
    if (pos().y() > 960)
        qobject_cast<QTimer *>(sender())->stop();
}

void Platform::fade()
{
    setOpacity(opacity() - 0.2);
    if (opacity() <= 0.05)
    {
        setVisible(false);
        qobject_cast<QTimer *>(sender())->stop();
    }
}

void Platform::move()
{
    if (getType() == moving)
    {
        if (pos().x() >= 420)
            v = -5;
        else if (pos().x() <= 0)
            v = 5;
        setX(pos().x() + v);
    }
}
