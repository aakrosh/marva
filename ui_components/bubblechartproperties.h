#ifndef BUBBLECHARTPROPERTIES_H
#define BUBBLECHARTPROPERTIES_H

#include <QDialog>

namespace Ui {
class BubbleChartProperties;
}

#define MAX_NODE_SIZE 60

struct BubbleChartConfig
{
    BubbleChartConfig(): bubbleSize(MAX_NODE_SIZE), maxBubbleSize(MAX_NODE_SIZE*2), showTitle(true) {}
    quint32 bubbleSize;
    quint32 maxBubbleSize;
    bool showTitle;
};

class BubbleChartProperties : public QDialog
{
    Q_OBJECT
    BubbleChartConfig *config;
    void setValues();
public:
    Ui::BubbleChartProperties *ui;
    explicit BubbleChartProperties(QWidget *parent, BubbleChartConfig *_config);
    ~BubbleChartProperties();

signals:
    void maxBubbleSizeChanged(int);
    void showTitleToggled(bool);
private slots:
    void onBubbleMaxSizeSliderValueChanged(int val);
    void onShowTitleToggled(bool val);

private:
};

#endif // BUBBLECHARTPROPERTIES_H
