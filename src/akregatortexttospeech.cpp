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
#include "utils.h"

#include <KLocalizedString>
#include <kcharsets.h>

#include "mainwidget.h"

#include <article.h>
using namespace Akregator;

AkregatorTextToSpeech::AkregatorTextToSpeech(QObject *parent)
    : QObject(parent),
      mTextToSpeechActions(new KPIMTextEdit::TextToSpeechActions(this)),
      mSpeechInterface(new AkregatorTextToSpeechInterface(this)),
      mMainWindow(Q_NULLPTR)
{
    connect(mTextToSpeechActions, &KPIMTextEdit::TextToSpeechActions::stateChanged, this, &AkregatorTextToSpeech::stateChanged);
    connect(KPIMTextEdit::TextToSpeech::self(), &KPIMTextEdit::TextToSpeech::stateChanged, this, &AkregatorTextToSpeech::slotStateChanged);
}

AkregatorTextToSpeech::~AkregatorTextToSpeech()
{

}

void AkregatorTextToSpeech::setMainWindow(Akregator::MainWidget *mainWidget)
{
    mMainWindow = mainWidget;
}

void AkregatorTextToSpeech::stateChanged(KPIMTextEdit::TextToSpeechWidget::State state)
{
    if (mMainWindow) {
        QString speakMe;
        const QVector<Akregator::Article> lstArticle = mMainWindow->speakSelectedArticles();
        for (int i = 0; i < lstArticle.size(); ++i) {
            if (!speakMe.isEmpty()) {
                speakMe += QLatin1String(". . . . . . ") + i18n("Next Article: ");
            }
            const Akregator::Article art = lstArticle.at(i);
            speakMe += KCharsets::resolveEntities(Utils::stripTags((art).title()))
                       + QLatin1String(". . . . ")
                       + KCharsets::resolveEntities(Utils::stripTags((art).description()));

        }
        if (!speakMe.isEmpty()) {
            mSpeechInterface->stateChanged(state);
            mSpeechInterface->say(speakMe);
        }
    }
}

void AkregatorTextToSpeech::slotStateChanged(KPIMTextEdit::TextToSpeech::State state)
{
    switch (state) {
    case KPIMTextEdit::TextToSpeech::Ready: {
        if (state == KPIMTextEdit::TextToSpeech::Ready) {
            mTextToSpeechActions->setState(KPIMTextEdit::TextToSpeechWidget::Stop);
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

