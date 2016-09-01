#ifndef BUBBLECHARTPROPERTIES_H
#define BUBBLECHARTPROPERTIES_H

#include <QDialog>
#include <QAbstractItemModel>

namespace Ui {
class BubbleChartProperties;
}

class BlastTaxDataProviders;
class BubbleChartParameters;

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
    void bubbleSizeCalcMethodChanged(int);

private slots:
    void onBubbleMaxSizeSliderValueChanged(int val);
    void onShowTitleToggled(bool val);
    void onDataSourceCheckBoxTriggered(QModelIndex,QModelIndex,QVector<int>);
    void onBubbleSizeCalcMethodChanged(bool);

private:
};

#endif // BUBBLECHARTPROPERTIES_H
