#include "game.h"
#include <QGraphicsTextItem>
#include <QFont>
#include <QMediaPlayer>
#include <QBrush>
#include <QImage>
#include <QGraphicsView>
#include <QObject>
#include <QGraphicsPixmapItem>
#include <QString>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <QDebug>

namespace {

const int kScreenW = 540;
const int kScreenH = 960;
const int kPlatformW = 120;
const double kFirstPlatformY = 880;   // the first platform is height 0

// Special platforms from the spec: unlock score, "at least once every N",
// how close to the end of an interval one gets forced, and the normal chance (%).
struct SpecialRule { int type; int unlock; int every; int window; int chance; };
const SpecialRule kRules[] = {
    {1 /* broken      */,    0, 1000,  400, 8},
    {2 /* moving      */, 6000, 3000,  900, 6},
    {3 /* disappearing*/, 9000, 5000, 1500, 5},
};

}

Game::Game()
{
    //start interface
    scene = new QGraphicsScene();
    scene->setSceneRect(0,0,540,960);
    setScene(scene);

    QPixmap backgroundPixmap("./dataset/images/welcome.png");
    scene->setBackgroundBrush(backgroundPixmap);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(540,960);

    playButton = new Button();
    connect(playButton, &Button::clicked, this, &Game::startplay);
    scene->addItem(playButton);
}

void Game::startplay()
{
    if (playButton)
    {
        // we are inside the button's click handler, so delete it later
        scene -> removeItem(playButton);
        playButton -> deleteLater();
        playButton = nullptr;
    }
    scene -> clear();
    QPixmap backgroundPixmap("./dataset/images/background.png");
    scene->setBackgroundBrush(backgroundPixmap);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(540,960);

    toolbar = new QGraphicsPixmapItem();
    toolbar->setPixmap(QPixmap("./dataset/images/toolbar.png"));
    toolbar->setPos(0,-40);
    toolbar->setZValue(6);
    scene->addItem(toolbar);

    // reset the run
    platform.clear();
    monster.clear();
    item.clear();
    climbed = 0;
    gameOver = false;
    numHearts = 3;
    for (int &i : lastInterval)
        i = -1;

    // create the player
    player = new Player();
    player->setPos(200,860);
    player->setZValue(4);
    player->setFlag(QGraphicsItem::ItemIsFocusable);
    player->setFocus();

    // add the player to the scene
    scene->addItem(player);

    // hand-placed opening platforms (1 = broken), x in [0, 420] so they fit on screen;
    // like generated ones, real steps are at most 210 px apart
    addPlatform(0, 200, 880);
    addPlatform(0, std::rand() % 421, 700);
    addPlatform(1, std::rand() % 421, 580);
    addPlatform(0, std::rand() % 421, 500);
    addPlatform(0, std::rand() % 421, 340);
    addPlatform(0, std::rand() % 421, 210);
    addPlatform(1, std::rand() % 421, 120);
    addPlatform(0, std::rand() % 421, 30);
    lastpy = 30;
    topY = 30;
    lastWasBroken = false;
    lastInterval[1] = 0;   // the opening broken platforms cover the first 1000

    // keep the area just above the screen filled with platforms
    while (topY > -150)
        createPlatform();

    score = new Score();
    scene -> addItem(score);
    score -> setZValue(7);

    for (int i = 0; i < 3; ++i)
    {
        hearts[i] = new QGraphicsPixmapItem(QPixmap("./dataset/images/hp.png"));
        QPixmap newPixmap = hearts[i]->pixmap().scaled(40, 40);
        hearts[i]->setPixmap(newPixmap);
        hearts[i]->setPos(490 - i * 50, 15);
        scene->addItem(hearts[i]);
        hearts[i]->setZValue(8);
    }
}

void Game::addPlatform(int type, double x, double y)
{
    Platform *p = new Platform(type);
    p->setPos(x, y);
    platform.push_back(p);
    scene->addItem(p);
}

double Game::heightAt(double sceneY) const
{
    return climbed + (kFirstPlatformY - sceneY);
}

// Y < 0: the camera follows the player up, so the whole world moves down by -Y
void Game::movePlatforms(double Y)
{
    climbed -= Y;
    lastpy -= Y;
    topY -= Y;
    for (Platform *p : platform)
        p->setY(p->pos().y() - Y);
    for (Item *i : item)
        i->setY(i->pos().y() - Y);
    for (Monster *m : monster)
        m->setY(m->pos().y() - Y);

    // free everything that scrolled off the bottom ...
    for (std::size_t i = 0; i < platform.size(); )
    {
        if (platform[i]->pos().y() > kScreenH) { delete platform[i]; platform.erase(platform.begin() + i); }
        else ++i;
    }
    for (std::size_t i = 0; i < item.size(); )
    {
        if (item[i]->pos().y() > kScreenH) { delete item[i]; item.erase(item.begin() + i); }
        else ++i;
    }
    for (std::size_t i = 0; i < monster.size(); )
    {
        if (monster[i]->pos().y() > kScreenH) { delete monster[i]; monster.erase(monster.begin() + i); }
        else ++i;
    }

    // ... and spawn new platforms above the top edge
    while (topY > -150)
        createPlatform();
}

