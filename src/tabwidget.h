/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <ktabwidget.h>
#include <qdict.h>

namespace Akregator
{

    class TabWidget:public KTabWidget
    {
        Q_OBJECT public:
            TabWidget(QWidget * parent = 0, const char *name = 0);
            ~TabWidget();

            unsigned int tabBarWidthForMaxChars( uint maxLength );
            void setTitle( const QString &title , QWidget* sender);

            private:
                QDict <QWidget>m_titleDict;
                unsigned int m_CurrentMaxLength;
                QWidget *currentItem;
            private slots:
                void slotDetachTab();
                void slotCloseTab();
                void contextMenu (int item, const QPoint &p);
    };
}

#endif
