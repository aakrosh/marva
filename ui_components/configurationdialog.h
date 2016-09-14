#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigurationDialog;
}

class ConfigurationDialog : public QDialog
{
    Q_OBJECT
    bool inited;
public:
    explicit ConfigurationDialog(QWidget *parent = 0);
    ~ConfigurationDialog();
    virtual void closeEvent(QCloseEvent *event);
private:
    Ui::ConfigurationDialog *ui;
private slots:
    void onConfigChanged();
    void onGiToTaxMapPathClicked();
signals:
    void configChanged();
};

#endif // CONFIGURATIONDIALOG_H
