#include "blast_record.h"
#include "gi2taxmaptxtloader.h"

#include <QStringList>

QMap<quint32, quint32> gi2taxmap;
//=========================================================================
BlastRecord::BlastRecord(BlastFileType type, QString &line, bool short_format)
{
    switch ( type )
    {
        case tabular:
        {
            QStringList list = line.split("\t", QString::SkipEmptyParts);
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
            taxa_id = list[12].toInt();
        }
        break;
        case sequence:
        {
            if ( line[0] != '>' )
                return;
//            if ( gi2taxmap.isEmpty() )
//            {
//                //Gi2TaxMapTxtLoader loader(NULL, &gi2taxmap);
//                //loader.run();
//            }

            QStringList list = line.split(" ", QString::SkipEmptyParts);
            if ( list.size() < 10 )
                throw("Bad allignment file format");
            query_name = list[0];
            if ( !short_format )
            {
                alligment_id = list[2];
                bitscore = list[3].toDouble();
                e_value = list[4].toDouble();
                identity = list[5].toDouble();
                allignment_len = list[6].toUInt();
                mismatch_count = list[7].toUInt();
                gapopens_count = list[8].toUInt();
            }
            quint32 gi = list[2].split('|').at(1).toUInt();
            taxa_id = gi2TaxProvider->get(gi);
        }
        break;
        default:
            throw("Unknown file format");
    }
}
