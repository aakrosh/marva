#ifndef BLAST_RECORD
#define BLAST_RECORD
#include <math.h>
#include <QString>
enum BlastFileType
{
    tabular,
    rapsearch,
};

class QStringList;

class BlastRecord
{
public:
    BlastRecord(BlastFileType type, QStringList list);
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
    quint32 taxa_id;
};

#endif // BLAST_RECORD

