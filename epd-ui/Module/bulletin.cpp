#include "bulletin.h"

Bulletin::Bulletin(QFrame *parent, uint16_t ypos):
    QFrame(parent)
{
    this->setGeometry(QRect(0, ypos, 1200, 145));
    this->setFrameShape(QFrame::Box);
    this->setFrameShadow(QFrame::Raised);
    bulletin_widget = new QWidget(this);
    bulletin_widget->setObjectName(QStringLiteral("bulletin_widget"));
    bulletin_widget->setGeometry(QRect(40, 0, 1120, 145));
    bulletin_widget->setStyleSheet(QStringLiteral("#bulletin_widget{border-image: url(:/new/prefix1/bulletin.bmp);}"));
    text_label = new QLabel(bulletin_widget);
    title_label = new QLabel(bulletin_widget);
    text_label->setObjectName(QStringLiteral("text"));
    title_label->setObjectName(QStringLiteral("title"));
    title_label->setGeometry(QRect(5, 0, 1119, 30));
    text_label->setGeometry(QRect(7, 30, 1115, 100));
    QFont font;
    font.setFamily(QStringLiteral("Monospace"));
    font.setBold(true);
    font.setPointSize(22);
    title_label->setFont(font);
    title_label->setAlignment(Qt::AlignCenter);
    font.setPointSize(20);
    font.setBold(false);
    text_label->setFont(font);
    text_label->setWordWrap(true);
}

Bulletin::~Bulletin()
{
    delete bulletin_widget;
    delete title_label;
    delete text_label;
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
