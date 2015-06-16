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

#include "akregatortexttospeech.h"
#include "akregatortexttospeechinterface.h"
#include "pimcommon/texttospeech/texttospeechactions.h"
using namespace Akregator;

AkregatorTextToSpeech::AkregatorTextToSpeech(QObject *parent)
    : QObject(parent),
      mTextToSpeechActions(new PimCommon::TextToSpeechActions(this)),
      mSpeechInterface(new AkregatorTextToSpeechInterface(this))
{
    connect(mTextToSpeechActions, &PimCommon::TextToSpeechActions::stateChanged, mSpeechInterface, &AkregatorTextToSpeechInterface::stateChanged);
    connect(PimCommon::TextToSpeech::self(), &PimCommon::TextToSpeech::stateChanged, this, &AkregatorTextToSpeech::slotStateChanged);
}

AkregatorTextToSpeech::~AkregatorTextToSpeech()
{

}

void AkregatorTextToSpeech::slotStateChanged(PimCommon::TextToSpeech::State state)
{
    switch(state) {
    case PimCommon::TextToSpeech::Ready: {
        if (state == PimCommon::TextToSpeech::Ready) {
            mTextToSpeechActions->setState(PimCommon::TextToSpeechWidget::Stop);
        }
        break;
    }
    default:
        //TODO
        break;
    }
}


QAction *AkregatorTextToSpeech::stopAction() const
{
    return mTextToSpeechActions->stopAction();
}

QAction *AkregatorTextToSpeech::playPauseAction() const
{
    return mTextToSpeechActions->playPauseAction();
}


