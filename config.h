#ifndef CONFIG_H
#define CONFIG_H

#include "abstractconfigfile.h"

class BubbleChartConfig : public AbstractConfigBlock
{
    Q_OBJECT
public:
    BubbleChartConfig();
    CONFIG_PROPERTY(int, maxChartTaxes)              // Maximum number of taxonomies in bubble chart
    CONFIG_PROPERTY(int, defaultVisibleChartTaxes)   // Number of visible taxonomies by default
    CONFIG_PROPERTY(int, defaultMaxBubbleSize)       // Default size of biggest bubble
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);
};

class GraphNodeConfig : public AbstractConfigBlock
{
    Q_OBJECT
public:
    GraphNodeConfig();
    CONFIG_PROPERTY(int, nodeCircleSize)             // Maximum number of taxonomies in bubble chart
    CONFIG_PROPERTY(int, halfPlusSize)               // Number of visible taxonomies by default
    CONFIG_PROPERTY(int, maxNodeRadius)              // Default size of biggest bubble
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);
};


class Config : public AbstractConfigFile
{
public:
    Config(QObject *parent);
    virtual void init();
    GETBLOCK(BubbleChart)
    GETBLOCK(GraphNode)
};

extern Config *configuration;

#endif // CONFIG_H
