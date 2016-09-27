#include "blast_record.h"
#include "gi2taxmaptxtloader.h"

#include <QStringList>

QHash<quint32, quint32> gi2taxmap;
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
            }
            bitscore = list[11].toDouble();
            taxa_id = list[12].toInt();
        }
        break;
        case sequence:
        {
            if ( line[0] != '>' )
                return;
            QStringList list = line.split(" ", QString::SkipEmptyParts);
            if ( list.size() < 10 )
                throw("Bad allignment file format");
            query_name = list[0];
            if ( !short_format )
            {
                alligment_id = list[2];
                e_value = exp(list[4].midRef(13).toDouble());
                identity = list[5].midRef(9).toDouble();
                allignment_len = list[6].midRef(8).toUInt();
                mismatch_count = list[7].midRef(9).toUInt();
                gapopens_count = list[8].midRef(13).toUInt();
                query_start = 0;
                query_end = 0;
                ref_start = 0;
                ref_end = 0;
            }
            bitscore = list[3].midRef(5).toDouble();
            quint32 gi = list[2].split('|').at(1).toUInt();
            taxa_id = gi2TaxProvider->get(gi);
        }
        break;
        default:
            throw("Unknown file format");
    }
}

//=========================================================================
bool BlastRecord::parse(BlastFileType type, QString &line, BlastRecord &rec)
{
    switch ( type )
    {
        case tabular:
        {
            QStringList list = line.split("\t", QString::SkipEmptyParts);
            if ( list.size() < 13 )
                throw("Bad tabular BLAST file");
            rec.query_name.swap(list[0]);         //Ignore them for now
            if ( false )
            {
                rec.alligment_id = list[1];
                rec.identity = list[2].toDouble();
                rec.allignment_len = list[3].toUInt();
                rec.mismatch_count = list[4].toUInt();
                rec.gapopens_count = list[5].toUInt();
                rec.query_start = list[6].toUInt();
                rec.query_end = list[7].toUInt();
                rec.ref_start = list[8].toULongLong();
                rec.ref_end = list[9].toULongLong();
            }
            rec.e_value = list[10].toDouble();
            rec.bitscore = list[11].toDouble();
            rec.taxa_id = list[12].toInt();
        }
        break;
        case sequence:
        {
            if ( line[0] != '>' )
                return false;
            QStringList list = line.split(" ", QString::SkipEmptyParts);
            if ( list.size() < 10 )
                throw("Bad allignment file format");
            rec.query_name.swap(list[0]);
            if ( false )
            {
                rec.alligment_id = list[2];
                rec.e_value = list[4].midRef(13).toDouble();
                rec.identity = list[5].midRef(9).toDouble();
                rec.allignment_len = list[6].midRef(8).toUInt();
                rec.mismatch_count = list[7].midRef(9).toUInt();
                rec.gapopens_count = list[8].midRef(13).toUInt();
                rec.query_start = 0;
                rec.query_end = 0;
                rec.ref_start = 0;
                rec.ref_end = 0;
            }
            rec.bitscore = list[3].midRef(5).toDouble();
            quint32 gi = list[2].split('|').at(1).toUInt();
            rec.taxa_id = gi2TaxProvider->get(gi);
        }
        break;
        default:
            throw("Unknown file format");
    }
    return true;
}
