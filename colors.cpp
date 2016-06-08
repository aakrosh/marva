#include "colors.h"
#include "taxdataprovider.h"
#include "taxnodesignalsender.h"

#include <QDir>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QColorDialog>

#define COLORS_FILE_NAME    "marva.clr"
TaxColorSrc taxColorSrc;

Colors colors;

Colors::Colors()
{
    QString marvaPath = QDir::homePath()+"/Marva";
    QDir(marvaPath).mkpath(".");
    file.setFileName(QDir(marvaPath).filePath(COLORS_FILE_NAME));
    if ( file.exists() )
        load();
    else
        init();
    connect(this, SIGNAL(colorsChanged(qint32)), this, SLOT(saveColors()));
}

//=========================================================================
void Colors::load()
{
    if ( !file.open(QIODevice::ReadOnly) )
        return;
    QByteArray saveData = file.readAll();
    file.close();
    QJsonDocument loadDoc = QJsonDocument::fromJson(saveData);
    QJsonObject jobj = loadDoc.object();
    fromJson(jobj);
}

//=========================================================================
void Colors::init()
{
    configuredColors.clear();
}

//=========================================================================
void Colors::save()
{
    if ( !file.open(QIODevice::WriteOnly) )
    {
        QMessageBox::warning(0, "Failed to save history", QString("Cannot open history file %1").arg(file.fileName()));
        return;
    }
    QJsonObject saveObject;
    toJson(saveObject);
    QJsonDocument saveDoc(saveObject);
    file.write(saveDoc.toJson(QJsonDocument::Indented));
    file.close();
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
    configuredColors.clear();
    QJsonArray arr = json["tax_colors"].toArray();
    for ( int i = 0; i < arr.count(); i++ )
    {
        QJsonArray elem = arr[i].toArray();
        qint32 tax_id  = elem[0].toInt();
        quint32 color = (quint32)elem[1].toDouble();
        configuredColors.insert(tax_id, color);
    }
}

//=========================================================================
void Colors::saveColors()
{
    save();
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
