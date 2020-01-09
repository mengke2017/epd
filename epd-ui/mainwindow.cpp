#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "e-paper/IT8951_USB.h"
#include "e-paper/miniGUI.h"
#include <QThread>
#include <QFile>
#include <QtXml>
#include <QDebug>
#include <QDomDocument>

#define MAX_LINE_COUNT_MAIN   10
#define MAX_LINE_COUNT_CHILD  5

#define MAX_WIN_WIDTH         1200
#define MAX_WIN_HIGTH         1600

#define MAIN_LINE_FIRST       40
#define MAIN_LINE_DIST        (30+101)

#define MAIN_PAGE    0
#define CHILD_PAGE    1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer();

    service = new BackstageManager();
//    QThread *thread = new QThread(this);
//    service->moveToThread(thread);
//    service->start();
//    connect(thread,SIGNAL(started()),service,SLOT(start()),Qt::QueuedConnection);
    connect(service,SIGNAL(update_status(QString,QString,QList<qint8>))
            ,this,SLOT(update_status(QString,QString,QList<qint8>)));

    mainpage_line_max = 10;
    childpage_line_max = 6;
    bullein_flag = false;
    top = new QFrame();
    top->setGeometry(0,0,1200,1600);
    top_widget = new TopWidget();
    top_widget->setParent(top);
    sec_half = new QFrame(top);
    sec_half->setGeometry(0,234,1200,1366);


    bullentin = new Bulletin(sec_half);
    bullentin->setGeometry(0, 1066, 1200, 300);

    bot = new QFrame();
    bot->setGeometry(0,0,1200,1600);

    line_total = 0;
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerOut()));
    timer->start(1000);
    ui->centralWidget->hide();
    init_device();

//    read_lineinfo();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotTimerOut()
{

 //   QList<PageInfo> page_info;

    static int i = -1;


    if(i == 1) {
    //    showNextPage(MAIN_PAGE);
        top->hide();
        bot->show();
    //    top->repaint();
    //    top->show();
        i = 0;
        qWarning("bot");
    }  else if(i == 0){
    //    showNextPage(MAIN_PAGE);
        bot->hide();
        top->show();
    //    top->repaint();
        //bot->show();
        i = 1;
        qWarning("top");
    }
    if(i == -1){
        timer->stop();
        read_lineinfo();
        bot->show();
        i = 1;
        timer->start(10000);
        qWarning("init");
    } else {
        dis_epd(AUTO_MODE);
    }
}

void MainWindow::createMainpage(QList<PageInfo> page_info)
{
    MainLine *line;
    uint16_t ypos = 0, max_y = 0;

    max_y = (mainpage_line_max)*MAIN_LINE_DIST;
    for (int16_t i = 0; i < line_total; i++) {
        ypos = (i*MAIN_LINE_DIST)%max_y;
        line = new MainLine(MAIN_LINE_FIRST + ypos);
        line->creat_line(page_info.at(i).stat_id, page_info.at(i).endstat_name,
                          page_info.at(i).Begtime, page_info.at(i).Endtime,
                          page_info.at(i).price);
        line->setParent(sec_half);
        if(i < mainpage_line_max) {
           // line->creat_line("0", "杭州", "05:00", "22:00", "2");
            main_tail = i;
        } else {
            line->hide();
        }
        bullentin->update_text("公告", "明天放假。");
        mainpage_list.append(line);
    }
    main_tail += 1;
}
void MainWindow::createChildpage(QList<PageInfo> page_info )
{
    ChildLine *line;
    uint16_t ypos = 0, max_y = 0;/*
   // QList<int> array = {0,7,2,5,14,12};
    static QList<QString> list;
    list << "杭州图软科技杭州图软科技"<<"杭州图软科技杭州图软科技西湖站"<<"武林小广场"<<"杭州图软科技"<<"西湖站"<<"武林小广场"
         << "杭州图软科技"<<"西湖站"<<"西湖武林小广场"<<"杭州图软科技"<<"西湖站"<<"武林小广场"
         <<"杭州图软科技"<<"西湖站"<<"武林小广场"<<"杭州图软科技";*/
    max_y = (childpage_line_max)*320;
    for(int16_t i = 0; i < line_total; i++) {
        ypos = (i*320)%max_y;
        line = new ChildLine(15+ypos);

        line->create_line(page_info.at(i).stat_id, page_info.at(i).current_index,
                          page_info.at(i).station_total,
                          page_info.at(i).name_list, page_info.at(i).timeWin,
                          page_info.at(i).timeSum, page_info.at(i).price);

        line->setParent(bot);
        if(i < childpage_line_max) {
            child_tail = i;
        } else {
            line->hide();
        }
        childpage_list.append(line);
    }
    child_tail +=1;
}

//void MainWindow::switchPage(int Index)
//{
//    m_pStackedWidget->setCurrentIndex(Index);
//}

