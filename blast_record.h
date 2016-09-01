#ifndef BLAST_RECORD
#define BLAST_RECORD

#include <QString>
#include <QHash>

enum BlastFileType
{
    tabular     = 0,
    sequence    = 1,
    last,
    rapsearch,
};

class QStringList;

class BlastRecord
{
public:
    BlastRecord(){}
    BlastRecord(BlastFileType type, QString &line, bool short_format=true);
    QString query_name;
    QString alligment_id;
    qreal identity;
    quint32 allignment_len;
    quint32 mismatch_count;
    quint32 gapopens_count;
    quint32 query_start;
    quint32 query_end;
    quint64 ref_start;
    quint64 ref_end;
    qreal e_value;
    qreal bitscore;
    qint32 taxa_id;
    bool parse(BlastFileType type, QString &line, BlastRecord &rec);
};

extern QHash<quint32, quint32> gi2taxmap;

#endif // BLAST_RECORD

