/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef AKREGATORTEXTTOSPEECH_H
#define AKREGATORTEXTTOSPEECH_H

#include <QObject>
#include "kpimtextedit/texttospeech.h"
#include "kpimtextedit/texttospeechactions.h"
class QAction;

namespace Akregator
{
class AkregatorTextToSpeechInterface;
class MainWidget;
class AkregatorTextToSpeech : public QObject
{
    Q_OBJECT
public:
    explicit AkregatorTextToSpeech(QObject *parent = Q_NULLPTR);
    ~AkregatorTextToSpeech();

    QAction *stopAction() const;
    QAction *playPauseAction() const;

    void setMainWindow(Akregator::MainWidget *mainWidget);
public Q_SLOTS:
    void stateChanged(KPIMTextEdit::TextToSpeechWidget::State state);

private Q_SLOTS:
    void slotStateChanged(KPIMTextEdit::TextToSpeech::State state);

private:
    KPIMTextEdit::TextToSpeechActions *mTextToSpeechActions;
    AkregatorTextToSpeechInterface *mSpeechInterface;
    Akregator::MainWidget *mMainWindow;
};
}

#endif // AKREGATORTEXTTOSPEECH_H