// Picks the type of the next platform at the given height.
// Each special type must show up at least once per interval (spec), so when an
// interval is about to end without one, that type is forced.
int Game::pickPlatformType(double h, bool brokenAllowed)
{
    int forced = 0;
    double soonest = 1e9;
    for (const SpecialRule &r : kRules)
    {
        if (h < r.unlock || (r.type == 1 && !brokenAllowed))
            continue;
        int interval = int((h - r.unlock) / r.every);
        if (interval == lastInterval[r.type])
            continue;   // already appeared in this interval
        double left = r.every - std::fmod(h - r.unlock, r.every);
        if (left <= r.window && left < soonest)
        {
            forced = r.type;
            soonest = left;
        }
    }
    if (forced)
        return forced;

    int roll = std::rand() % 100;
    for (const SpecialRule &r : kRules)
    {
        if (h < r.unlock || (r.type == 1 && !brokenAllowed))
            continue;
        if (roll < r.chance)
            return r.type;
        roll -= r.chance;
    }
    return 0;
}

void Game::createPlatform()
{
    // A broken platform can't be stood on, so the gap (60-210 px, always within
    // the 320 px jump) is measured from the last platform the player can land on.
    int minGap = lastWasBroken ? int(lastpy - topY) + 60 : 60;
    int gap = minGap + std::rand() % (210 - minGap + 1);
    int platformType = pickPlatformType(heightAt(lastpy - gap), !lastWasBroken);
    if (platformType == 1)
        gap = 60 + std::rand() % 81;   // keep the decoy below the next real step

    double y = lastpy - gap;
    double h = heightAt(y);
    addPlatform(platformType, std::rand() % (kScreenW - kPlatformW + 1), y);
    Platform *p = platform.back();

    topY = y;
    lastWasBroken = (platformType == 1);
    if (!lastWasBroken)
        lastpy = y;
    if (platformType != 0)
    {
        const SpecialRule &r = kRules[platformType - 1];
        lastInterval[platformType] = int((h - r.unlock) / r.every);
    }

    // monsters and items only sit on normal platforms, never both on one
    if (platformType != 0)
        return;

    //generate monster
    if (h >= 2000 && monster.size() < 2 && std::rand() % 100 < (h < 9000 ? 8 : 12))
    {
        int unlocked = h >= 12000 ? 4 : h >= 9000 ? 3 : h >= 6000 ? 2 : 1;
        Monster *m = new Monster(1 + std::rand() % unlocked);
        double w = m->pixmap().width();
        double mx = std::max(0.0, std::min(p->pos().x() + (kPlatformW - w) / 2, kScreenW - w));
        m->setPos(mx, y - m->pixmap().height() + 4);
        monster.push_back(m);
        scene->addItem(m);
        return;
    }

    //generate item: at most two on screen, unlocked by score as in the spec
    if (h >= 3000 && item.size() < 2 && std::rand() % 100 < 25)
    {
        static const int unlockAt[] = {0, 3000, 6000, 9000, 12000};  // spring, trampoline, hat, jetpack
        static const int weight[]   = {0,    5,    3,    2,     1};
        int total = 0;
        for (int t = 1; t <= 4; ++t)
            if (h >= unlockAt[t]) total += weight[t];
        int roll = std::rand() % total;
        int itemType = 1;
        for (int t = 1; t <= 4 && h >= unlockAt[t]; ++t)
        {
            if (roll < weight[t]) { itemType = t; break; }
            roll -= weight[t];
        }
        Item *i = new Item(itemType);
        i->setPos(p->pos().x() + (kPlatformW - i->pixmap().width()) / 2, y - i->pixmap().height() + 2);
        item.push_back(i);
        scene->addItem(i);
    }
}

void Game::discardMonster(Monster *m)
{
    monster.erase(std::remove(monster.begin(), monster.end(), m), monster.end());
    scene->removeItem(m);
    m->deleteLater();
}

void Game::discardItem(Item *i)
{
    item.erase(std::remove(item.begin(), item.end(), i), item.end());
    scene->removeItem(i);
    i->deleteLater();
}

// Called from inside player/monster callbacks, so the actual teardown waits
// for the next turn of the event loop.
void Game::requestGameOver()
{
    if (gameOver)
        return;
    gameOver = true;
    int finalScore = score->getfinalscore();
    QTimer::singleShot(0, this, [this, finalScore] { endGame(finalScore); });
}

void Game::endGame(int finalscore)
{
    // deletes every item still in the scene: player, platforms, monsters, items, bullets
    scene -> clear();
    player = nullptr;
    score = nullptr;
    platform.clear();
    monster.clear();
    item.clear();

    QPixmap backgroundPixmap("./dataset/images/welcome.png");
    scene->setBackgroundBrush(backgroundPixmap);

    gameover = new QGraphicsPixmapItem();
    gameover->setPixmap(QPixmap("./dataset/images/end.png"));
    gameover->setPos(130,259);
    gameover->setZValue(6);
    scene->addItem(gameover);

    QGraphicsTextItem* scoreText = new QGraphicsTextItem();  // Create a text item
    scoreText->setPlainText(QString("Score:  ") + QString::number(finalscore));  // Set the text
    scoreText->setDefaultTextColor(Qt::black);
    scoreText->setFont(QFont("Comic Sans MS",12));
    scoreText->setPos(130,480);
    scene->addItem(scoreText);  // Add the text item to the scene
    scoreText->setZValue(7);

    // play again
    playButton = new Button();
    playButton->setPos(130, 560);
    playButton->setZValue(7);
    connect(playButton, &Button::clicked, this, &Game::startplay);
    scene->addItem(playButton);
}

void Game::decreaseHp()
{
    if (numHearts <= 0)
        return;
    numHearts--;
    scene->removeItem(hearts[numHearts]);
    delete hearts[numHearts];
    hearts[numHearts] = nullptr;
    if (numHearts == 0)
        requestGameOver();
}
