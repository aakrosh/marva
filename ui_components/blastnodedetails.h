#ifndef BLASTNODEDETAILS_H
#define BLASTNODEDETAILS_H

#include <QDialog>

namespace Ui {
class BlastNodeDetails;
}

class BlastNodeDetails : public QDialog
{
    Q_OBJECT

public:
    explicit BlastNodeDetails(QWidget *parent = 0);
    ~BlastNodeDetails();

private:
    Ui::BlastNodeDetails *ui;
};

#endif // BLASTNODEDETAILS_H
