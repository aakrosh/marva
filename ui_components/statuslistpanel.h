#ifndef STATUSLISTPANEL_H
#define STATUSLISTPANEL_H

#include <QListWidget>
class QListWidgetItem;

class StatusListPanel : public QListWidget
{
public:
    StatusListPanel(QWidget *parent);
    QListWidgetItem *AddItem(QString &text);
    void RemoveItem(QListWidgetItem *);
};

#endif // STATUSLISTPANEL_H
