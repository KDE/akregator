/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef APP_H
#define APP_H

#include <kuniqueapplication.h>

#define akreapp (static_cast<Akregator::Application*>(kapp))

namespace Akregator
{

class Application : public KUniqueApplication {
    Q_OBJECT
    public:
        Application();
        ~Application();
        void setHaveWindowLoaded(bool b);
        bool haveWindowLoaded();
    private:
        bool m_haveWindowLoaded;
};

}
#endif

