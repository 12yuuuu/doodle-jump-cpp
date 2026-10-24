#include "score.h"
#include <QFont>
#include <QDebug>

Score::Score(QGraphicsItem *parent): QGraphicsTextItem(parent)
{
    grade = 0;
    // draw the text
    setPlainText(QString("Score: ") + QString::number(grade)); // Score: 0
    setDefaultTextColor(Qt::black);
    setFont(QFont("Comic Sans MS",12));
}
 
void Score::increase(int highest)
{
    grade = highest;
    setPlainText(QString("Score: ") + QString::number(grade));
    finalscore = grade;
}
int Score::getScore(){
    return grade;
}

int Score::getfinalscore(){
    return finalscore;
}