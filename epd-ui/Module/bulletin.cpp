#include "bulletin.h"

Bulletin::Bulletin(QFrame *parent):
    QFrame(parent)
{
    //this->setGeometry(QRect(0, 1300, 1200, 300));
    this->setFrameShape(QFrame::Box);
    this->setFrameShadow(QFrame::Raised);
    bulletin_widget = new QWidget(this);
    bulletin_widget->setObjectName(QStringLiteral("bulletin_widget"));
    bulletin_widget->setGeometry(QRect(0, 0, 1200, 300));
    text_label = new QLabel(bulletin_widget);
    title_label = new QLabel(bulletin_widget);
    text_label->setObjectName(QStringLiteral("text"));
    title_label->setObjectName(QStringLiteral("title"));
    title_label->setGeometry(QRect(0, 0, 1200, 50));
    text_label->setGeometry(QRect(0, 50, 1200, 250));
    QFont font;
    font.setFamily(QStringLiteral("Monospace"));
    font.setBold(true);
    font.setPointSize(30);
    title_label->setFont(font);
    title_label->setAlignment(Qt::AlignCenter);
    font.setPointSize(22);
    font.setBold(false);
    text_label->setFont(font);
}

void Bulletin::update_text(QString title, QString text) {
    if(!title.isEmpty()) {
        title_label->setText(title);
        title_label->update();
    }
    if(!text.isEmpty()) {
        text_label->setText(text);
        text_label->update();
    }
}
