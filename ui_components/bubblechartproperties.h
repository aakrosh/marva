#ifndef BUBBLECHARTPROPERTIES_H
#define BUBBLECHARTPROPERTIES_H

#include <QDialog>
#include <QAbstractItemModel>

namespace Ui {
class BubbleChartProperties;
}

class BlastTaxDataProviders;

#define MAX_NODE_SIZE 60

struct BubbleChartParameters
{
    BubbleChartParameters(): bubbleSize(MAX_NODE_SIZE), maxBubbleSize(MAX_NODE_SIZE*2), showTitle(true) {}
    quint32 bubbleSize;
    quint32 maxBubbleSize;
    bool showTitle;
};

class BubbleChartProperties : public QDialog
{
    Q_OBJECT
    BubbleChartParameters *config;
    void setValues();
public:
    Ui::BubbleChartProperties *ui;
    explicit BubbleChartProperties(QWidget *parent, BubbleChartParameters *_config, BlastTaxDataProviders *dp);
    ~BubbleChartProperties();

signals:
    void maxBubbleSizeChanged(int);
    void showTitleToggled(bool);
    void dataSourceVisibilityChanged(int);
private slots:
    void onBubbleMaxSizeSliderValueChanged(int val);
    void onShowTitleToggled(bool val);
    void onDataSourceCheckBoxTriggered(QModelIndex,QModelIndex,QVector<int>);

private:
};

#endif // BUBBLECHARTPROPERTIES_H
