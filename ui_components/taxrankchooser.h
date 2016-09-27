#ifndef TAXRANKCHOOSER_H
#define TAXRANKCHOOSER_H

#include "tax_map.h"

#include <QWidget>

namespace Ui {
class TaxRankChooser;
}

class TaxRankChooser : public QWidget
{
    Q_OBJECT

public:
    explicit TaxRankChooser(QWidget *parent = 0);
    ~TaxRankChooser();
    TaxRank rank();

private:
    Ui::TaxRankChooser *ui;

signals:
    selectedTaxRankChanged(TaxRank rank);

private slots:
    void onIndexChanged(int index);
};

#endif // TAXRANKCHOOSER_H
