/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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
#include "pimcommon/texttospeech/texttospeech.h"
#include "pimcommon/texttospeech/texttospeechactions.h"
#include "akregator_export.h"
class QAction;
namespace PimCommon
{
class TextToSpeechActions;
}

namespace Akregator
{
class AkregatorTextToSpeechInterface;
class AKREGATOR_EXPORT AkregatorTextToSpeech : public QObject
{
    Q_OBJECT
public:
    explicit AkregatorTextToSpeech(QObject *parent = Q_NULLPTR);
    ~AkregatorTextToSpeech();

    QAction *stopAction() const;
    QAction *playPauseAction() const;

public Q_SLOTS:
    void stateChanged(PimCommon::TextToSpeechWidget::State state);

private Q_SLOTS:
    void slotStateChanged(PimCommon::TextToSpeech::State state);

private:
    PimCommon::TextToSpeechActions *mTextToSpeechActions;
    AkregatorTextToSpeechInterface *mSpeechInterface;
};
}

#endif // AKREGATORTEXTTOSPEECH_H