void MainWindow::showNextPage(int page)
{
    uint8_t i = 0;
    static bool top_first = true, bot_first = true;
    // 第一次进来 先显示 top

    if(page == MAIN_PAGE) {
        if(line_total <= mainpage_line_max) {
            bot->hide();
            top->show();
            return;
        }
        if(main_tail < line_total && main_tail > mainpage_line_max) {  // 两页显示不下时，继续创建
            for(i = main_tail; i < line_total && (i - main_tail) < mainpage_line_max; i++) {
                mainpage_list.at(i-main_tail)->hide();
                mainpage_list.at(i)->show();
            }
            main_tail = i;
            // 显示top
            bot->hide();
            top->repaint();
            top->show();
        } else if (main_tail == line_total) {  // 如果创建到最后一个了，回到开头
            for(uint8_t i = 0; i < mainpage_line_max; i++) {  // mainpage_line_max是大于mainpage_list.len的
                mainpage_list.at(i)->show();
                main_tail = i;
            }
            main_tail += 1;
            for(uint8_t i = mainpage_line_max; i < line_total; i++) {
                mainpage_list.at(i)->hide();
            }
            // 显示bot
            top->repaint();
            top->hide();
            bot->show();
        }
    } else if(page == CHILD_PAGE) {
        if(line_total <= childpage_line_max)
            return;
        if(child_tail < line_total) {  // 两页显示不下时，继续创建
            for(i = child_tail; i < line_total && (i - child_tail) < childpage_line_max; i++) {
                childpage_list.at(i-child_tail)->hide();
                childpage_list.at(i)->show();
            }
            child_tail = i;
        } else if (child_tail == line_total) {  // 如果创建到最后一个了，回到开头
            for(uint8_t i = 0; i < childpage_line_max; i++) {  // mainpage_line_max是大于mainpage_list.len的
                childpage_list.at(i)->show();
                child_tail = i;
            }
            child_tail += 1;
            for(uint8_t i = childpage_line_max; i < line_total; i++) {
                childpage_list.at(i)->hide();
            }
        }
    }
}

void MainWindow::update_status(QString stat_id,QString count,QList<qint8> pos)
{
    qDebug()<<"111111";
    for(uint16_t j = 0; j < line_total ;j++) {
        if (stat_id.compare(childpage_list.at(j)->line_id->text())) {
            childpage_list.at(j)->update_status(pos);
            mainpage_list.at(j)->update_status(count);
        }
    }
}

void MainWindow::read_lineinfo()
{
 //   QDomDocument doc;
    QFile file("./line.xml");
    QString str;
    QString line;
    int star_index = 0, end_index = 0;
    QList<PageInfo> page_info;
    PageInfo info;
    QStringList lineinfo;
    qint16 count = 0;
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
 //   QTextCodec *gbk = QTextCodec::codecForName("gbk");
    QString text = file.readAll();
    str = text;// gbk->toUnicode(text.toLocal8Bit());
    file.close();
    while(1) {
        /*
            <line id="335544524" name="1路" ic_card_price="0" country_type="0" isUP=""
            staCount="32" curstaid="1929380848" sSta_Summers="06:00--21:30" sSta_Winters="06:00--21:30"
            eSta_Summers="06:00--21:30" eSta_Winters="06:00--21:30"
            price="2" comment="" price_range="2元" avg_speed="25">
        */
        // 获取线路基本信息
        star_index = str.indexOf("<line id=");
        qWarning("index:%d", star_index);
        if(star_index < 0) {
            qWarning()<<"break";
            break;
        }
        end_index = str.indexOf('>',star_index);
        line = str.mid(star_index, end_index - star_index).simplified();  // 不包括最后的’>‘
        str = str.mid(end_index+1).simplified();
        lineinfo = line.split("\"");
        // 获取线路名称
    //    info.stat_id = line.section("\"",3,3);  // 截取线路名称 获取从第1个“分割的块开始到第1个“分割的块结束
        info.stat_id = lineinfo.at(3);
        if(info.stat_id.right(1) == "路") {
            info.stat_id = info.stat_id.left(info.stat_id.count()-1);
            info.stat_id = info.stat_id.simplified();  // 去除空白字符
        }
        // 获取总站点数
        info.station_total = lineinfo.at(11).simplified().toInt();
        // 获取发车时间
        info.timeSum = lineinfo.at(15);
        info.timeWin = lineinfo.at(17);
        info.timeSum = info.timeSum.simplified();
        info.timeWin = info.timeWin.simplified();
        // 获取价格
        info.price = lineinfo.at(23);
        info.price = info.price.simplified();
        /*<station id="1929381411" name="飞云渡-西环线" index="1" latitude="27.78393" longitude="120.61973" subway="" />*/
        //获取站点名
        star_index = str.indexOf("<station id=");
        end_index = str.indexOf("</line>");
        if(star_index < 0 || end_index < 0)
            return;
        line = str.mid(star_index, end_index - star_index);  // 不包括最后的"</line>"
        lineinfo.clear();
        lineinfo = line.split("/>");  // 通过“分解字符串
        QStringList list;
        for(uint16_t i = 0; i < lineinfo.length(); i++) {
            list = lineinfo.at(i).split("\"");
            if(list.size() < 3)
                continue;
            QString s = list.at(3).simplified().replace("（", "︵ ").replace("）", " ︶");
            s = s.replace("(", "︵ ");  //︵︶
            s= s.replace(")", " ︶");
            info.name_list.append(s);
            list.clear();
        }
      //  qDebug()<<"stat_id:"<<info.stat_id;
      //  qDebug()<<"price:"<<info.price;
      //  qDebug()<<"timeSum:"<<info.timeSum;
      // qDebug()<<"timeWin:"<<info.timeWin;
      //  qDebug()<<"station_total:"<<QString::number(info.station_total);
        //for(uint16_t i = 0; i < info.name_list.length();i++)
      //  {
        //    qDebug()<<"name_list:"<<info.name_list.at(i);
      //  }
        info.endstat_name = info.name_list.at(info.station_total-1);
        page_info.append(info);
        info.name_list.clear();
        count++;
    }
    line_total = count;

    createChildpage(page_info);
    createMainpage(page_info);
}
