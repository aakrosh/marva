#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include <QMainWindow>
#include "graphwidget.h"
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
  Q_OBJECT
public:
  MainWindow() : QMainWindow(NULL)
  {
    setupUi(this);
    connect(this->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    GraphWidget *gwidget = new GraphWidget;
    setCentralWidget(gwidget);
  }
};

#endif
