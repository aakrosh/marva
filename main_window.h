#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "graphwidget.h"
#include "blast_data.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    BlastData *blastData;
private:
    Ui::MainWindow *ui;
private slots:
    void open_tab_blast_file();

};

#endif
