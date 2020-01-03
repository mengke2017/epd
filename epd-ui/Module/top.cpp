#include "Module/top.h"
#include <QDateTime>
#include <QTimer>

TopWidget::TopWidget()
{
    QFont font;
    font.setFamily(QStringLiteral("Monospace"));

    this->setGeometry(0,0,1200,234);
    top_widget = new QWidget(this);
    top_widget->setObjectName("top_widget");
    top_widget->setStyleSheet(QStringLiteral("#top_widget{background-image: url(:/new/prefix1/line00.bmp);}"));
    top_widget->setGeometry(0,0,1200,234);

    stat_name = new QLabel(top_widget);
    font.setPointSize(39);
    font.setBold(true);
    font.setWeight(75);
    stat_name->setFont(font);
    stat_name->setAlignment(Qt::AlignCenter);
    stat_name->setText("杭州东站");
    stat_name->setGeometry(0,40,361,101);

    weath_ico = new QLabel(top_widget);
    weath_ico->setStyleSheet("image: url(:/new/prefix1/duoyun.bmp);");
    weath_ico->setGeometry(450,45,101,71);

    time_label = new QLabel(top_widget);
    font.setFamily(QStringLiteral("Monospace"));
    font.setPointSize(52);
    font.setBold(false);
    font.setWeight(50);
    time_label->setFont(font);
    time_label->setAlignment(Qt::AlignCenter);
    time_label->setText("12:00");
    time_label->setGeometry(950,40,231,91);

    weath_text = new QLabel(top_widget);
    weath_text->setFont(QFont("Sans Serif", 18, 50));
    weath_text->setAlignment(Qt::AlignCenter);
    weath_text->setText("多云\n10~12℃");
    weath_text->setGeometry(550,35,141,91);

    date_text = new QLabel(top_widget);
    date_text->setFont(QFont("Sans Serif", 18, 50));
    date_text->setAlignment(Qt::AlignCenter);
    date_text->setText("2019年12月25日\n星期三");
    date_text->setGeometry(720,40,211,91);

    QTimer * timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(TimeUpdate()));//将定时器与TimeUpdate函数绑定
    timer->start(100);//一秒计时一次

    title_widget = new QWidget(top_widget);
    title_widget->setGeometry(QRect(0, 154, 1200, 80));
    title_widget->setStyleSheet(QStringLiteral("background-color: rgb(7, 7, 7);"));

    lineId_name = new QLabel(title_widget);
    lineId_name->setObjectName(QStringLiteral("lineId_name"));
    lineId_name->setEnabled(false);
    lineId_name->setGeometry(QRect(60, 0, 181, 80));
    QFont font1;
    font1.setFamily(QStringLiteral("Monospace"));
    font1.setPointSize(36);
    lineId_name->setFont(font1);
    lineId_name->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
    lineId_name->setAlignment(Qt::AlignCenter);
    lineId_name->setText("线路");

    line_info_name = new QLabel(title_widget);
    line_info_name->setObjectName(QStringLiteral("line_info_name"));
    line_info_name->setEnabled(false);
    line_info_name->setGeometry(QRect(460, 0, 281, 80));
    line_info_name->setFont(font1);
    line_info_name->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
    line_info_name->setAlignment(Qt::AlignCenter);
    line_info_name->setText("线路信息");

    over_stat_name = new QLabel(title_widget);
    over_stat_name->setObjectName(QStringLiteral("over_stat_name"));
    over_stat_name->setEnabled(false);
    over_stat_name->setGeometry(QRect(900, 0, 271, 80));
    over_stat_name->setFont(font1);
    over_stat_name->setStyleSheet(QStringLiteral("color: rgb(255, 255, 255);"));
    over_stat_name->setAlignment(Qt::AlignCenter);
    over_stat_name->setText("剩余站点数");

    weath_list_ico<<"image: url(:/new/prefix1/baoxue.bmp);"<<"image: url(:/new/prefix1/baoyu.bmp);"
                 <<"image: url(:/new/prefix1/dabaoxue.bmp);"<<"image: url(:/new/prefix1/dabaoyu.bmp);"
                 <<"image: url(:/new/prefix1/daxie.bmp);"<<"image: url(:/new/prefix1/dayu.bmp);"
                 <<"image: url(:/new/prefix1/duoyun.bmp);"<<"image: url(:/new/prefix1/leizhengyu.bmp);"
                 <<"image: url(:/new/prefix1/mai.bmp);"<<"image: url(:/new/prefix1/qing.bmp);"
                 <<"image: url(:/new/prefix1/shachengbao.bmp);"<<"image: url(:/new/prefix1/taifeng.bmp);"
                 <<"image: url(:/new/prefix1/teidabaoyu.bmp);"<<"image: url(:/new/prefix1/wu.bmp);"
                 <<"image: url(:/new/prefix1/xiaoxie.bmp);"<<"image: url(:/new/prefix1/xiaoyu.bmp);"
                 <<"image: url(:/new/prefix1/ying.bmp);"<<"image: url(:/new/prefix1/yujiaxie.bmp);"
                 <<"image: url(:/new/prefix1/zhengyu.bmp);"<<"image: url(:/new/prefix1/zhongxie.bmp);"
                 <<"image: url(:/new/prefix1/zhongyu.bmp);";
    weath_list_text<<"暴雪"<<"暴雨"<<"大暴雪"<<"大暴雨"<<"大雪"<<"大雨"<<"多云"<<"雷阵雨"<<"霾"<<"晴"
                  <<"沙尘暴"<<"台风"<<"特大暴雨"<<"雾"<<"小雪"<<"小雨"<<"阴"<<"雨夹雪"<<"阵雨"<<"中雪"<<"中雨";
}

