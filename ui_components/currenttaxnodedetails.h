#ifndef CURRENTTAXNODEDETAILS_H
#define CURRENTTAXNODEDETAILS_H

#include <QWidget>

class BaseTaxNode;
class TaxDataProvider;

namespace Ui {
class CurrentTaxNodeDetails;
}

class CurrentTaxNodeDetails : public QWidget
{
    Q_OBJECT
    qint32 curNodeId;
    TaxDataProvider *dataProvider;
public:
    explicit CurrentTaxNodeDetails(QWidget *parent = 0);
    ~CurrentTaxNodeDetails();
    void setTaxDataProvider(TaxDataProvider *tdp);

    void refresh();

private:
    Ui::CurrentTaxNodeDetails *ui;

public slots:
    void onCurrentNodeChanged(BaseTaxNode *node);
    void onColorChanged(BaseTaxNode *node);
    void onBColorClicked();
};

#endif // CURRENTTAXNODEDETAILS_H
