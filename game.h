#ifndef GAME_H
#define GAME_H

#include <QGraphicsPixmapItem>
#include <QWidget>
#include <QGraphicsScene>
#include <vector>
#include <QTimer>
#include <QGraphicsTextItem>
#include <QFont>
#include <QMediaPlayer>
#include <QBrush>
#include <QImage>
#include <QGraphicsView>
#include <QDebug>
#include <QPixmap>
#include "player.h"
#include "platform.h"
#include "monster.h"
#include "score.h"
#include "item.h"
#include "button.h"

class Player;
class Platform;
class Monster;
class Score;
class Item;
class Button;

class Game: public QGraphicsView
{
public:
    Game();
    QGraphicsScene * scene;
    Player * player = nullptr;
    std::vector<Platform*> platform;
    std::vector<Monster*> monster;
    std::vector<Item*> item;
    void movePlatforms(double);
    void createPlatform();
    void endGame(int);
    void requestGameOver();
    void startplay();
    void decreaseHp();
    void discardMonster(Monster *);
    void discardItem(Item *);
    double heightAt(double sceneY) const;   // world height of a scene y (first platform = 0)
    Score * score = nullptr;
    double climbed = 0;          // how far the world has scrolled down so far
    double lastpy = 0;           // y of the newest platform the player can land on
    double topY = 0;             // y of the newest platform of any type
    bool lastWasBroken = false;  // the newest platform is a broken one
    int lastInterval[4] = {-1, -1, -1, -1};  // per special platform type: last interval it appeared in
    bool gameOver = false;
    QGraphicsPixmapItem * toolbar, * gameover;
    Button * playButton = nullptr;
    QGraphicsPixmapItem* hearts[3];
    int numHearts = 3;
private:
    int pickPlatformType(double height, bool brokenAllowed);
    void addPlatform(int type, double x, double y);
};

#endif // GAME_H
