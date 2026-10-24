#include "button.h"
Button::Button(QGraphicsItem *)
{
    QPixmap playerPixmap("./dataset/images/play_button.png");
    setPixmap(playerPixmap);
    setPos(130,259);
}
 
void Button::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    emit clicked();
}
