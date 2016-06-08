#ifndef COLORS_H
#define COLORS_H

#include <QObject>
#include <QFile>
#include <QMap>
#include <QJsonObject>

typedef QMap<qint32, quint32> TaxColorMap;


class TaxColorSrc : public TaxColorMap
{
public:
    virtual quint32 getColor(qint32 tax_id);
};

class Colors : QObject
{
    Q_OBJECT
    QFile file;
    TaxColorMap configuredColors;
    void init();
public:
    Colors();
    void load();
    void save();
    quint32 getColor(qint32 tax_id);
    void setColor(qint32 tax_id, quint32 color);
    quint32 pickColor(qint32 tax_id);
private:
    void toJson(QJsonObject &json) const;
    void fromJson(QJsonObject &json);
public slots:
    void saveColors();
signals:
    colorsChanged(qint32);
};

extern Colors colors;

#endif // COLORS_H
