#ifndef COLORS_H
#define COLORS_H

#include <QObject>
#include <QMap>
#include <QJsonObject>

#include "abstractconfigfile.h"

typedef QMap<qint32, quint32> TaxColorMap;


class TaxColorSrc : public TaxColorMap
{
public:
    virtual quint32 getColor(qint32 tax_id);
};

class Colors : public AbstractConfigFile
{
    Q_OBJECT
    TaxColorMap configuredColors;
public:
    Colors(QObject *parent =0);
    quint32 getColor(qint32 tax_id);
    void setColor(qint32 tax_id, quint32 color);
    quint32 pickColor(qint32 tax_id);
    virtual void init();
private:
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);

signals:
    void colorsChanged(qint32);
};

extern Colors *colors;

#endif // COLORS_H
