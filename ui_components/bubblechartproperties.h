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
    void normalizedChanged(bool);
    void showGridChanged(bool);
    void horIntervalChanged(int);
    void dataSourceVisibilityChanged(int);
    void bubbleSizeCalcMethodChanged(int);
    void dataSourceMoved(int, int);

private slots:
    void onBubbleMaxSizeSliderValueChanged(int val);
    void onHorIntervalChanged(int val);
    void onShowTitleToggled(bool val);
    void onNormalizedToggled(bool val);
    void onShowGridToggled(bool val);
    void onDataSourceCheckBoxTriggered(QModelIndex,QModelIndex,QVector<int>);
    void onBubbleSizeCalcMethodChanged(bool);    
    void onDataSourceRowsMoved(int start, int row);

private:
};

#endif // BUBBLECHARTPROPERTIES_H
