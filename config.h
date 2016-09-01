#ifndef CONFIG_H
#define CONFIG_H

#include "abstractconfigfile.h"

// Bubble size calculation methods
#define METHOD_LINEAR   0
#define METHOD_SQRT     1

class BubbleChartConfig : public AbstractConfigBlock
{
    Q_OBJECT
public:
    BubbleChartConfig();
    CONFIG_PROPERTY(int, maxChartTaxes)              // Maximum number of taxonomies in bubble chart
    CONFIG_PROPERTY(int, defaultVisibleChartTaxes)   // Number of visible taxonomies by default
    CONFIG_PROPERTY(int, defaultMaxBubbleSize)       // Default size of biggest bubble
    CONFIG_PROPERTY(int, bubbleSizeCalculationMethod)   // Method to calculate bubble size depending on biggest bubble size
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);
};

#define EDGE_LINE   0
#define EDGE_CURVE  1

class GraphNodeConfig : public AbstractConfigBlock
{
    Q_OBJECT
public:
    GraphNodeConfig();
    CONFIG_PROPERTY(int, nodeCircleSize)             // Maximum number of taxonomies in bubble chart
    CONFIG_PROPERTY(int, halfPlusSize)               // Number of visible taxonomies by default
    CONFIG_PROPERTY(int, maxNodeRadius)              // Default size of biggest bubble
    CONFIG_PROPERTY(int, edgeStyle)                  // Edge style
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);
};

class ImportDataConfig : public AbstractConfigBlock
{
    Q_OBJECT
public:
    ImportDataConfig();
    CONFIG_PROPERTY(qreal, minBitscore)
    CONFIG_PROPERTY(qreal, topPercent)
    CONFIG_PROPERTY(QString, gi2taxmap)             // Path to GI to Taxonomy mapping file
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
    GETBLOCK(ImportData)
};

extern Config *configuration;

#endif // CONFIG_H
