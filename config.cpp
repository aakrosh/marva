#include "config.h"

#include <QJsonObject>
#include <QMessageBox>

Config *configuration;

//=========================================================================
BubbleChartConfig::BubbleChartConfig():
    AbstractConfigBlock("BubbleChart")
{
    INITPROPERTY(maxChartTaxes, 120);
    INITPROPERTY(defaultVisibleChartTaxes, 40);
    INITPROPERTY(defaultMaxBubbleSize, 60);
}

//=========================================================================
void BubbleChartConfig::toJson(QJsonObject &json) const
{
    QJsonObject jBubbleChart;
    TOJSON(jBubbleChart, maxChartTaxes);
    TOJSON(jBubbleChart, defaultVisibleChartTaxes);
    TOJSON(jBubbleChart, defaultMaxBubbleSize);
    json["BubbleChart"] = jBubbleChart;
}

//=========================================================================
void BubbleChartConfig::fromJson(QJsonObject &json)
{
    try
    {
        QJsonObject jBubbleChart = json["BubbleChart"].toObject();
        INTFROMJSON(jBubbleChart, maxChartTaxes)
        INTFROMJSON(jBubbleChart, defaultVisibleChartTaxes)
        INTFROMJSON(jBubbleChart, defaultMaxBubbleSize)
    }
    catch(...)
    {
        QMessageBox::warning(NULL, "Cannot restore config", "Cannot restore bubble chart configuration");
    }
}

//=========================================================================
GraphNodeConfig::GraphNodeConfig():
    AbstractConfigBlock("GraphNode")
{
    INITPROPERTY(nodeCircleSize, 8);
    INITPROPERTY(halfPlusSize, 3);
    INITPROPERTY(maxNodeRadius, 30);
}

//=========================================================================
void GraphNodeConfig::toJson(QJsonObject &json) const
{
    QJsonObject jBubbleChart;
    TOJSON(jBubbleChart, nodeCircleSize);
    TOJSON(jBubbleChart, halfPlusSize);
    TOJSON(jBubbleChart, maxNodeRadius);
    json["GraphNode"] = jBubbleChart;
}

//=========================================================================
void GraphNodeConfig::fromJson(QJsonObject &json)
{
    try
    {
        QJsonObject jBubbleChart = json["BubbleChart"].toObject();
        INTFROMJSON(jBubbleChart, nodeCircleSize)
        INTFROMJSON(jBubbleChart, halfPlusSize)
        INTFROMJSON(jBubbleChart, maxNodeRadius)
    }
    catch(...)
    {
        QMessageBox::warning(NULL, "Cannot restore config", "Cannot restore bubble chart configuration");
    }
}

//=========================================================================
PathsConfig::PathsConfig():
    AbstractConfigBlock("Paths")
{
    INITPROPERTY(gi2taxmap, "");
}

//=========================================================================
void PathsConfig::toJson(QJsonObject &json) const
{
    QJsonObject jPaths;
    TOJSON(jPaths, gi2taxmap);
    json["Paths"] = jPaths;
}

//=========================================================================
void PathsConfig::fromJson(QJsonObject &json)
{
    try
    {
        QJsonObject jPaths = json["Paths"].toObject();
        STRFROMJSON(jPaths, gi2taxmap)
    }
    catch(...)
    {
        QMessageBox::warning(NULL, "Cannot restore config", "Cannot restore pathes from configuration");
    }
}

//=========================================================================
Config::Config(QObject *parent):
    AbstractConfigFile("cfg", "configuration", parent)
{
    ADDBLOCK(BubbleChart);
    ADDBLOCK(GraphNode);
    ADDBLOCK(Paths);
}

//=========================================================================
void Config::init()
{    
}



