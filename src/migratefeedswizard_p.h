/*
 * This file is part of Akregator
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef AKREGATOR_MIGRATEFEEDSWIZARD_P_H
#define AKREGATOR_MIGRATEFEEDSWIZARD_P_H

#include <QProgressBar>
#include <QWizardPage>

class QComboBox;
class QLabel;
class QStringList;
class QStandardItemModel;

namespace Akregator {

class ImportItemsJob;

class WizardPage : public QWizardPage {
    Q_OBJECT
public:
    explicit WizardPage( QWidget* parent=0 ) : QWizardPage( parent ), m_completed( false ) {}

    void setCompleted( bool completed ) {
        if ( completed == m_completed )
            return;
        m_completed = completed;
        emit completeChanged();
    }

    /* reimp */ bool isComplete() const {
        return m_completed;
    }

private:
    bool m_completed;
};

class StartPage : public QWizardPage {
    Q_OBJECT
public:
    explicit StartPage( const QStringList& resources, QWidget* parent=0 );

    QComboBox* m_resourceBox;
};

class OpmlImportResultPage : public WizardPage {
    Q_OBJECT
public:
    explicit OpmlImportResultPage( QWidget* parent=0 );

    enum State {
        NotStarted,
        Running,
        Finished
    };


    void setState( State state );

    QProgressBar* m_progressBar;
    QLabel* m_resultLabel;
};

class ItemImportResultPage : public WizardPage {
    Q_OBJECT
public:
    explicit ItemImportResultPage( QWidget* parent=0 );

    void connectJob( Akregator::ImportItemsJob* job, const QString& title );

    enum State {
        NotStarted,
        Running,
        Finished
    };

    void setState( State state, int current=0, int total=100 );
    void itemImportFinished( const Akregator::ImportItemsJob* job, const QString& title );

    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    enum Role {
        SortRole=Qt::UserRole
    };
    enum ErrorValue {
        NoError=0,
        Warning,
        Error
    };
    QStandardItemModel* m_model;
};

}

#endif // AKREGATOR_MIGRATEFEEDSWIZARD_P_H
