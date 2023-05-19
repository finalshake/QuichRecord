#include "mylineedit.h"

MyLineEdit::MyLineEdit(QWidget *parent) : QLineEdit(parent)
{

}

void MyLineEdit::focusInEvent(QFocusEvent *event)
{
    if(event->gotFocus())
        emit focus();
    QLineEdit::focusInEvent(event);
}
