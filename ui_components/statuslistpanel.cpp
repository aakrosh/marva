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
    int cur_height = size().height();
    setMaximumHeight(cur_height+h);
    resize(sizeHint().width(), cur_height+h);
    return item;
}

//=========================================================================
void StatusListPanel::RemoveItem(QListWidgetItem *item)
{
    int h = this->sizeHintForRow(0);
    int cur_height = size().height();
    setMaximumHeight(cur_height-h);
    delete item;
}
