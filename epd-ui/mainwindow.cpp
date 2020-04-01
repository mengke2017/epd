#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "e-paper/IT8951_USB.h"
#include "e-paper/miniGUI_n.h"
#include <QThread>
#include <QFile>
#include <QtXml>
#include <QDebug>
#include <QDomDocument>
#include <QScreen>

#define MAX_LINE_COUNT_MAIN   9 //9
#define MAX_LINE_COUNT_CHILD  5

#define MAX_WIN_WIDTH         1200
#define MAX_WIN_HIGTH         1600

#define MAIN_LINE_FIRST       40
#define MAIN_LINE_DIST        (30+101)

#define MAIN_PAGE    0
#define CHILD_PAGE    1

#define MAIN_TIME   20
#define CHILD_TIME  20

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer();
    screen = QGuiApplication::primaryScreen();
    service = new BackstageManager();
//    show_thread = new EPaper();
//    show_thread->start();

    connect(service,SIGNAL(update_status(QString,QString,QList<qint8>))
            ,this,SLOT(update_status_slot(QString,QString,QList<qint8>)));
    connect(service,SIGNAL(read_weather()),this,SLOT(read_weather_xml()));
    connect(service,SIGNAL(read_line()),this,SLOT(read_lineinfo_xml()));
    connect(service,SIGNAL(read_initpara()),this,SLOT(read_initpara_xml()));
    connect(service,SIGNAL(read_line_style()),this,SLOT(read_lineStyle_xml()));
    connect(service,SIGNAL(update_bulletin(QString)),this,SLOT(update_bulletin_text(QString)));
    connect(service,SIGNAL(shot_screen()),this,SLOT(slotGrabFullScreen()));
    connect(this,SIGNAL(update_shot_screen(uint8)),service,SLOT(to_http(uint8)));

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
    timer->start(500);
    ui->centralWidget->hide();
#if LINUX_32BIT
    init_device();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
    freePage(MAIN_PAGE);
    freePage(CHILD_PAGE);
}

void MainWindow::slotTimerOut()
{
    static int count = -1;
    static bool main_flag = true;

    if(count == MAIN_TIME && main_flag == true) {
        showMainpage();
        count = 0;
        main_flag = false;
        qWarning("bot");
        return;
    } else if(count == CHILD_TIME && main_flag == false) {
        showChildpage();
        count = 0;
        qWarning("top");
        main_flag = true;
        return;
    }

    if(count%8 == 0) {
        screen->grabWindow(0).save("./Screenshot.bmp", "bmp");
    }

    QFileInfo file("./Screenshot.bmp");
    if(file.exists() == true ) {
        dis_epd("./Screenshot.bmp", AUTO_MODE);
        system("rm ./Screenshot.bmp");
    }
    if(count == -1) {
        emit top_widget->timeover(0);
        count = 0;
        return;
    }
    count++;
}

void MainWindow::createMainpage(QList<PageInfo> page_info)
{
    MainLine *line;
    uint16_t ypos = 0, max_y = 0;

    max_y = (mainpage_line_max)*MAIN_LINE_DIST;
    freePage(MAIN_PAGE);
    main_tail = 0;
    uint16 n = 0;
    for (int16_t i = 0; i < page_info.length(); i++) {
        if(!page_info.at(i).showFlag)
            continue;
        ypos = (n*MAIN_LINE_DIST)%max_y;
        line = new MainLine(MAIN_LINE_FIRST + ypos);
        line->creat_line(page_info.at(i).stat_id, page_info.at(i).endstat_name,
                          page_info.at(i).timeSum, page_info.at(i).timeWin,
                          page_info.at(i).price);
        line->setParent(sec_half);
        if(n < mainpage_line_max) {
            main_tail = n;
        } else {
            line->hide();
        }
        mainpage_list.append(line);
        n++;
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
    child_tail = 0;
    uint16 n = 0;
    for(int16_t i = 0; i < page_info.length(); i++) {
        if(!page_info.at(i).showFlag)
            continue;

        ypos = (n*320)%max_y;
        line = new ChildLine(15+ypos);

        line->create_line(page_info.at(i).stat_id, page_info.at(i).current_index,
                          page_info.at(i).station_total,
                          page_info.at(i).name_list, page_info.at(i).timeWin,
                          page_info.at(i).timeSum, page_info.at(i).price);

        line->setParent(bot);
        if(n < childpage_line_max) {
            child_tail = n;
        } else {
            line->hide();
        }
        childpage_list.append(line);
        n++;
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
    QTextCodec *codeC = QTextCodec::codecForName("UTF-8");
 //   read_initpara_xml();
//    read_lineStyle_xml();
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    byte = file.readAll();
    str = codeC->toUnicode(byte.data());
//    str = byte;
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
        if(star_index < 0) {
            break;
        }
        end_index = str.indexOf('>',star_index);
        line = str.mid(star_index, end_index - star_index).simplified();  // 不包括最后的’>‘
        str = str.mid(end_index+1).simplified();

        lineinfo = line.split("\"");

//      获取线路id
        index = lineinfo.indexOf("<line id=");
        if(index < 0 || lineinfo.length() < index + 1)
            continue;
        info.line_id = lineinfo.at(index+1);
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

        info.showFlag = false;
        page_info.append(info);
        info.name_list.clear();
    }
//    line_total = page_info.length();
    line_total = 0;
    uint16 len = page_info.length();
    uint16 i = 0;
    foreach (StyelInfo styleInfo, style_info_list) {
        for(uint16 j = 0; j < len; j++) {
            if(styleInfo.line_id.compare(page_info.at(j).line_id))
                continue;
            page_info.swap(i,j);
            page_info[i].showFlag = true;
            line_total ++;
            break;
        }
        i++;
    }
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
    QString str, weather_str,temp_str;
    int16 index;
    QStringList list;
    QTextCodec *codeC = QTextCodec::codecForName("UTF-8");
    QString temp, weather;
    if (!file.open(QIODevice::ReadOnly))
        return;
    byte = file.readAll();
    file.close();

    str = codeC->toUnicode(byte.data());

    weather_str = read_xml_node(&str, "<weather", "/>", false);
    list = weather_str.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1))
        weather = list.at(index+1);
    list.clear();
    temp_str = read_xml_node(&str, "<temperature", "/>", false);
    list = temp_str.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1))
        temp = list.at(index+1);

    top_widget->updateWeather(weather,temp);
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

    value = read_xml_node(&str,"<setLamp","/>",false);
    list = value.split("\"");
    index = list.indexOf(" open=");
    if(!(index < 0 || list.length() < index + 1))
        para.open = list.at(index+1);
    index = list.indexOf(" shut=");
    if(!(index < 0 || list.length() < index + 1))
        para.shut = list.at(index+1);
    list.clear();

    value = read_xml_node(&str,"<brightness","/>",false);
    list = value.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1))
        para.brightness = list.at(index+1);
    list.clear();

    value = read_xml_node(&str,"<StationName","/>",false);
    list = value.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1))
        para.station_name = list.at(index+1).simplified().replace("（", "(").replace("）", ")");
    list.clear();

    value = read_xml_node(&str,"<switch","/>",false);
    list = value.split("\"");
    index = list.indexOf(" bg_time=");
    if(!(index < 0 || list.length() < index + 1))
        para.bg_time = list.at(index+1);
    index = list.indexOf(" end_time=");
    if(!(index < 0 || list.length() < index + 1))
        para.end_time = list.at(index+1);
    list.clear();

    value = read_xml_node(&str,"<black_scr","/>",false);
    list = value.split("\"");
    index = list.indexOf(" value=");
    if(!(index < 0 || list.length() < index + 1)) {
        para.black_value = list.at(index+1).toInt();
    }
    index = list.indexOf(" count=");
    if(!(index < 0 || list.length() < index + 1)) {
        para.black_count = list.at(index+1).toInt();
    }
    list.clear();

    top_widget->updateStat_name(para.station_name);  // 更新站点名
}

