#ifndef BUBBLECHARTPROPERTIES_H
#define BUBBLECHARTPROPERTIES_H

#include <QDialog>

namespace Ui {
class BubbleChartProperties;
}

class BubbleChartProperties : public QDialog
{
    Q_OBJECT

public:
    Ui::BubbleChartProperties *ui;
    explicit BubbleChartProperties(QWidget *parent = 0);
    void setValues(quint32 value, quint32 maxValue=0);
    ~BubbleChartProperties();

signals:
    void maxBubbleSizeChanged(int);

private slots:
    void onBubbleMaxSizeSliderValueChanged(int val);

private:
};

#endif // BUBBLECHARTPROPERTIES_H
