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

    BackstageManager *service;

    QFrame *top;
    TopWidget *top_widget;

    QFrame *sec_half;
    QList<MainLine*> mainpage_list;

    QFrame *bot;
    QList<ChildLine*> childpage_list;

    int16_t line_total;
    uint16_t mainpage_line_max;
    uint16_t childpage_line_max;
    uint8_t main_tail;
    uint8_t child_tail;

private :
    void createMainpage(QList<PageInfo> page_info);
    int8_t createLine(int16_t page_number, int16_t count);
    void freePage();
    void createChildpage(QList<PageInfo> page_info);
    void switchPage(int Index);
    void showNextPage(int page);
public slots:
    void update_status(QList<LineStatus>);
private slots:
    void slotTimerOut();
};

#endif // MAINWINDOW_H
