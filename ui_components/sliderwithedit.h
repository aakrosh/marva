#ifndef SLIDERWITHEDIT_H
#define SLIDERWITHEDIT_H

#include <QWidget>

namespace Ui {
class SliderWithEdit;
}

class SliderWithEdit : public QWidget
{
    Q_OBJECT
    int oldValue;
public:
    explicit SliderWithEdit(QWidget *parent = 0);
    ~SliderWithEdit();
    int maxValue();
    int value();
    void setLabel(QString label);
public slots:
    void setMaxValue(int maxVal);
    void setMinValue(int minVal);
    void setValue(int val);
private slots:
    void onValueChanged(int);
signals:
    void valueChanged(quint32, quint32);
private:    
    Ui::SliderWithEdit *ui;
};

#endif // SLIDERWITHEDIT_H
