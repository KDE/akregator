#include "document.h"
#include "documentsource.h"
#include "parserregistry.h"
#include "rss2/parser.h"

#include <QByteArray>
#include <QFile>
#include <QString>

#include <iostream>

using namespace LibSyndication;

int main(int argc, char **argv)
{
    ParserRegistry::self()->registerParser(new RSS2::Parser);

    if (argc < 2)
    {
        std::cerr << "filename expected" << std::endl;
        return 1;
    }

    QString filename(argv[1]);
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    if (file.error() != QFile::NoError)
    {
        std::cerr << "Couldn't read file: " << file.errorString().toLocal8Bit().data() << std::endl;
    }

    QByteArray raw = file.readAll();
    file.close();

    DocumentSource src(raw);
    Document* doc = ParserRegistry::self()->parse(src);

    if (doc)
    {
         std::cout << doc->debugInfo().toLocal8Bit().data() << std::endl;
    }
    else
    {
         std::cout << "Couldn't parse document " << argv[1] << std::endl;
         return 1;
    }
}

