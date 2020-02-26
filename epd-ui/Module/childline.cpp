#include "Module/childline.h"

//#define FIRST_STATION_CHE  171

ChildLine::ChildLine(int16_t ypos)
{
    this->setGeometry(0, ypos, 1200, 300);
    child_line_widget = new QWidget(this);
    child_line_widget->setObjectName(QStringLiteral("child_line"));
    child_line_widget->setGeometry(0, 0, 1200, 300);
    child_line_widget->setStyleSheet(QStringLiteral("#child_line{border-image: url(:/new/prefix1/line.bmp);}"));

    line_id = new QLabel(child_line_widget);
    line_id->setObjectName(QStringLiteral("line_id"));
    line_id->setGeometry(QRect(13, 47, 111, 63));
    line_id->setStyleSheet(QLatin1String("background-color: transparent;"));
    QFont font;
    font.setFamily(QStringLiteral("Monospace"));
    font.setPointSize(28);
    line_id->setFont(font);
    line_id->setAlignment(Qt::AlignCenter);

    info = new QLabel(child_line_widget);
    info->setObjectName(QStringLiteral("info"));
    info->setGeometry(QRect(20, 130, 130, 140));
    font.setPointSize(17);
    info->setFont(font);
    info->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    info->setWordWrap(true);
}

ChildLine::~ChildLine()
{
    line_list_clear();
    che_ico_list_clear();

    delete child_line_widget;
    child_line_widget = NULL;
}

void ChildLine::update_line_id(QString id)
{
    line_id->setText(id);
    line_id->update();
}

void ChildLine::update_info(QString timeS, QString timeE, QString price)
{
    info->setText("首班车时间\n" + timeS + "\n末班车时间\n" + timeE
                  + "\n票价 " + price + " 元");
    info->update();
}

void ChildLine::create_line(QString id, int8_t current_index, int8_t station_total, QList<QString> stat_name_list,
                            QString timeS="", QString timeE="", QString price="")   // index 当前站点的站点序号站点名需要加框
{
    StationNmae* stat_name;
    int16_t dist = 0;
    int xpos = 0;
    if(station_total > 25)
        station_total = 25;   // 最多25个站点
    if(station_total > 1) {
        dist = VAILD_DIST/(station_total-1);
    }
    if(!stat_name_list.isEmpty()) {
        line_list_clear();
        for(int8_t i = 0;i < station_total;i++)
        {
            stat_name = new StationNmae(FIRST_STATION + (i*dist), 25);

            line_list.append(stat_name);
            line_list.at(i)->setParent(child_line_widget);

            che_ico_list.append(new QLabel(child_line_widget));
            xpos = line_list.at(i)->pos().x();   //  获取当前站点名控件的X坐标
            che_ico_list.at(i)->setGeometry(QRect(xpos + 1, 0, 42, 20));
            che_ico_list.at(i)->setStyleSheet(QLatin1String("background-color: transparent;"));

            if(stat_name_list.length() > i) {
                line_list.at(i)->SetText(stat_name_list.at(i));
            }
            if(i == current_index)
            {
                line_list.at(i)->setSelect();
            }
        }
    }
    update_line_id(id);
    update_info(timeS.mid(0,5), timeS.mid(7,5), price);
}

void ChildLine::line_list_clear()
{
    int16_t l = 0;
    l = line_list.length();
    if(!line_list.isEmpty()) {
        for(int16_t i = 0; i < l; i++) {
            delete line_list.at(i);
        }
        line_list.clear();
    }
}

void ChildLine::che_ico_list_clear()
{
    int16_t l = 0;
    l = che_ico_list.length();
    if(!che_ico_list.isEmpty()) {
        for(int16_t i = 0; i < l; i++) {
            delete che_ico_list.at(i);
        }
        che_ico_list.clear();
    }
}

void ChildLine::update_status(QList<qint8> pos) {
    if(pos.isEmpty())
        return;
   // int16_t l = pos.length();
    for(uint16_t j = 0; j < line_list.length(); j++) {
        int16_t i;
        for(i = 0; i < pos.length(); i++) {
            if(line_list.length() <= pos.at(i)) {  // 当前车所在的站点序号大于line_list的长度 视为无效
                pos.removeAt(i);
                continue;
            }
            if(pos.at(i) == j) {
               break;
            }
        }
        if(i < pos.length()) {
            pos.removeAt(i);
           // qWarning("111");
            che_ico_list.at(j)->setStyleSheet(QLatin1String("border-image: url(:/new/prefix1/che.png);\n"
                    "background-color: transparent;"));
        } else {
            // qWarning("222");
            che_ico_list.at(j)->setStyleSheet(QLatin1String("background-color: transparent;"));
        }
    }
}
