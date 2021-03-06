#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFrame>
#include <QStackedWidget>
#include "customize.h"
#include "Module/top.h"
#include "Module/mainline.h"
#include "Module/childline.h"
#include "backstagemanager.h"
#include "systemutils.h"
#include "e-paper/e-paper.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QStackedWidget *m_pStackedWidget;
    QScreen *screen;

    BackstageManager *service;

    QFrame *top;
    TopWidget *top_widget;

    QFrame *sec_half;

    Bulletin *bullentin;
    QList<MainLine*> mainpage_list;

    QFrame *bot;
    QList<ChildLine*> childpage_list;

    int16_t line_total;
    uint16_t mainpage_line_max;
    uint16_t childpage_line_max;
    uint8_t main_tail;
    uint8_t child_tail;
    InitPara para;
    bool bullein_flag;
    EPaper *show_thread;
    QList<StyelInfo> style_info_list;

signals:
    void update_shot_screen(uint8 command);
    void set_led(uint8);
private :
    void createMainpage(QList<PageInfo> page_info);
//    int8_t createLine(int16_t page_number, int16_t count);
    void freePage(uint16_t flag);
    void createChildpage(QList<PageInfo> page_info);
    void switchPage(int Index);
    void showNextPage(int page);
    uint8 showMainpage();
    uint8 showChildpage();
//    QString read_xml_node(QString *,QString,QString,bool);
public slots:
    void update_status_slot(QString,QString,QList<qint8>);
    void read_lineinfo_xml();
    void read_weather_xml();
    void read_initpara_xml();
    void read_lineStyle_xml();
    void update_bulletin_text(QString);
    void ui_star_slot(bool);
    void slotGrabFullScreen();
private slots:
    void slotTimerOut();
};

#endif // MAINWINDOW_H
