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
Config::Config(QObject *parent):
    AbstractConfigFile("cfg", "configuration", parent)
{
    ADDBLOCK(BubbleChart);
    ADDBLOCK(GraphNode);
}

//=========================================================================
void Config::init()
{    
}


