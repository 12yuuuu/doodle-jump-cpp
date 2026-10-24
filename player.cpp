#include "player.h"

extern Game * game;

Player::Player(QGraphicsItem *parent): QGraphicsPixmapItem(parent)
{
    vel = 5;
    // set graphic
    setSprite("./dataset/images/doodleL.png");

    // the 8 ms tick that drives the player: physics, camera and score
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));
    connect(timer, &QTimer::timeout, this, &Player::Camera);
    connect(timer, SIGNAL(timeout()), this, SLOT(highest()));
    timer->start(8);

    // horizontal movement, started and stopped by the arrow keys
    timerx = new QTimer(this);
    connect(timerx, SIGNAL(timeout()), this, SLOT(moveX()));

    shoot = new QMediaPlayer(this);
    shoot->setMedia(QUrl::fromLocalFile("./dataset/sounds/shoot.wav"));
        // 設定音量
    shoot->setVolume(50);

    jump = new QMediaPlayer(this);
    jump->setMedia(QUrl::fromLocalFile("./dataset/sounds/jump.wav"));
        // 設定音量
    jump->setVolume(50);
}

// Keeps the 80 x 80 body in place when the sprite size changes: taller sprites
// (shooting pose, propeller hat) grow upwards; jetpack frames anchor at the top-left.
void Player::setSprite(const QString &path, bool alignBottom)
{
    QPixmap pix(path);
    setPixmap(pix);
    if (alignBottom)
        setOffset((80 - pix.width()) / 2.0, 80 - pix.height());
    else
        setOffset(0, 0);
}

void Player::faceSprite()
{
    setSprite(leftface ? "./dataset/images/doodleUL.png" : "./dataset/images/doodleUR.png");
}

void Player::keyPressEvent(QKeyEvent *event)
{
    // move the player left and right
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
    {
        leftface = (event->key() == Qt::Key_Left);
        dx = leftface ? -4 : 4;
        if (!isFlying())
            faceSprite();
        if (!timerx->isActive())
            timerx->start(5);
    }

    // shoot with the spacebar, aiming at the mouse cursor
    else if (event->key() == Qt::Key_Space && !event->isAutoRepeat())
    {
        QPointF muzzle = pos() + QPointF(40, 0);
        QPointF target = muzzle - QPointF(0, 100);   // straight up if there is no view
        if (!scene()->views().isEmpty())
        {
            // global cursor position -> view -> scene coordinates
            QGraphicsView *view = scene()->views().first();
            target = view->mapToScene(view->viewport()->mapFromGlobal(QCursor::pos()));
        }
        Bullet *bullet = new Bullet(nullptr, muzzle.x(), muzzle.y(), target.x(), target.y());
        scene()->addItem(bullet);
        if (!isFlying())
        {
            setSprite("./dataset/images/doodleS.png");
            QTimer::singleShot(250, this, [this] { if (!isFlying()) faceSprite(); });
        }
        shoot->play();
    }
}

void Player::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;
    if ((event->key() == Qt::Key_Left && dx < 0) || (event->key() == Qt::Key_Right && dx > 0))
    {
        dx = 0;
        timerx->stop();
    }
}

void Player::handleCollisions()
{
    const QList<QGraphicsItem*> hits = collidingItems();

    // platforms only count while falling, so the doodle passes through them on the way up
    if (vel < 0)
    {
        for (QGraphicsItem *g : hits)
        {
            Platform * platform = dynamic_cast<Platform *>(g);
            if (platform && platform->collide(this))
            {
                yCounter = 0;
                jump->play();
                break;
            }
        }
    }

    for (QGraphicsItem *g : hits)
    {
        Monster * monster = dynamic_cast<Monster *>(g);
        if (monster)
        {
            monster->collide(this);
            if (game->gameOver)
                return;
        }
    }

    for (QGraphicsItem *g : hits)
    {
        Item * item = dynamic_cast<Item *>(g);
        if (item)
            item->collide(this);
    }
}

void Player::startBoost(int itemType)
{
    touchSpring = (itemType == 1);
    touchTrampoline = (itemType == 2);
    touchHat = (itemType == 3);
    touchJet = (itemType == 4);
    yCounter = 0;
    if (!middle)
        vel = 5;   // rising again; move() sets the boosted speed
    if (!isFlying())
        faceSprite();
}

void Player::endBoost()
{
    bool wasFlying = isFlying();
    touchSpring = touchTrampoline = touchHat = touchJet = false;
    if (wasFlying)
        faceSprite();
}

void Player::animateFlight()
{
    if (touchHat)
    {
        if (yCounter % 2 == 0 && ++hcount > 4)
            hcount = 1;
        setSprite(QString("./dataset/images/doodleH/doodleH%1.png").arg(hcount));
    }
    else if (touchJet)
    {
        if (yCounter % 2 == 0 && ++jcount > 10)
            jcount = 1;
        setSprite(QString("./dataset/images/doodleJ/doodleJ%1.png").arg(jcount), false);
    }
}

void Player::move()
{
    if (game->gameOver)
        return;
    handleCollisions();
    if (game->gameOver)
        return;

    // Rising is constant-speed for a fixed number of ticks, so distance = ticks x speed.
    int speed = 5, ticks = 64;                                   //  320 px
    if (touchSpring)          { speed = 10; ticks = 96; }        //  960 px
    else if (touchTrampoline) { speed = 20; ticks = 96; }        // 1920 px
    else if (touchHat)        { speed = 40; ticks = 96; }        // 3840 px
    else if (touchJet)        { speed = 80; ticks = 96; }        // 7680 px

    yCounter++;
    if (vel > 0 || middle)   // still on the way up
    {
        if (middle)
        {
            // camera locked at half screen: the world moves instead of the doodle
            vel = 0;
            game -> movePlatforms(-speed);
        }
        else
        {
            vel = speed;
        }
        if (yCounter >= ticks)
        {
            // top of the jump: fall at a constant 5 px per tick
            vel = -5;
            middle = false;
            endBoost();
        }
    }
    animateFlight();
    setY(pos().y() - vel);

    //end game
    if (pos().y() > 960)
        game -> requestGameOver();
}

void Player::setVel()
{
    vel = 5;
    if (!isFlying())
        faceSprite();
}

double Player::getVel()
{
    return vel;
}

void Player::Camera()
{
    // past the half-screen line on the way up: lock the doodle there
    if (pos().y() < 480 && vel > 0)
    {
        middle = true;
        vel = 0;
    }
}

void Player::highest()
{
    if (game->gameOver)
        return;
    // score = highest point reached: distance scrolled + height above the start
    int height = int(game->climbed + (860 - pos().y()));
    if (height > best)
    {
        best = height;
        game -> score -> increase(best);
    }
}

void Player::moveX()
{
    setX(pos().x()+dx);
    // wraparound: leaving one side brings the doodle back on the other
    if (pos().x() + 40 < 0)
        setX(pos().x() + 540);
    else if (pos().x() + 40 > 540)
        setX(pos().x() - 540);
}
