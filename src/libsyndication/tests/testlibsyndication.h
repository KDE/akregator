#ifndef TEST_TESTLIBSYNDICATION_H
#define TEST_TESTLIBSYNDICATION_H

#include "loader.h"

#include <QObject>

class TestLibSyndication : public QObject
{
    Q_OBJECT
            
    public:
        
        TestLibSyndication(const QString& url);
    
    public slots:
        
        void slotLoadingComplete(LibSyndication::Loader* loader,
                            LibSyndication::FeedPtr feed,
                            LibSyndication::ErrorCode error);
};

#endif // TEST_TESTLIBSYNDICATION_H
