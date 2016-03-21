#include "taxnodesignalsender.h"

TaxNodeSignalSender taxNodeSignalSender;

TaxNodeSignalSender::TaxNodeSignalSender(QObject *parent)
    : QObject(parent)
{

}

void TaxNodeSignalSender::setNode(BaseTaxNode *_node)
{
    node = _node;
}

void TaxNodeSignalSender::VisibilityChanged(bool visible)
{
    emit visibilityChanged(node, visible);
}

void TaxNodeSignalSender::CollapsedChanged(bool collapsed)
{
    emit collapsedChanged(node, collapsed);
}

void TaxNodeSignalSender::GnodeCreated()
{
    emit gnodeCreated(node);
}

void TaxNodeSignalSender::GnodeRemoved()
{
    emit gnodeRemoved(node);
}


TaxNodeSignalSender *getTaxNodeSignalSender(BaseTaxNode *n)
{
    taxNodeSignalSender.setNode(n);
    return &taxNodeSignalSender;
}
