/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATOR_RUN_H 
#define AKREGATOR_RUN_H

#include <kparts/browserrun.h>

namespace Akregator
{

    class Viewer;
    
    class BrowserRun : public KParts::BrowserRun
{
	Q_OBJECT
    public:
        BrowserRun(Viewer *, QWidget *, KParts::ReadOnlyPart *, const KURL & , const KParts::URLArgs &, bool);
        virtual ~BrowserRun();
    
    protected:
	    virtual void foundMimeType( const QString & _type );

    private:
        bool m_shouldEmbed;
        Viewer *m_viewer; 
};

}

#endif

// vim: set et ts=4 sts=4 sw=4:
