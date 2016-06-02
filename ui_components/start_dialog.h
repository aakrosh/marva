#ifndef START_DIALOG_H
#define START_DIALOG_H

#include <QDialog>

namespace Ui {
class StartDialog;
}

class StartDialog : public QDialog
{
    Q_OBJECT

public:
    QString projectFileName;
    explicit StartDialog(QWidget *parent = 0);
    ~StartDialog();

private:
    Ui::StartDialog *ui;

private slots:
    void  openLastProject();
    void  openProjectFile();
    void  createEmptyProject();

signals:
    fileChoosen(QString fileName);
};

#endif // START_DIALOG_H
