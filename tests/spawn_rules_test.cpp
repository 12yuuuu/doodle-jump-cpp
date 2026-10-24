// Checks the level generator against the course spec, without a window or assets.
//
//   mkdir build-tests && cd build-tests
//   qmake ../tests/tests.pro && make
//   QT_QPA_PLATFORM=offscreen ./spawn_rules_test
//
// For each seed it scrolls the real game world up by 60,000 px and checks that
//   - consecutive platforms are at least 60 px apart,
//   - the gap between platforms the player can stand on never exceeds 210 px
//     (a normal jump is 320 px), so every level is climbable,
//   - broken / moving / disappearing platforms appear at least once per
//     1000 / 3000 / 5000 px once unlocked (0 / 6000 / 9000),
//   - no more than two items and two monsters exist at the same time.
#include "game.h"
#include <QApplication>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>

Game * game;

static void quiet(QtMsgType, const QMessageLogContext &, const QString &) {}

static int checkSeed(unsigned seed)
{
    std::srand(seed);
    game->startplay();

    std::map<long, int> seen;   // height -> platform type
    std::size_t maxItems = 0, maxMonsters = 0;
    for (int step = 0; step < 6000; ++step)
    {
        game->movePlatforms(-10);
        for (Platform *p : game->platform)
            seen[std::lround(game->heightAt(p->pos().y()))] = p->type;
        maxItems = std::max(maxItems, game->item.size());
        maxMonsters = std::max(maxMonsters, game->monster.size());
    }

    int failures = 0;
    long prev = 0, prevSolid = 0, minGap = 1000000, maxSolidGap = 0;
    bool first = true;
    for (const auto &kv : seen)
    {
        if (!first)
            minGap = std::min(minGap, kv.first - prev);
        if (kv.second != 1)   // not broken: the player can stand on it
        {
            if (!first)
                maxSolidGap = std::max(maxSolidGap, kv.first - prevSolid);
            prevSolid = kv.first;
        }
        prev = kv.first;
        first = false;
    }
    if (minGap < 60)        { std::printf("seed %u: platforms only %ld px apart\n", seed, minGap); ++failures; }
    if (maxSolidGap > 210)  { std::printf("seed %u: unreachable gap of %ld px\n", seed, maxSolidGap); ++failures; }
    if (maxItems > 2)       { std::printf("seed %u: %zu items at once\n", seed, maxItems); ++failures; }
    if (maxMonsters > 2)    { std::printf("seed %u: %zu monsters at once\n", seed, maxMonsters); ++failures; }

    struct Rule { int type, unlock, every; const char *name; };
    const Rule rules[] = {{1, 0, 1000, "broken"}, {2, 6000, 3000, "moving"}, {3, 9000, 5000, "disappearing"}};
    long top = seen.rbegin()->first - 300;   // ignore the not-yet-finished last interval
    for (const Rule &r : rules)
    {
        for (long from = r.unlock; from + r.every <= top; from += r.every)
        {
            bool found = false;
            for (auto it = seen.lower_bound(from); it != seen.end() && it->first < from + r.every; ++it)
                if (it->second == r.type) { found = true; break; }
            if (!found)
            {
                std::printf("seed %u: no %s platform in %ld-%ld\n", seed, r.name, from, from + r.every);
                ++failures;
            }
        }
    }
    return failures;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qInstallMessageHandler(quiet);   // no assets or audio backend needed
    game = new Game();

    int failures = 0;
    const unsigned seeds = 50;
    for (unsigned seed = 1; seed <= seeds; ++seed)
        failures += checkSeed(seed);

    std::printf("%u seeds x 60000 px: %s\n", seeds, failures ? "FAILED" : "all spawn rules hold");
    return failures ? 1 : 0;
}
