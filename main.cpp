#include "game.h"
#include <QApplication>
#include <cstdlib>
#include <ctime>

Game * game;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::srand(static_cast<unsigned>(std::time(nullptr)));  // a different level every run
    game = new Game();
    game->show();
    return a.exec();
}
