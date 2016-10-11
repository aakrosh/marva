#ifndef TAXNODESIGNALSENDER_H
#define TAXNODESIGNALSENDER_H

#include <QObject>

class BaseTaxNode;

class TaxNodeSignalSender : public QObject
{
    Q_OBJECT

    BaseTaxNode *node;
public:
    bool sendSignals;

    explicit TaxNodeSignalSender(QObject *parent = 0);
    void setNode(BaseTaxNode *_node);
    void VisibilityChanged(bool visible);
    void CollapsedChanged(bool collapsed);
    void GnodeCreated();
    void GnodeRemoved();
    void makeCurrent();
    void BigChangesHappened();
    void ColorChanged();
signals:
    void visibilityChanged(BaseTaxNode *n, bool visible);
    void collapsedChanged(BaseTaxNode *n, bool collapsed);
    void gnodeCreated(BaseTaxNode *n);
    void gnodeRemoved(BaseTaxNode *n);
    void makeCurrent(BaseTaxNode *n);
    void bigChangesHappened();
    void colorChanged(BaseTaxNode *n);
public slots:
};

/* WARNING!!!! THIS IS NOT THREAD SAFE AT ALL!!!! */
/* BE CAREFUL WITH USING IT!!!!*/
extern TaxNodeSignalSender taxNodeSignalSender;

TaxNodeSignalSender *getTaxNodeSignalSender(BaseTaxNode *n);

#endif // TAXNODESIGNALSENDER_H
