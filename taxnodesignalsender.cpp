#include "taxnodesignalsender.h"

TaxNodeSignalSender taxNodeSignalSender;

TaxNodeSignalSender::TaxNodeSignalSender(QObject *parent)
    : QObject(parent),
      sendSignals(true)
{

}

void TaxNodeSignalSender::setNode(BaseTaxNode *_node)
{
    node = _node;
}

void TaxNodeSignalSender::VisibilityChanged(bool visible)
{
    if ( sendSignals )
        emit visibilityChanged(node, visible);
}

void TaxNodeSignalSender::CollapsedChanged(bool collapsed)
{
    if ( sendSignals )
        emit collapsedChanged(node, collapsed);
}

void TaxNodeSignalSender::GnodeCreated()
{
    if ( sendSignals )
        emit gnodeCreated(node);
}

void TaxNodeSignalSender::GnodeRemoved()
{
    if ( sendSignals )
       emit gnodeRemoved(node);
}

void TaxNodeSignalSender::makeCurrent()
{
    if ( sendSignals )
        emit makeCurrent(node);
}

void TaxNodeSignalSender::BigChangesHappened()
{
    if ( sendSignals )
        emit bigChangesHappened();
}


TaxNodeSignalSender *getTaxNodeSignalSender(BaseTaxNode *n)
{
    taxNodeSignalSender.setNode(n);
    return &taxNodeSignalSender;
}
