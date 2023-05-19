#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
#include <QFocusEvent>

class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = 0);

protected:
    virtual void focusInEvent(QFocusEvent *event);

signals:
    void focus();
};

#endif // MYLINEEDIT_H
