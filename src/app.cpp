#include "app.h"


aKregatorApp::aKregatorApp(): KApplication(), m_haveWindowLoaded(false)
{
}

aKregatorApp::~aKregatorApp()
{

}

void aKregatorApp::setHaveWindowLoaded(bool b)
{
    m_haveWindowLoaded=b;
}

bool aKregatorApp::haveWindowLoaded()
{
    return m_haveWindowLoaded;
}

#include "app.moc"
