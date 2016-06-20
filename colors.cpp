#include "colors.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"

#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QColorDialog>

#define COLORS_FILE_NAME    "marva.clr"
TaxColorSrc taxColorSrc;

Colors *colors;

Colors::Colors(QObject *parent) : AbstractConfigFile("clr", "colors", parent)
{
}

//=========================================================================
void Colors::init()
{
    configuredColors.clear();
}

//=========================================================================
quint32 Colors::getColor(qint32 tax_id)
{
    if ( configuredColors.contains(tax_id) )
        return configuredColors[tax_id];
    else
        return taxColorSrc.getColor(tax_id);
}

//=========================================================================
void Colors::setColor(qint32 tax_id, quint32 color)
{
    configuredColors[tax_id] = color;

    getTaxNodeSignalSender(taxMap.value(tax_id))->ColorChanged();
    save();
    emit colorsChanged(tax_id);
}

//=========================================================================
quint32 Colors::pickColor(qint32 tax_id)
{
    QColor initialColor = QColor::fromRgb(getColor(tax_id));
    QColor newColor = QColorDialog::getColor(initialColor, NULL, "Choose color");
    if ( !newColor.isValid() )
        return initialColor.rgb();
    setColor(tax_id, newColor.rgb());
    return newColor.rgb();
}

//=========================================================================
void Colors::toJson(QJsonObject &json) const
{
    QJsonArray arr;

    for ( TaxColorMap::ConstIterator it = configuredColors.constBegin();
          it != configuredColors.constEnd(); it++)
    {
        qint32 tax_id = it.key();
        quint32 color = it.value();
        QJsonArray elem;
        elem.append(tax_id);
        elem.append((qint64)color);
        arr.append(elem);
    }
    json["tax_colors"] = arr;
}

//=========================================================================
void Colors::fromJson(QJsonObject &json)
{
    try
    {
        configuredColors.clear();
        QJsonValue arrVal = json["tax_colors"];
        if ( !arrVal.isArray() )
            return;
        QJsonArray arr = json["tax_colors"].toArray();
        for ( int i = 0; i < arr.count(); i++ )
        {
            QJsonArray elem = arr[i].toArray();
            qint32 tax_id  = elem[0].toInt();
            quint32 color = (quint32)elem[1].toDouble();
            configuredColors.insert(tax_id, color);
        }
    }
    catch(...)
    {
      QMessageBox::warning(NULL, "Cannot restore colors", "Cannot restore color configuration");
    }
}

//=========================================================================
quint32 TaxColorSrc::getColor(qint32 tax_id)
{
    iterator it = find(tax_id);
    if ( it == end() )
    {
        quint32 color = ((rand() % 0xFFFF) + ((rand() % 0xFF) << 16)) | 0xFF000000;
        insert(tax_id, color);
        return color;
    }
    else
    {
        return it.value();
    }
}
