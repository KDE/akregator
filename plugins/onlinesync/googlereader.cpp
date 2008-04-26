#include "googlereader.h"

#include <kdebug.h>
#include <QDomDocument>

namespace feedsync
{

GoogleReader::GoogleReader(const KConfigGroup& configgroup) 
{
    kDebug();
    setUser(configgroup.readEntry("Login"));
    setPassword(configgroup.readEntry("Password"));
    _subscriptionList = new SubscriptionList();
    _cursor=0;
}

GoogleReader::~GoogleReader() 
{
    kDebug();
    delete _subscriptionList;
}

void GoogleReader::load() 
{
    kDebug();

    // Now: Authentication

    // Data
    QByteArray data;
    data.append(QString(QString("Email=")+getUser()+QString("&Passwd=")+getPassword()).toUtf8());
    QHttpRequestHeader header("POST", "/accounts/ClientLogin");
    header.setValue("Host", "https://www.google.com");
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(data.length());
    http = new QHttp();
    http->setHost("www.google.com",QHttp::ConnectionModeHttps);
    http->request(header,data);
    connect(http, SIGNAL(done(bool)), this, SLOT(slotAuthenticationDone(bool)));
}

void GoogleReader::add(SubscriptionList * list) 
{
    // End
    if (_cursor==list->count()) {
        _cursor=0;
        // Emit signal
        emit addDone();
        return;
    }
    _cursorList = list;

    QByteArray data;
    // Add new
    if ( (getSubscriptionList()->indexOf(list->getRss(_cursor))<0) && (list->indexOf(list->getRss(_cursor))>_cursor) ) {
        kDebug() << "New";
        data.append(QString(    QString("s=feed/")+list->getRss(_cursor)
                                +QString("&ac=subscribe")
                                +QString("&a=user/-/label/")+list->getCat(_cursor,SubscriptionList::Simple)
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());

    // Add tag
    } else {
        kDebug() << "Add Tag";
        data.append(QString(    QString("s=feed/")+list->getRss(_cursor)
                                +QString("&ac=edit")
                                +QString("&a=user/-/label/")+list->getCat(_cursor,SubscriptionList::Simple)
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());
    }

    QHttpRequestHeader header("POST","http://www.google.com/reader/api/0/subscription/edit");
    header.setValue("Host", "http://www.google.com");
    header.setValue("Cookie", "SID="+getSID());
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(data.length());
    http = new QHttp();
    http->setHost("www.google.com",QHttp::ConnectionModeHttp);
    http->request(header,data);
    connect(http, SIGNAL(done(bool)), this, SLOT(slotAddDone(bool)));
    _cursor++;
}

void GoogleReader::update(SubscriptionList * list) 
{
    kDebug();

    // Emit signal
    emit updateDone();
}

void GoogleReader::remove(SubscriptionList * list) 
{
    // End
    if (_cursor==list->count()) {
        _cursor=0;
        // Emit signal
        emit removeDone();
        return;
    }
    _cursorList = list;

    QByteArray data;
    // Revove
    if ( (getSubscriptionList()->countRss(list->getRss(_cursor))==list->countRss(list->getRss(_cursor))) && (list->indexOf(list->getRss(_cursor))==_cursor) ) {
        kDebug() << "Remove";
        data.append(QString(    QString("s=feed/")+list->getRss(_cursor)
                                +QString("&ac=unsubscribe")
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());

    // Remove tag
    } else {
        kDebug() << "Remove Tag";
        data.append(QString(    QString("s=feed/")+list->getRss(_cursor)
                                +QString("&ac=edit")
                                +QString("&r=user/-/label/")+list->getCat(_cursor,SubscriptionList::Simple)
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());
    }

    QHttpRequestHeader header("POST","http://www.google.com/reader/api/0/subscription/edit");
    header.setValue("Host", "http://www.google.com");
    header.setValue("Cookie", "SID="+getSID());
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(data.length());
    http = new QHttp();
    http->setHost("www.google.com",QHttp::ConnectionModeHttp);
    http->request(header,data);
    connect(http, SIGNAL(done(bool)), this, SLOT(slotRemoveDone(bool)));
    _cursor++;
    kDebug();
}

void GoogleReader::error() 
{
    kDebug();

    _cursor=0;
    // Emit signal
    emit error();
}

// SLOTS

void GoogleReader::slotAddDone(bool error) 
{
    QByteArray m_data = http->readAll();
    QString text(m_data.data());
    kDebug() << text.left(20);
    add(_cursorList);
}

void GoogleReader::slotRemoveDone(bool error) 
{
    QByteArray m_data = http->readAll();
    QString text(m_data.data());
    kDebug() << text.left(20);
    remove(_cursorList);
}

void GoogleReader::slotUpdateDone(bool error) 
{
    kDebug();
}

void GoogleReader::slotTokenDone(bool error) 
{
    kDebug();

    // Now: read token

    QByteArray m_data = http->readAll();
    QString text(m_data.data());
    kDebug() << "Token:" << text.left(20);
    setToken(text);

    // Load finished

    emit loadDone();
}

void GoogleReader::slotListDone(bool error) 
{
    kDebug();

    // Now: read the subscription list

    QByteArray m_data = http->readAll();
    QString text(m_data.data());
    QDomDocument doc("googlereader");
    doc.setContent(text);
    QDomNode nodeList = doc.documentElement().firstChild().firstChild();
    while(!nodeList.isNull()) {
        QDomNode nodeSub;
        nodeSub = nodeList.firstChild();
        QString m_rss = nodeSub.firstChild().toText().data();
        m_rss = m_rss.right(m_rss.length()-5);
        nodeSub = nodeSub.nextSibling();
        QString m_name = nodeSub.firstChild().toText().data();
        nodeSub = nodeSub.nextSibling();
        nodeSub = nodeSub.nextSibling();

        // 
        QDomNode nodeCat = nodeSub.firstChild();
        while(!nodeCat.isNull()) {
            QString m_cat = nodeCat.firstChild().nextSibling().firstChild().toText().data();
            // add to sub list
            _subscriptionList->add(m_rss,m_name,m_cat);
            nodeCat = nodeCat.nextSibling();
        }

        // next sub
        nodeList = nodeList.nextSibling();
    }

    // Next: get the token

    QHttpRequestHeader header("GET", QString("http://www.google.com/reader/api/0/token?client=contact:")+getUser());
    header.setValue("Host", "http://www.google.com");
    header.setValue("Cookie", "SID="+getSID());
    http = new QHttp();
    http->setHost("www.google.com");
    http->request(header);
    connect(http, SIGNAL(done(bool)), this, SLOT(slotTokenDone(bool)));
}

void GoogleReader::slotAuthenticationDone(bool error) 
{
    kDebug();

    // Now: read authentication SID

    QByteArray m_data = http->readAll();
    QString text(m_data.data());
    text = text.right(text.length()-text.indexOf("SID=")-4);
    _sid = text.left(text.indexOf("\n"));
    kDebug() << "SID:" << _sid.left(10)+QString("...");

    // Next: get the list

    QHttpRequestHeader header("GET", "http://www.google.com/reader/api/0/subscription/list");
    header.setValue("Host", "http://www.google.com");
    header.setValue("Cookie", "SID="+getSID());
    http = new QHttp();
    http->setHost("www.google.com");
    http->request(header);
    connect(http, SIGNAL(done(bool)), this, SLOT(slotListDone(bool)));
}

// Getters/Setters

QString GoogleReader::getUser() const 
{
    return _user;
}

QString GoogleReader::getPassword() const 
{
    return _password;
}

SubscriptionList * GoogleReader::getSubscriptionList() const 
{
    return _subscriptionList;
}

void GoogleReader::setUser(const QString& user) 
{
    _user=user;
}

void GoogleReader::setPassword(const QString& password) 
{
    _password=password;
}

QString GoogleReader::getSID() const 
{
    return _sid;
}

void GoogleReader::setToken(const QString& token) 
{
    _token=token;
}

QString GoogleReader::getToken() const 
{
    return _token;
}

}
