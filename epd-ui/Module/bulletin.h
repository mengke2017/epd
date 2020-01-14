#ifndef _BULLETIN_H
#define _BULLETIN_H

#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include "customize.h"

class Bulletin:public QFrame
{
    Q_OBJECT

public:
    explicit Bulletin(QFrame* parent = Q_NULLPTR, uint16_t ypos = 0);

    QWidget *bulletin_widget;
    QLabel *title_label;
    QLabel *text_label;

    void update_text(QString title, QString text);
};

#endif // BULLETIN_H
