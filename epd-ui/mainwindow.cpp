#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "e-paper/IT8951_USB.h"
#include "e-paper/miniGUI.h"
#include <QThread>
#include <QFile>
#include <QtXml>
#include <QDebug>
#include <QDomDocument>

#define MAX_LINE_COUNT_MAIN   9 //9
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
    show_thread = new EPaper();
    show_thread->start();

    connect(service,SIGNAL(update_status(QString,QString,QList<qint8>))
            ,this,SLOT(update_status_slot(QString,QString,QList<qint8>)));
    connect(service,SIGNAL(read_weather()),this,SLOT(read_weather_xml()));
    connect(service,SIGNAL(read_line()),this,SLOT(read_lineinfo_xml()));
    connect(service,SIGNAL(read_initpara()),this,SLOT(read_initpara_xml()));
    connect(service,SIGNAL(update_bulletin(QString)),this,SLOT(update_bulletin_text(QString)));

    mainpage_line_max = MAX_LINE_COUNT_MAIN;
    childpage_line_max = MAX_LINE_COUNT_CHILD;
    bullein_flag = false;
    top = new QFrame();
    top->setGeometry(0,0,1200,1600);
    top_widget = new TopWidget();
    top_widget->setParent(top);
    sec_half = new QFrame(top);
    sec_half->setGeometry(0,234,1200,1366);

    bullentin = new Bulletin(sec_half, 1206); // 1216

    bot = new QFrame();
    bot->setGeometry(0,0,1200,1600);

    line_total = 0;
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerOut()));
    connect(top_widget, SIGNAL(timeover(uint)), service, SLOT(serTimerOut(uint)));
    timer->start(1000);
    ui->centralWidget->hide();

//    RecvThread * recv;
//    recv = new RecvThread;
//    recv->start();
}

MainWindow::~MainWindow()
{
    delete ui;
    freePage(MAIN_PAGE);
    freePage(CHILD_PAGE);
}

void MainWindow::slotTimerOut()
{
    static int i = -1;

    if(i == 1) {
        showMainpage();
        i = 0;
        qWarning("bot");
    }  else if(i == 0){
        showChildpage();
        i = 1;
        qWarning("top");
    }
    if(i == -1){
        timer->stop();
       // read_initpara_xml();
       // read_lineinfo_xml();
       // top->show();
       top_widget->timeover(0);
        i = 1;
        timer->start(10000);
        qWarning("init");
    }
}