/***********************************************************
**<?xml version="1.0" encoding="UTF-8"?>
** <root>
** <item seq="2" num="1">
**     <left value="335544896" type="1" />
** </item>
** <item seq="3" num="1">
**     <left value="335544935" type="1" />
** </item>
** </root>
*************************************************************/
void MainWindow::read_lineStyle_xml() {
    qWarning("read_lineStyle_xml");
    QFile file("./LineStyle.xml");
    QByteArray byte;
    QString str, value, item;
    StyelInfo style_info;
    QStringList list;
    int index;
    QTextCodec *codeC = QTextCodec::codecForName("UTF-8");
    if (!file.open(QIODevice::ReadOnly))
        return;
    byte = file.readAll();
    file.close();

    str = codeC->toUnicode(byte.data());
    style_info_list.clear();
    do {
        item.clear();
        item = read_xml_node(&str,"<item","</item>",true);  // 先获取itemi节点
        value = read_xml_node(&item,"<left","/>",false);  // 再获取itemi子节点left
        if(item.isEmpty() || value.isEmpty())
            return;
        // 获取该线路排版位置
        list = item.split("\"");
        index = list.indexOf(" seq=");
        if(!(index < 0 || list.length() < index + 1))
            style_info.seq = list.at(index+1);
        list.clear();
        // 获取该线路id
        list = value.split("\"");
        index = list.indexOf(" value=");
        if(!(index < 0 || list.length() < index + 1))
            style_info.line_id = list.at(index+1);
        //获取该线路类型
        index = list.indexOf(" type=");
        if(!(index < 0 || list.length() < index + 1))
            style_info.type = list.at(index+1);
        list.clear();
        style_info_list.append(style_info);
    } while(!item.isEmpty());
}

QString MainWindow::read_xml_node(QString *xml, QString node, QString node_end, bool cut) {  // 若node_end没有，则默认为"/>"
    QString value;
    int star_index, end_index, star_node_index;
    if(xml->isEmpty())
        return "";
    star_index = xml->indexOf(node);
    if(star_index < 0) {
        node = "&lt;"+node.mid(1);
        star_index = xml->indexOf(node);
        if(star_index < 0)
           return "";
    }

    star_node_index = star_index + node.length();

    if(node_end.isEmpty()) {
        node_end = "/>";
    }
    end_index = xml->indexOf(node_end);

    if(end_index < 0) {
        node_end = node_end.remove(">")+"&gt;";
        end_index = xml->indexOf(node_end);
        if(end_index < 0)
           return "";
    }

    value = xml->mid(star_node_index, end_index-star_node_index);
    if(cut)
        xml->remove(star_index, end_index-star_index+node_end.length());
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

void MainWindow::slotGrabFullScreen()
{
   // QScreen *screen = QGuiApplication::primaryScreen();
    QString filePathName = "./Screenshot.jpg";

//    filePathName += name;//QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");
//    filePathName += ".jpg";

    if (!screen->grabWindow(0).save(filePathName, "jpg")) {
        qWarning()<<"save full screen failed!";
    }
    emit update_shot_screen(PUT_SHOT_SCREEN);
}
