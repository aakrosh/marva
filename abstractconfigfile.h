#ifndef ABSTRACTCONFIGFILE_H
#define ABSTRACTCONFIGFILE_H

#include <QObject>
#include <QFile>
#include <QMap>

#define CONFIG_PROPERTY(_TYPE_, _NAME_)         \
private:                                        \
    _TYPE_ _ ## _NAME_;                         \
public:                                         \
    _TYPE_ _NAME_()  {return _ ## _NAME_;}      \
    void set ## _NAME_(_TYPE_ _value_)          \
    {                                           \
        _ ## _NAME_ = _value_;                       \
        emit dataChanged();                     \
    }

#define TOJSON(_JVAR_, _NAME_) _JVAR_[#_NAME_] = _ ## _NAME_;
#define INTFROMJSON(_JVAR_, _NAME_) _ ## _NAME_ = _JVAR_[#_NAME_].toInt();
#define REALFROMJSON(_JVAR_, _NAME_) _ ## _NAME_ = _JVAR_[#_NAME_].toDouble();
#define STRFROMJSON(_JVAR_, _NAME_) _ ## _NAME_ = _JVAR_[#_NAME_].toString();
#define INITPROPERTY(__NAME__, __VAL__) _ ## __NAME__ = __VAL__

class AbstractConfigBlock : public QObject
{
    Q_OBJECT
public:
    AbstractConfigBlock(QString _name) : name(_name){}
    virtual ~AbstractConfigBlock(){}
    QString name;
    virtual void toJson(QJsonObject &json) const = 0;
    virtual void fromJson(QJsonObject &json) = 0;
signals:
    void dataChanged();
};


#define BLOCKCONFIGCLASS_START(__X__) class __X__ ## Config : public AbstractConfigBlock \
{ Q_OBJECT                                                                               \
public:                                                                                  \
__X__ ## Config();                                                                       \
virtual void toJson(QJsonObject &json) const;                                            \
virtual void fromJson(QJsonObject &json);

#define BLOCKCONFIGCLASS_END };

#define GETBLOCK(__X__) __X__ ## Config * __X__() { return (__X__ ## Config *)blocks[#__X__];}
#define ADDBLOCK(__X__) blocks.insert(#__X__, new __X__ ## Config()); \
    connect(blocks.value(#__X__), SIGNAL(dataChanged()), this, SLOT(save()));

class AbstractConfigFile : public QObject
{
    Q_OBJECT
protected:
    QString fileName;
    QString caption;
    QMap<QString, AbstractConfigBlock *> blocks;
public:
    QFile file;
    explicit AbstractConfigFile(QString extension, QString _caption, QObject *parent = 0);
    virtual ~AbstractConfigFile();
    virtual void init();
    virtual void load();
public slots:
    virtual void save();
private:
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);

    template <class  T> friend class AbstractConfigFileFactory;
};

template <class  T> class AbstractConfigFileFactory
{
public:
    static T *create(QObject *parent=0);
};

template <class  T> T *AbstractConfigFileFactory<T>::create(QObject *parent)
{
    T *ptr = new T(parent);
    if ( ptr->file.exists() )
        ptr->load();
    else
        ptr->init();
    return ptr;
}

#endif // ABSTRACTCONFIGFILE_H
