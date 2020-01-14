#include "mainline.h"
#include <QDateTime>

MainLine::MainLine(int32_t ypos = 0)
{
    this->setGeometry(0,ypos,1200,101);
    line_widget = new QWidget(this);
    line_widget->setObjectName(QStringLiteral("line_widget"));
    line_widget->setGeometry(QRect(40, 0, 1108, 101));
    line_widget->setStyleSheet(QStringLiteral("#line_widget{border-image: url(:/new/prefix1/line01.bmp);}"));

    line_id = new QLabel(line_widget);
    line_id->setObjectName(QStringLiteral("lineId"));
    line_id->setGeometry(QRect(40, 10, 181, 91));
    QFont font;
    font.setPointSize(56);
    line_id->setFont(font);
    line_id->setLayoutDirection(Qt::RightToLeft);
    line_id->setAlignment(Qt::AlignCenter);

    over_stat = new QLabel(line_widget);
    over_stat->setObjectName(QStringLiteral("over_stat"));
    over_stat->setGeometry(QRect(910, 10, 181, 91));
    over_stat->setFont(font);
    over_stat->setAlignment(Qt::AlignCenter);

    line_info = new QLabel(line_widget);
    line_info->setObjectName(QStringLiteral("line_info"));
    line_info->setGeometry(QRect(250, 5, 641, 51));
    font.setPointSize(37);
    font.setBold(false);
    font.setItalic(false);
    font.setWeight(50);
    line_info->setFont(font);
    line_info->setAlignment(Qt::AlignCenter);

    beg_end_time_label = new QLabel(line_widget);
    beg_end_time_label->setObjectName(QStringLiteral("beg_end_time_label"));
    beg_end_time_label->setGeometry(QRect(250, 60, 181, 41));
    QFont font1;
    font1.setFamily(QStringLiteral("Monospace"));
    font1.setPointSize(20);
    beg_end_time_label->setFont(font1);
    beg_end_time_label->setText("首末班车时间:");

    beg_end_time_value = new QLabel(line_widget);
    beg_end_time_value->setObjectName(QStringLiteral("beg_end_time_value"));
    beg_end_time_value->setGeometry(QRect(440, 60, 221, 41));
    QFont font2;
    font2.setPointSize(20);
    beg_end_time_value->setFont(font2);

    price_label = new QLabel(line_widget);
    price_label->setObjectName(QStringLiteral("price_label"));
    price_label->setGeometry(QRect(690, 60, 81, 41));
    price_label->setFont(font2);
    price_label->setText("单价:");

    price_value = new QLabel(line_widget);
    price_value->setObjectName(QStringLiteral("price_value"));
    price_value->setGeometry(QRect(770, 60, 71, 41));
    price_value->setFont(font2);
}

void MainLine::showall()
{
    line_widget->show();
}
void MainLine::creat_line(QString id="", QString info="", QString timeSum="", QString timeWin="", QString price="")
{
    QString  time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm ddd");
    if(!id.isEmpty())
        line_id->setText(id);
    if(!info.isEmpty())
        line_info->setText("开往"+info);
    if((!timeSum.isEmpty()) && (!timeWin.isEmpty())) {
        if(time.mid(5,2).toInt() >= 5 && time.mid(5,2).toInt() < 10) {
            beg_end_time_value->setText(timeSum.remove(5,1));
        } else {
            beg_end_time_value->setText(timeWin.remove(5,1));
        }
    }
    if(!price.isEmpty())
        price_value->setText(price + "元");
}
void MainLine::updateLine_id(QString id)
{
    line_id->setText(id);
    line_id->update();
}
void MainLine::updateLine_info(QString info)
{
    line_info->setText("开往"+info);
    line_info->update();
}
void MainLine::updateOver_stat(QString count)
{
    over_stat->setText(count);
    over_stat->update();
}
void MainLine::updateBeg_end_time(QString Begtime, QString Endtime)
{
    beg_end_time_value->setText(Begtime + " - "+Endtime);
    beg_end_time_value->update();
}
void MainLine::updatePrice_value(QString price)
{
    price_value->setText(price + "元");
    price_value->update();
}
void MainLine::update_status(QString count)
{
    updateOver_stat(count);
 //   update
}
