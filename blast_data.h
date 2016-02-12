#ifndef BLAST_DATA
#define BLAST_DATA

#include "blast_record.h"
#include "QList"


class BlastData : public QList<BlastRecord>
{
public:
    BlastData(BlastFileType type, QString &fileName);
};
#endif // BLAST_DATA

