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

#include "akregatortexttospeechinterface.h"

using namespace Akregator;

AkregatorTextToSpeechInterface::AkregatorTextToSpeechInterface(QObject *parent)
    : PimCommon::AbstractTextToSpeechInterface(parent)
{
    PimCommon::TextToSpeech::self(); //init
}

AkregatorTextToSpeechInterface::~AkregatorTextToSpeechInterface()
{

}

bool AkregatorTextToSpeechInterface::isReady() const
{
    return PimCommon::TextToSpeech::self()->isReady();
}

void AkregatorTextToSpeechInterface::say(const QString &text)
{
    PimCommon::TextToSpeech::self()->say(text);
}

void AkregatorTextToSpeechInterface::stateChanged(PimCommon::TextToSpeechWidget::State state)
{
    switch (state) {
    case PimCommon::TextToSpeechWidget::Stop:
        PimCommon::TextToSpeech::self()->stop();
        break;
    case PimCommon::TextToSpeechWidget::Play:
        PimCommon::TextToSpeech::self()->resume();
        break;
    case PimCommon::TextToSpeechWidget::Pause:
        PimCommon::TextToSpeech::self()->pause();
        break;
    }
}

