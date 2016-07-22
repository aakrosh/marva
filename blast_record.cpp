#include "blast_record.h"

#include <QStringList>

//=========================================================================
BlastRecord::BlastRecord(BlastFileType type, QStringList list, bool short_format)
{
    switch ( type )
    {
        case tabular:
            if ( list.size() < 13 )
                throw("Bad tabular BLAST file");
            query_name = list[0];         //Ignore them for now
            if ( !short_format )
            {
                alligment_id = list[1];
                identity = list[2].toDouble();
                allignment_len = list[3].toUInt();
                mismatch_count = list[4].toUInt();
                gapopens_count = list[5].toUInt();
                query_start = list[6].toUInt();
                query_end = list[7].toUInt();
                ref_start = list[8].toULongLong();
                ref_end = list[9].toULongLong();
                e_value = list[10].toDouble();
                bitscore = list[11].toDouble();
            }
            taxa_id = list[12].toUInt();
            break;
        default:
            throw("Unknown file format");
    }
}
