#include "statuslistpanel.h"

//=========================================================================
StatusListPanel::StatusListPanel(QWidget *parent) : QListWidget(parent)
{
    resize(sizeHint().width(), 0);
}

//=========================================================================
QListWidgetItem *StatusListPanel::AddItem(QString &text)
{
    QListWidgetItem *item = new QListWidgetItem(text, this);
    addItem(item);
    int h = this->sizeHintForRow(0);
    setMaximumHeight(size().height()+h);
    return item;
}

//=========================================================================
void StatusListPanel::RemoveItem(QListWidgetItem *item)
{
    int h = this->sizeHintForRow(0);
    setMaximumHeight(size().height()-h);
    delete item;
}
