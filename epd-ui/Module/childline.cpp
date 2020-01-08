#include "Module/childline.h"

#define FIRST_STATION_CHE  171

ChildLine::ChildLine(int16_t ypos)
{
    this->setGeometry(0, ypos, 1200, 300);
    child_line_widget = new QWidget(this);
    child_line_widget->setObjectName(QStringLiteral("child_line"));
    child_line_widget->setGeometry(0, 0, 1200, 300);
    child_line_widget->setStyleSheet(QStringLiteral("#child_line{border-image: url(:/new/prefix1/line.bmp);}"));

    line_id = new QLabel(child_line_widget);
//    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    sizePolicy.setHorizontalStretch(0);
//    sizePolicy.setVerticalStretch(0);
//    sizePolicy.setHeightForWidth(nameLabel->sizePolicy().hasHeightForWidth());

    line_id->setObjectName(QStringLiteral("line_id"));
    line_id->setGeometry(QRect(33, 47, 91, 63));
    line_id->setStyleSheet(QLatin1String("background-color: transparent;"));
    QFont font;
    font.setFamily(QStringLiteral("Monospace"));
    font.setPointSize(30);
    line_id->setFont(font);
    line_id->setAlignment(Qt::AlignCenter);

    info = new QLabel(child_line_widget);
    info->setObjectName(QStringLiteral("info"));
    info->setGeometry(QRect(20, 115, 130, 140));
    font.setPointSize(17);
    info->setFont(font);
    info->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    info->setWordWrap(true);
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
    if(station_total > 26)
        station_total = 26;
    if(station_total > 1) {
        dist = VAILD_DIST/(station_total-1);
    }
    if(!stat_name_list.isEmpty()) {
        line_list_clear();
        for(int8_t i = 0;i < station_total || i < 16;i++)   // 最多16个站点
        {
            stat_name = new StationNmae(FIRST_STATION + (i*dist), 25);
            line_list.append(stat_name);
            line_list.at(i)->setParent(child_line_widget);
            if(stat_name_list.length() > i) {
                line_list.at(i)->setText(stat_name_list.at(i));
            }
            if(i+1 == current_index)
            {
                line_list.at(i)->setSelect();
            }
        }
    }
    update_line_id(id);
    update_info(timeS.mid(0,5), timeS.mid(7,5), price);
}

void ChildLine::create_cheico(QList<qint8> che_index)
{
    if(!che_ico_list.isEmpty()) {
        che_ico_list_clear();
    }
    if(che_index.isEmpty())
        return;
    int16_t l = che_index.length();
    int xpos = 0;

    for(int16_t i = 0; i < l; i++) {
        if(line_list.length() <= che_index.at(i))   // 当前车所在的站点序号大于line_list的长度 视为无效
            continue;
        che_ico_list.append(new QLabel(child_line_widget));
        xpos = line_list.at(che_index.at(i))->pos().x();   //  获取当前站点名控件的X坐标
        che_ico_list.at(i)->setGeometry(QRect(xpos + 6, 0, 42, 20));
        che_ico_list.at(i)->setStyleSheet(QLatin1String("border-image: url(:/new/prefix1/che.png);\n"
                "background-color: transparent;"));
    }
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
    create_cheico(pos);
}
