/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "app.h"

using namespace Akregator;

Application::Application(): KUniqueApplication(), m_haveWindowLoaded(false)
{
}

Application::~Application()
{

}

void Application::setHaveWindowLoaded(bool b)
{
    m_haveWindowLoaded=b;
}

bool Application::haveWindowLoaded()
{
    return m_haveWindowLoaded;
}

#include "app.moc"
