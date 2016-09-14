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
    INITPROPERTY(bubbleSizeCalculationMethod, METHOD_LINEAR);
}

//=========================================================================
void BubbleChartConfig::toJson(QJsonObject &json) const
{
    QJsonObject jBubbleChart;
    TOJSON(jBubbleChart, maxChartTaxes);
    TOJSON(jBubbleChart, defaultVisibleChartTaxes);
    TOJSON(jBubbleChart, defaultMaxBubbleSize);
    TOJSON(jBubbleChart, bubbleSizeCalculationMethod);
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
        INTFROMJSON(jBubbleChart, bubbleSizeCalculationMethod)
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
    INITPROPERTY(edgeStyle, EDGE_CURVE);
    INITPROPERTY(showTitle, SHOW_TITLE_MAIN);
    INITPROPERTY(nodeTitleLen, 20);
}

//=========================================================================
void GraphNodeConfig::toJson(QJsonObject &json) const
{
    QJsonObject jGNode;
    TOJSON(jGNode, nodeCircleSize);
    TOJSON(jGNode, halfPlusSize);
    TOJSON(jGNode, maxNodeRadius);
    TOJSON(jGNode, edgeStyle);
    TOJSON(jGNode, showTitle);
    TOJSON(jGNode, nodeTitleLen);
    json["GraphNode"] = jGNode;
}

//=========================================================================
void GraphNodeConfig::fromJson(QJsonObject &json)
{
    try
    {
        QJsonObject jGNode = json["GraphNode"].toObject();
        INTFROMJSON(jGNode, nodeCircleSize)
        INTFROMJSON(jGNode, halfPlusSize)
        INTFROMJSON(jGNode, maxNodeRadius)
        INTFROMJSON(jGNode, edgeStyle)
        INTFROMJSON(jGNode, showTitle)
        INTFROMJSON(jGNode, nodeTitleLen)
    }
    catch(...)
    {
        QMessageBox::warning(NULL, "Cannot restore config", "Cannot restore bubble chart configuration");
    }
}

//=========================================================================
ImportDataConfig::ImportDataConfig():
    AbstractConfigBlock("TreeView")
{
    INITPROPERTY(minBitscore, 50);
    INITPROPERTY(topPercent, 10);
    INITPROPERTY(maxEValue, 0.01);
    INITPROPERTY(gi2taxmap, "");
}

//=========================================================================
void ImportDataConfig::toJson(QJsonObject &json) const
{
    QJsonObject jImportData;
    TOJSON(jImportData, minBitscore)
    TOJSON(jImportData, topPercent)
    TOJSON(jImportData, maxEValue)
    TOJSON(jImportData, gi2taxmap)
    json["ImportData"] = jImportData;
}

//=========================================================================
void ImportDataConfig::fromJson(QJsonObject &json)
{
    try
    {
        QJsonObject jImportData = json["ImportData"].toObject();
        REALFROMJSON(jImportData, minBitscore)
        REALFROMJSON(jImportData, topPercent)
        REALFROMJSON(jImportData, maxEValue)
        STRFROMJSON(jImportData, gi2taxmap)
    }
    catch(...)
    {
        QMessageBox::warning(NULL, "Cannot restore config", "Cannot restore Tree View Configuration from configuration");
    }
}

//=========================================================================
Config::Config(QObject *parent):
    AbstractConfigFile("cfg", "configuration", parent)
{
    ADDBLOCK(BubbleChart);
    ADDBLOCK(GraphNode);
    ADDBLOCK(ImportData)
}

//=========================================================================
void Config::init()
{    
}





