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
signals:
    visibilityChanged(BaseTaxNode *n, bool visible);
    collapsedChanged(BaseTaxNode *n, bool collapsed);
    gnodeCreated(BaseTaxNode *n);
    gnodeRemoved(BaseTaxNode *n);
    makeCurrent(BaseTaxNode *n);
    bigChangesHappened();
public slots:
};

/* WARNING!!!! THIS IS NOT THREAD SAFE AT ALL!!!! */
/* BE CAREFUL WITH USING IT!!!!*/
extern TaxNodeSignalSender taxNodeSignalSender;

TaxNodeSignalSender *getTaxNodeSignalSender(BaseTaxNode *n);

#endif // TAXNODESIGNALSENDER_H
