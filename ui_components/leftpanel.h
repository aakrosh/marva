#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>

namespace Ui {
class LeftPanel;
}

class TaxListWidget;
class CurrentTaxNodeDetails;

class LeftPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LeftPanel(QWidget *parent = 0);
    ~LeftPanel();
    TaxListWidget *taxList();
    CurrentTaxNodeDetails *curNodeDetails();

private:
    Ui::LeftPanel *ui;
};

#endif // LEFTPANEL_H