void TopWidget::showall()
{
    top_widget->show();
//    title_widget->show();
}
void TopWidget::updateStat_name(QString station)
{
    stat_name->setText(station);
    stat_name->update();
}

void TopWidget::updateDate(QString date, QString week)
{
    date_text->setText(date + "\n" + week);
    date_text->update();
}

void TopWidget::updateTime(QString time)
{
    time_label->setText(time);
    time_label->update();
}

void TopWidget::updateWeather(enum weather_t  weather, QString temp = "10~12")
{
    QString ico = weath_list_ico.at(weather);
    QString text = weath_list_text.at(weather);
    weath_ico->setStyleSheet(ico);
    weath_text->setText(text + "\n" +temp+"℃");
    weath_ico->update();
    weath_text->update();
}

void TopWidget::TimeUpdate()
{
//    static int i = -1;
    QString  time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm ddd");
    QString current_date = time.mid(0,4) + "年" + time.mid(5,2) + "月" + time.mid(8,2);
    QString current_time = time.mid(11,5);
    QString week = time.mid(17);

    if(week.at(0) == 'M') {
        week = "星期一";
    } else if (week.at(0)=='T') {
        if(week.at(1) == 'h') {
            week="星期四";
        } else {
            week = "星期二";
        }
    } else if(week.at(0)=='W') {
        week = "星期三";
    } else if(week.at(0) == 'F') {
        week = "星期五";
    } else if((week.at(0) == 'S')) {
        if(week.at(1)=='u') {
            week = "星期日";
        } else {
            week = "星期六";
        }
    }
    updateTime(current_time);
    updateDate(current_date,week);
//    i++;
//    switch(i) {
//    case 0:updateWeather(BAOXUE);break;
//    case 1:updateWeather(BAOYU);break;
//    case 2:updateWeather(DABAOXUE);break;
//    case 3:updateWeather(DABAOYU);break;
//    case 4:updateWeather(DAXUE);break;
//    case 5:updateWeather(DAYU);break;
//    case 6:updateWeather(DUOYUN);break;
//    case 7:updateWeather(LEIZHENYU);break;
//    case 8:updateWeather(MAI);break;
//    case 9:updateWeather(QING);break;
//    case 10:updateWeather(SHACHENBAO);break;
//    case 11:updateWeather(TAIFENG);break;
//    case 12:updateWeather(TEDABAOYU);break;
//    case 13:updateWeather(WU);break;
//    case 14:updateWeather(XIAOYU);break;
//    case 15:updateWeather(YIN);break;
//    case 16:updateWeather(YUJIAXUE);break;
//    case 17:updateWeather(ZHENYU);break;
//    case 18:updateWeather(ZHONGXUE);break;
//    case 19:updateWeather(ZHONGYU);break;
//    case 20:updateWeather(XIAOXUE);break;
//    default:
//        i=-1;
//    }


}