void MainWindow::createMainpage(QList<PageInfo> page_info)
{
    MainLine *line;
    uint16_t ypos = 0, max_y = 0;

    max_y = (mainpage_line_max)*MAIN_LINE_DIST;
    freePage(MAIN_PAGE);
    for (int16_t i = 0; i < page_info.length(); i++) {
        ypos = (i*MAIN_LINE_DIST)%max_y;
        line = new MainLine(MAIN_LINE_FIRST + ypos);
        line->creat_line(page_info.at(i).stat_id, page_info.at(i).endstat_name,
                          page_info.at(i).timeSum, page_info.at(i).timeWin,
                          page_info.at(i).price);
        line->setParent(sec_half);
        if(i < mainpage_line_max) {
            main_tail = i;
        } else {
            line->hide();
        }
        mainpage_list.append(line);
    }
    main_tail += 1;
    bullentin ->update_text("公告", "提高警惕，保持距离，电话沟通，钉钉开会，不要聚集。各部门负责人要疫情强化风险担当，为健康生产保驾护航。");
}
void MainWindow::createChildpage(QList<PageInfo> page_info)
{
    ChildLine *line;
    uint16_t ypos = 0, max_y = 0;
    max_y = (childpage_line_max)*320;
    freePage(CHILD_PAGE);
    for(int16_t i = 0; i < page_info.length(); i++) {
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

uint8 MainWindow::showMainpage() {
    static bool top_first = true;
    uint8_t i =0;
    if(line_total <= mainpage_line_max || top_first == true) {  // 一页就可以显示完
        bot->hide();
        top->show();
        top_first = false;
        if(line_total <= mainpage_line_max) {
            return 0;
        }
    } else if(main_tail < line_total) {  // 两页显示不下时，继续创建
        for(i = main_tail; i < line_total && (i - main_tail) < mainpage_line_max; i++) {
            mainpage_list.at(i-main_tail)->hide();
            mainpage_list.at(i)->show();
        }
        main_tail = i;
//            top->repaint();
        bot->hide();
        top->show();
        if(main_tail < line_total)
            return 1;
        else
            return 0;
    } else if (main_tail == line_total) {  // 如果创建到最后一个了，回到开头
        for(uint8_t i = 0; i < mainpage_line_max; i++) {  // mainpage_line_max是大于mainpage_list.len的
            mainpage_list.at(i)->show();
            main_tail = i;
        }
        main_tail += 1;
        for(uint8_t i = mainpage_line_max; i < line_total; i++) {
            mainpage_list.at(i)->hide();
        }
       // top_first = true;
        bot->hide();
        top->show();
        return 1;
    }
    return 0;
}

uint8 MainWindow::showChildpage() {
    static bool bot_first = true;
    uint8_t i= 0;
    if(line_total <= childpage_line_max ||  bot_first == true) {
        top->hide();
        bot->show();
        bot_first = false;
        if(line_total <= childpage_line_max)
            return 0;
    } else if(child_tail < line_total) {  // 两页显示不下时，继续创建
        for(i = child_tail; i < line_total && (i - child_tail) < childpage_line_max; i++) {
            childpage_list.at(i-child_tail)->hide();
            childpage_list.at(i)->show();
        }
        child_tail = i;
        top->hide();
        bot->show();
        if(child_tail < line_total)
            return 1;
        else
            return 0;
    } else if (child_tail == line_total) {  // 如果创建到最后一个了，回到开头
        for(uint8_t i = 0; i < childpage_line_max; i++) {  // mainpage_line_max是大于mainpage_list.len的
            childpage_list.at(i)->show();
            child_tail = i;
        }
        child_tail += 1;
        for(uint8_t i = childpage_line_max; i < line_total; i++) {
            childpage_list.at(i)->hide();
        }
       // bot_first = true;
      //  bot->repaint();
        top->hide();
        bot->show();
        return 1;
    }
    return 0;
}

void MainWindow::update_status_slot(QString stat_id,QString count,QList<qint8> pos) {
//    qDebug()<<"update_status";
    for(uint16_t j = 0; j < line_total ;j++) {
        if (!stat_id.compare(childpage_list.at(j)->line_id->text())) {
            childpage_list.at(j)->update_status(pos);
            if (count.toInt() < 0)
                count = "--";
            mainpage_list.at(j)->update_status(count);
        }
    }
}

void MainWindow::update_bulletin_text(QString text) {  //轮寻判断是否有公告，还有公告显示的时间
    if(text.size() > 125)
        text = text.mid(0,125);
    bullentin ->update_text("公告",text);
}

void MainWindow::read_lineinfo_xml() {
    qWarning("read_lineinfo_xml");
    QFile file("./station_line.xml");
    QString str;
    QString line;
    int star_index = 0, end_index = 0;
    QList<PageInfo> page_info;
    PageInfo info;
    QByteArray byte;
    QStringList lineinfo;
    int index;
    read_initpara_xml();
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    byte = file.readAll();
//    str = codeC->toUnicode(byte.data());
    str = byte;
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
       // qWarning("index:%d", star_index);
        if(star_index < 0) {
       //     qWarning()<<"break";
            break;
        }
        end_index = str.indexOf('>',star_index);
        line = str.mid(star_index, end_index - star_index).simplified();  // 不包括最后的’>‘
        str = str.mid(end_index+1).simplified();        
//        line.indexOf("name=");
        lineinfo = line.split("\"");
        // 获取线路名称
    //    info.stat_id = line.section("\"",3,3);  // 截取线路名称 获取从第1个“分割的块开始到第1个“分割的块结束
        index = lineinfo.indexOf(" name=");
        if(index < 0 || lineinfo.length() < index + 1)
            continue;
        info.stat_id = lineinfo.at(index+1);

        if(info.stat_id.right(1) == "路") {
            info.stat_id = info.stat_id.left(info.stat_id.count()-1);
            info.stat_id = info.stat_id.simplified();  // 去除空白字符
        }
        // 获取总站点数
        index = lineinfo.indexOf(" staCount=");
        if(index < 0 || lineinfo.length() < index + 1)
            continue;
        info.station_total = lineinfo.at(index + 1).simplified().toInt();
        if(info.station_total <= 0)
            continue;
        // 获取发车时间
        index = lineinfo.indexOf(" sSta_Summers=");
        if(index < 0 || lineinfo.length() < index + 1)
            continue;
        info.timeSum = lineinfo.at(index+1);

        index = lineinfo.indexOf(" sSta_Winters=");
        if(index < 0 || lineinfo.length() < index + 1)
            continue;
        info.timeWin = lineinfo.at(index+1);
        info.timeSum = info.timeSum.simplified();
        info.timeWin = info.timeWin.simplified();
        // 获取价格
        index = lineinfo.indexOf(" price=");
        if(index < 0 || lineinfo.length() < index + 1)
            continue;
        info.price = lineinfo.at(index+1);
        info.price = info.price.simplified();

        /*<station id="1929381411" name="飞云渡-西环线" index="1" latitude="27.78393" longitude="120.61973" subway="" />*/
        //获取站点名
        star_index = str.indexOf("<station id=");
        end_index = str.indexOf("</line>");
        if(star_index < 0 || end_index < 0)
            continue;
        line = str.mid(star_index, end_index - star_index);  // 不包括最后的"</line>"
        lineinfo.clear();
        lineinfo = line.split("/>");  // 通过“分解字符串
        QStringList list;
        QString statname;
        for(int32_t i = 0; i < lineinfo.length(); i++) {
            list = lineinfo.at(i).split("\"");
            if(list.size() < 3)
                continue;
            statname = list.at(3).simplified().replace("（", "(").replace("）", ")");
            if(!statname.compare(para.station_name))
                info.current_index = i;
            QString s = statname.replace("(", "︵ ").replace(")", " ︶");  //︵︶
            info.name_list.append(s);
            list.clear();
        }
        info.endstat_name = info.name_list.at(info.station_total-1);

        page_info.append(info);
        info.name_list.clear();
    }
    line_total = page_info.length();

    createChildpage(page_info);
    createMainpage(page_info);
    slotTimerOut();
}

/*
<?xml version="1.0" encoding="UTF-8"?>
<root><adcode value="00" /><weather value="晴" />
<winddirection value="风向北" /><windpower value="风力≤3级" /><temperature value="19" />
<humidity value="42" /></root>
*/
void MainWindow::read_weather_xml() {
    qWarning("read_weather_xml");
    QFile file("./weather_information.xml");
    QByteArray byte;
    QString str;
    int star_index = 0, end_index = 0;
    QStringList list;
    QTextCodec *codeC = QTextCodec::codecForName("UTF-8");
    if (!file.open(QIODevice::ReadOnly))
        return;
    byte = file.readAll();
    file.close();

    str = codeC->toUnicode(byte.data());
    star_index = str.indexOf("weather value");
    end_index = str.indexOf("/root");
    if(star_index < 0 || end_index < 0) {
        return;
    }
    str = str.mid(star_index,end_index-star_index);
    list = str.split("\"");

    QString weath = list.at(1);
    QString temp = list.at(7);
    top_widget->updateWeather(weath,temp);
}

/*<?xml version="1.0" encoding="UTF-8"?><root><StationName value="锦城春天-东向" /><Volume /><Lumia />
 *<setLamp open="0:00:00" shut="23:59:59" /><version value="" /><switch bg_time="05:00:00" end_time="22:59:59" />
 *<brightness value="2" /><fan max="0" min="0" /><heater max="0" min="0" /><black_scr value="4" count="1000" /></root>
*/
void MainWindow::read_initpara_xml() {
    // init_station
    qWarning("init_station");
    QFile file("./init_station.xml");
    QByteArray byte;
    QString str, value;
    QStringList list;
    int index;
    QTextCodec *codeC = QTextCodec::codecForName("UTF-8");
    if (!file.open(QIODevice::ReadOnly))
        return;
    byte = file.readAll();
    file.close();

    str = codeC->toUnicode(byte.data());

    value = read_xml_node(str,"<setLamp","/>");
    list = value.split("\"");
    index = list.indexOf(" open=");
    if(!(index < 0 || list.length() < index + 1))
        para.open = list.at(index+1);
    index = list.indexOf(" shut=");
    if(!(index < 0 || list.length() < index + 1))
        para.shut = list.at(index+1);
    list.clear();

    value = read_xml_node(str,"<brightness","/>");
    list = value.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1))
        para.brightness = list.at(index+1);
    list.clear();

    value = read_xml_node(str,"<StationName","/>");
    list = value.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1))
        para.station_name = list.at(index+1).simplified().replace("（", "(").replace("）", ")");
    list.clear();

    value = read_xml_node(str,"<switch","/>");
    list = value.split("\"");
    index = list.indexOf(" bg_time=");
    if(!(index < 0 || list.length() < index + 1))
        para.bg_time = list.at(index+1);
    index = list.indexOf(" end_time=");
    if(!(index < 0 || list.length() < index + 1))
        para.end_time = list.at(index+1);
    list.clear();

    value = read_xml_node(str,"<black_scr","/>");
    list = value.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1)) {
      //  qWarning()<<list.at(index+1);
        para.black_value = list.at(index+1).toInt();
    }
    index = list.indexOf(" count=");
    if(!(index < 0 || list.length() < index + 1)) {
     //   qWarning()<<list.at(index+1);
        para.black_count = list.at(index+1).toInt();
    }
    list.clear();

    top_widget->updateStat_name(para.station_name);  // 更新站点名
}

QString MainWindow::read_xml_node(QString xml, QString node, QString node_end) {  // 若node_end没有，则默认为"/>"
    QString value;
    int star_index, end_index;
    star_index = xml.indexOf(node);
    star_index += node.length();
    if(star_index < 0)
        return "";
    if(node_end.isEmpty()) {
        node_end = "/>";
    }
    end_index = xml.indexOf(node_end);
    value = xml.mid(star_index, end_index-star_index);
    return value;
}

void MainWindow::freePage(uint16_t flag) {
    if(flag == MAIN_PAGE) {
        if(mainpage_list.isEmpty())
            return;
        for(uint16_t i = 0; i < mainpage_list.length(); i++) {
            delete mainpage_list[i];
        }
        mainpage_list.clear();
    } else if(flag == CHILD_PAGE) {
        if(childpage_list.isEmpty())
            return;
        for(uint16_t i = 0; i < childpage_list.length(); i++) {
            delete childpage_list[i];
        }
        childpage_list.clear();
    }
}

void MainWindow::ui_star_slot(bool flag)
{
    if(flag)
        timer->start(5000);
    else
        timer->stop();
}
