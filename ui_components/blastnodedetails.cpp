#include "blastnodedetails.h"
#include "ui_blastnodedetails.h"

BlastNodeDetails::BlastNodeDetails(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlastNodeDetails)
{
    ui->setupUi(this);
}

BlastNodeDetails::~BlastNodeDetails()
{
    delete ui;
}
