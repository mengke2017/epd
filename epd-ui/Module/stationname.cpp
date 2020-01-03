#include "Module/stationname.h"

StationNmae::StationNmae(int16_t xpos,int16_t ypos)
{
    this->setGeometry(xpos,ypos,52,288);
    stat_name_widget = new QWidget(this);
    stat_name_widget->setGeometry(0,0,52,288);
    stat_name_widget->setObjectName("stat_name_widget");

    drop_label = new QLabel(stat_name_widget);
    drop_label->setObjectName(QStringLiteral("drop_label"));
    drop_label->setGeometry(QRect(14, 0, 25, 25));
    drop_label->setStyleSheet(QLatin1String("#drop_label{border-image: url(:/new/prefix1/d.png);\n"
                                            "background-color: transparent;}"));

    name_label = new QLabel(stat_name_widget);
    name_label->setObjectName("name_label");
    name_label->setGeometry(0,25,52,253);
    name_label->setStyleSheet("background-color: transparent;");
    QFont font;
    font.setFamily(QString::fromUtf8("\346\226\207\346\263\211\351\251\277\345\276\256\347\261\263\351\273\221"));
    font.setPointSize(22);
    name_label->setFont(font);
    name_label->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
    name_label->setWordWrap(true);
}

void StationNmae::setText(QString text)
{
    QFont font;
    if(text.length() <= 7) {
        font.setPointSize(22);
    } else if (text.length() > 7 && text.length() <= 14) {
       font.setPointSize(19);
    } else {
        font.setPointSize(18);
        text = text.mid(0,16);
    }
    name_label->setFont(font);
//    <html><head/><body><p style='line-height:80%'>浙江杭州图软科技杭州图软科技</p></body></html>
    name_label->setText("<html><head/><body><p style='line-height:80%'>" + text + "</p></body></html>");  // 使用HTML语言设置字符串的行高
}

void StationNmae::setSelect()
{
//    name_label->setStyleSheet("border-image: url(:/new/prefix1/select.png);");
    //name_label->update();
}
