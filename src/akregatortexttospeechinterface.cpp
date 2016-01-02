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

#include "akregatortexttospeechinterface.h"

using namespace Akregator;

AkregatorTextToSpeechInterface::AkregatorTextToSpeechInterface(QObject *parent)
    : KPIMTextEdit::AbstractTextToSpeechInterface(parent)
{
    KPIMTextEdit::TextToSpeech::self(); //init
}

AkregatorTextToSpeechInterface::~AkregatorTextToSpeechInterface()
{

}

bool AkregatorTextToSpeechInterface::isReady() const
{
    return KPIMTextEdit::TextToSpeech::self()->isReady();
}

void AkregatorTextToSpeechInterface::say(const QString &text)
{
    KPIMTextEdit::TextToSpeech::self()->say(text);
}

void AkregatorTextToSpeechInterface::stateChanged(KPIMTextEdit::TextToSpeechWidget::State state)
{
    switch (state) {
    case KPIMTextEdit::TextToSpeechWidget::Stop:
        KPIMTextEdit::TextToSpeech::self()->stop();
        break;
    case KPIMTextEdit::TextToSpeechWidget::Play:
        KPIMTextEdit::TextToSpeech::self()->resume();
        break;
    case KPIMTextEdit::TextToSpeechWidget::Pause:
        KPIMTextEdit::TextToSpeech::self()->pause();
        break;
    }
}

