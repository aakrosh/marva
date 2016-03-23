#ifndef CURRENTTAXNODEDETAILS_H
#define CURRENTTAXNODEDETAILS_H

#include <QWidget>

class BaseTaxNode;

namespace Ui {
class CurrentTaxNodeDetails;
}

class CurrentTaxNodeDetails : public QWidget
{
    Q_OBJECT

public:
    explicit CurrentTaxNodeDetails(QWidget *parent = 0);
    ~CurrentTaxNodeDetails();

private:
    Ui::CurrentTaxNodeDetails *ui;

public slots:
    void onCurrentNodeChanged(BaseTaxNode *node);
};

#endif // CURRENTTAXNODEDETAILS_H
