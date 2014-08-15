/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "searchbar.h"
#include "akregatorconfig.h"
#include "articlematcher.h"
#include "article.h"

#include <kcombobox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <KLocalizedString>


#include <khbox.h>
#include <QIcon>

#include <QLabel>
#include <QList>
#include <QString>
#include <QTimer>
#include <QStandardPaths>


using namespace boost;
using namespace Akregator;
using namespace Akregator::Filters;

namespace Akregator
{

class SearchBar::SearchBarPrivate
{
public:
    QString searchText;
    QTimer timer;
    KLineEdit* searchLine;
    KComboBox* searchCombo;
    int delay;
    std::vector<shared_ptr<const AbstractMatcher> > matchers;
    void triggerTimer() {
        if (timer.isActive())
            timer.stop();

        timer.start(200);
    }
};

SearchBar::SearchBar(QWidget* parent) : KHBox(parent), d(new SearchBar::SearchBarPrivate)
{
    d->delay = 400;
    setMargin(2);
    setSpacing(5);
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );

    QLabel* searchLabel = new QLabel(this);
    searchLabel->setText( i18nc("Title of article searchbar", "S&earch:") );

    d->searchLine = new KLineEdit(this);
    d->searchLine->setClearButtonShown(true);

    connect(d->searchLine, SIGNAL(textChanged(QString)),
                        this, SLOT(slotSearchStringChanged(QString)));

    searchLabel->setBuddy(d->searchLine);

    QLabel* statusLabel = new QLabel(this);
    statusLabel->setText( i18n("Status:") );

    d->searchCombo = new KComboBox(this);

    QIcon iconAll = KIconLoader::global()->loadIcon("system-run", KIconLoader::Small);
    QIcon iconNew(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "akregator/pics/kmmsgnew.png"));
    QIcon iconUnread(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "akregator/pics/kmmsgunseen.png"));
    const QIcon iconKeep = QIcon::fromTheme( "mail-mark-important" );

    d->searchCombo->addItem(iconAll, i18n("All Articles"));
    d->searchCombo->addItem(iconUnread, i18nc("Unread articles filter", "Unread"));
    d->searchCombo->addItem(iconNew, i18nc("New articles filter", "New"));
    d->searchCombo->addItem(iconKeep, i18nc("Important articles filter", "Important"));

    d->searchLine->setToolTip( i18n( "Enter space-separated terms to filter article list" ) );
    d->searchCombo->setToolTip( i18n( "Choose what kind of articles to show in article list" ) );

    connect(d->searchCombo, SIGNAL(activated(int)),
                        this, SLOT(slotSearchComboChanged(int)));

    connect(&(d->timer), SIGNAL(timeout()), this, SLOT(slotActivateSearch()));
    d->timer.setSingleShot(true);
}

SearchBar::~SearchBar()
{
    delete d;
    d = 0;
}

QString SearchBar::text() const
{
    return d->searchText;
}

int SearchBar::status() const
{
    return d->searchCombo->currentIndex();
}

void SearchBar::setDelay(int ms)
{
    d->delay = ms;
}

int SearchBar::delay() const
{
    return d->delay;
}

void SearchBar::slotClearSearch()
{
    if (status() != 0 || !d->searchLine->text().isEmpty())
    {
        d->searchLine->clear();
        d->searchCombo->setCurrentIndex(0);
        d->timer.stop();
        slotActivateSearch();
    }
}

void SearchBar::slotSetStatus(int status)
{
     d->searchCombo->setCurrentIndex(status);
     d->triggerTimer();
}

void SearchBar::slotSetText(const QString& text)
{
    d->searchLine->setText(text);
    d->triggerTimer();
}

void SearchBar::slotSearchComboChanged(int /*index*/)
{
    d->triggerTimer();
}

std::vector<shared_ptr<const AbstractMatcher> > SearchBar::matchers() const {
    return d->matchers;
}


void SearchBar::slotSearchStringChanged(const QString& search)
{
    d->searchText = search;
    d->triggerTimer();
}

void SearchBar::slotActivateSearch()
{
    QList<Criterion> textCriteria;
    QList<Criterion> statusCriteria;

    if (!d->searchText.isEmpty())
    {
        Criterion subjCrit( Criterion::Title, Criterion::Contains, d->searchText);
        textCriteria << subjCrit;
        Criterion crit1( Criterion::Description, Criterion::Contains, d->searchText);
        textCriteria << crit1;
        Criterion authCrit( Criterion::Author, Criterion::Contains, d->searchText);
        textCriteria << authCrit;
    }

    if (d->searchCombo->currentIndex())
    {
        switch (d->searchCombo->currentIndex())
        {
            case 1: // Unread
            {
                Criterion crit1( Criterion::Status, Criterion::Equals, New);
                Criterion crit2( Criterion::Status, Criterion::Equals, Unread);
                statusCriteria << crit1;
                statusCriteria << crit2;
                break;
            }
            case 2: // New
            {
                Criterion crit( Criterion::Status, Criterion::Equals, New);
                statusCriteria << crit;
                break;
            }
            case 3: // Keep flag set
            {
                Criterion crit( Criterion::KeepFlag, Criterion::Equals, true);
                statusCriteria << crit;
                break;
            }
            default:
                break;
        }
    }

    std::vector<boost::shared_ptr<const AbstractMatcher> > matchers;

    matchers.push_back( boost::shared_ptr<const AbstractMatcher>( new ArticleMatcher( textCriteria, ArticleMatcher::LogicalOr ) ) );
    matchers.push_back( boost::shared_ptr<const AbstractMatcher>( new ArticleMatcher( statusCriteria, ArticleMatcher::LogicalOr ) ) );
    Settings::setStatusFilter( d->searchCombo->currentIndex() );
    Settings::setTextFilter( d->searchText );
    d->matchers = matchers;
    emit signalSearch( matchers );
}

}

