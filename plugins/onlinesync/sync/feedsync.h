#ifndef FEEDSYNC_H
#define FEEDSYNC_H

#include <QObject>

class SubscriptionList;
class KConfigGroup;

namespace feedsync {

class Aggregator;

class FeedSync : public QObject
{
    Q_OBJECT

 public:
    explicit FeedSync( QObject* parent=0 );
    ~FeedSync();
    enum SyncType { Get, Send };

 protected Q_SLOTS:
    void error(const QString& msg);
    void sync();
    void slotLoadDone();
    void slotAddDone();
    void slotRemoveDone();

 private:
    Aggregator * _aggrSend;
    Aggregator * _aggrGet;
    Aggregator * createAggregatorFactory(KConfigGroup configgroup);
    int _syncstep;
    int _loadedAggrCount;
    SubscriptionList * tmp_removelist;
    SubscriptionList * tmp_addlist;
    void log();
};

}

#endif
