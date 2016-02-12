#include "blast_data.h"
#include <QFile>
#include <QTextStream>

BlastData::BlastData(BlastFileType type, QString &fileName)
{
    switch ( type )
    {
        case tabular:
        {
            QFile file(fileName);
            QTextStream in(&file);
            QString line;
            if ( file.open(QIODevice::ReadOnly|QIODevice::Text) )
            {
                do
                {
                    line = in.readLine();
                    if ( line == NULL || line.isEmpty() )
                        continue;
                    QStringList list = line.split("\t", QString::SkipEmptyParts);
                    append(BlastRecord(type, list));
                }
                while (!line.isNull());
            }
            file.close();
        }
        break;
        default:
        {
            throw("Unknow file format");
        }
    }
}
