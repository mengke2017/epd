#include "Module/stationname.h"

StationNmae::StationNmae(int16_t xpos,int16_t ypos)
{
    this->setGeometry(xpos,ypos,40,283);
    stat_name_widget = new QWidget(this);
    stat_name_widget->setGeometry(0,0,40,283);
    stat_name_widget->setObjectName("stat_name_widget");

    drop_label = new QLabel(stat_name_widget);
    drop_label->setObjectName(QStringLiteral("drop_label"));
    drop_label->setGeometry(QRect(10, 2, 20, 20));
    drop_label->setStyleSheet(QLatin1String("#drop_label{border-image: url(:/new/prefix1/d.png);\n"
                                            "background-color: transparent;}"));

    name_widget = new QWidget(stat_name_widget);
    name_widget->setGeometry(0,25,40,245);
    name_widget->setObjectName("name_widget");
    name_widget->setStyleSheet("background-color: transparent;");

    name_label = new QLabel(name_widget);
    name_label2 = new QLabel(name_widget);
    name_label->setObjectName("name_label");
    name_label2->setObjectName("name_label2");
    name_label->setGeometry(0,0,40,245);
    name_label2->setGeometry(20,0,20,245);
    name_label->setStyleSheet("background-color: transparent;");
    name_label2->setStyleSheet("background-color: transparent;");
    QFont font;
 //   font.setFamily(QString::fromUtf8("\346\226\207\346\263\211\351\251\277\345\276\256\347\261\263\351\273\221"));
    font.setFamily(QStringLiteral("Monospace"));
    font.setPointSize(22);
    name_label->setFont(font);
    name_label2->setFont(font);
    name_label2->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    name_label->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    name_label->setWordWrap(true);
    name_label2->setWordWrap(true);
}

StationNmae::~StationNmae()
{
    delete stat_name_widget;
    delete drop_label;
    delete name_label;
    delete name_label2;
}

void StationNmae::SetText(QString text)
{
    QFont font;
    QString text2;
    if(text.length() <= 7) {
        font.setPointSize(22);
        name_label2->hide();
    } else if (text.length() > 7 && text.length() <= 9) {
       font.setPointSize(16);
       name_label->setGeometry(10,0,25,518);
    } else if (text.length() > 9 && text.length() <= 11) {
        font.setPointSize(15);
        name_label->setGeometry(10,0,20,518);
    } else {
        if(text.length() > 20)
            text = text.mid(0,20);
        font.setPointSize(15);
        text2 = text.mid(10);
        if(text2 != " ︶") {
            text = text.mid(0,10);
            name_label->setGeometry(0,0,20,515);
            name_label2->setFont(font);
            name_label2->setGeometry(20,0,20,515);
            name_label2->setText("<html><head/><body><p style='line-height:80%'>" + text2 + "</p></body></html>");
        }
    }
    name_label->setFont(font);
//    <html><head/><body><p style='line-height:80%'>浙江杭州图软科技杭州图软科技</p></body></html>
    name_label->setText("<html><head/><body><p style='line-height:80%'>" + text + "</p></body></html>");  // 使用HTML语言设置字符串的行高
}

void StationNmae::setSelect()
{
    name_widget->setStyleSheet("#name_widget{border-image: url(:/new/prefix1/select.bmp);}");
    name_widget->update();
}
