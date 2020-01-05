#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "e-paper/IT8951_USB.h"
#include "e-paper/miniGUI.h"
#include <QThread>

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
//    m_pStackedWidget = new QStackedWidget();

    service = new BackstageManager();
    QThread *thread = new QThread(this);
    service->moveToThread(thread);
    connect(thread,SIGNAL(started()),service,SLOT(start()),Qt::QueuedConnection);
    connect(service,SIGNAL(update_status(QList<LineStatus>))
            ,this,SLOT(update_status(QList<LineStatus>)));

    mainpage_line_max = 10;
    childpage_line_max = 6;
    top = new QFrame();
    top->setGeometry(0,0,1200,1600);
    top_widget = new TopWidget();
    top_widget->setParent(top);
    sec_half = new QFrame(top);
    sec_half->setGeometry(0,234,1200,1366);

    bot = new QFrame();
    bot->setGeometry(0,0,1200,1600);

    line_total = 23;
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerOut()));
    timer->start(1000);
    ui->centralWidget->hide();
    init_device();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotTimerOut()
{

    QList<PageInfo> page_info;

    static int i = -1;

    timer->stop();
    if(i == 1) {
        showNextPage(MAIN_PAGE);
    //    top->hide();
    //    bot->show();
        top->repaint();
        top->show();
        i = 0;
        qWarning("bot");
    }  else if(i == 0){
        showNextPage(MAIN_PAGE);
    //    bot->hide();
    //    top->show();
        top->repaint();
        top->show();
        i = 1;
        qWarning("top");
    }
    if(i == -1){
        createMainpage(page_info);
        createChildpage(page_info);
        top->show();
        i = 1;
        qWarning("init");
    } else {
        dis_epd(AUTO_MODE);
    }
    timer->start(10000);
}

void MainWindow::createMainpage(QList<PageInfo> page_info)
{
    MainLine *line;
    uint16_t ypos = 0, max_y = 0;

    max_y = (mainpage_line_max)*MAIN_LINE_DIST;
    for (int16_t i = 0; i < line_total; i++) {
        ypos = (i*MAIN_LINE_DIST)%max_y;
        line = new MainLine(MAIN_LINE_FIRST + ypos);
        line->setParent(sec_half);
        if(i < mainpage_line_max) {
            line->update_line("0", "杭州", "05:00", "22:00", "2", "6");
            main_tail = i;
        } else {
//        line->update_line(page_info.at(i).stat_id, page_info.at(i).endstat_name,
//                          page_info.at(i).Begtime, page_info.at(i).Endtime,
//                          page_info.at(i).price, page_info.at(i).over_count);

        line->update_line("1", "杭州",
                          "05:00", "22:00",
                          "2", "6");
        line->hide();
        }
        mainpage_list.append(line);
    }
    main_tail += 1;
}
void MainWindow::createChildpage(QList<PageInfo> page_info )
{
    ChildLine *line;
    uint16_t ypos = 0, max_y = 0;
    QList<int> array = {0,7,2,5,14,12};
    static QList<QString> list;
    list << "杭州图软科技杭州图软科技"<<"杭州图软科技杭州图软科技西湖站"<<"武林小广场"<<"杭州图软科技"<<"西湖站"<<"武林小广场"
         << "杭州图软科技"<<"西湖站"<<"西湖武林小广场"<<"杭州图软科技"<<"西湖站"<<"武林小广场"
         <<"杭州图软科技"<<"西湖站"<<"武林小广场"<<"杭州图软科技";
    max_y = (childpage_line_max)*320;
    for(int16_t i = 0; i < line_total; i++) {
        ypos = (i*320)%max_y;
        line = new ChildLine(15+ypos);
        if(i < childpage_line_max) {
            line->setParent(bot);
            line->create_line("0", 1,16,
                              list, "05:00",
                              "22:00", "2");
            child_tail = i;
        } else {
//        line->create_line(page_info.at(i).stat_id, page_info.at(i).current_index,page_info.at(i).station_total,
//                          page_info.at(i).name_list, page_info.at(i).Begtime,
//                          page_info.at(i).Endtime, page_info.at(i).price);
        line->create_line("1", 1,16,
                          list, "05:00",
                          "22:00", "2");
        line->setParent(bot);
        }
        line->create_cheico(array);
        childpage_list.append(line);
    }
}

//void MainWindow::switchPage(int Index)
//{
//    m_pStackedWidget->setCurrentIndex(Index);
//}

void MainWindow::showNextPage(int page)
{
    uint8_t i = 0;
    if(page == MAIN_PAGE) {
        if(line_total <= mainpage_line_max)
            return;
        if(main_tail < line_total) {  // 两页显示不下时，继续创建
            for(i = main_tail; i < line_total && (i - main_tail) < mainpage_line_max; i++) {
                mainpage_list.at(i-main_tail)->hide();
                mainpage_list.at(i)->show();
                if(i > 2*mainpage_line_max)
                    mainpage_list.at(i)->update_line("2","1","05:00","22:00","1","1");
            }
            main_tail = i;
        } else if (main_tail == line_total) {  // 如果创建到最后一个了，回到开头
            for(uint8_t i = 0; i < mainpage_line_max; i++) {  // mainpage_line_max是大于mainpage_list.len的
                mainpage_list.at(i)->show();
                main_tail = i;
            }
            main_tail += 1;
            for(uint8_t i = mainpage_line_max; i < line_total; i++) {
                mainpage_list.at(i)->hide();
            }
        }
    } else if(page == CHILD_PAGE) {
        if(line_total <= childpage_line_max)
            return;
        if(child_tail < line_total) {  // 两页显示不下时，继续创建
            for(i = child_tail; i < line_total && (i - child_tail) < childpage_line_max; i++) {
                childpage_list.at(i-child_tail)->hide();
                childpage_list.at(i)->show();
               // if(i > 2*childpage_line_max)
                //    childpage_list.at(i)->update_line("2","1","05:00","22:00","1","1");
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

void MainWindow::update_status(QList<LineStatus> status_list)
{
    for(uint16_t i = 0; i < status_list.length(); i++) {
        for(uint16_t j = 0; j < line_total ;j++) {
            if (status_list.at(i).stat_id.compare(childpage_list.at(j)->line_id->text())) {
                childpage_list.at(j)->update_status(status_list.at(i));
                mainpage_list.at(j)->update_status(status_list.at(i));
            }
        }
    }
}
