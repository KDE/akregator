/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "mk4config.h"
#include "mk4confwidget.h"
#include "storagemk4impl.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

namespace Akregator {
namespace Backend {

MK4ConfWidget::MK4ConfWidget() : MK4ConfWidgetBase()
{
    if (MK4Config::archivePath() == StorageMK4Impl::defaultArchivePath() || MK4Config::archivePath().isEmpty())
    {
        filereq->setURL(StorageMK4Impl::defaultArchivePath());
        MK4Config::setArchivePath(StorageMK4Impl::defaultArchivePath());
        cbUseDefault->setChecked(true);
        filereq->setEnabled(false);
        label->setEnabled(false);
    }
    else
    {
        cbUseDefault->setChecked(false);
        filereq->setEnabled(true);
        label->setEnabled(true);
    }
    filereq->setURL(MK4Config::archivePath());
    connect(cbUseDefault, SIGNAL(toggled(bool)), this, SLOT(slotChkBoxUseDefault(bool)));
    
}

void MK4ConfWidget::accept()
{

    QString path = cbUseDefault->isChecked() ? StorageMK4Impl::defaultArchivePath() : filereq->url();
    if (path != MK4Config::archivePath())
    {
        // TODO: if the user changed the archive location, inform him that
        // the archive is not migrated automatically, but that he has to
        // close Akregator and copy the files over/use some fancy CLI tool not
        // yet written
    }
    MK4Config::setArchivePath(path);
    MK4Config::writeConfig();
    MK4ConfWidgetBase::accept();
}

void MK4ConfWidget::slotChkBoxUseDefault(bool checked)
{
    if (checked)
    {
        filereq->setURL(StorageMK4Impl::defaultArchivePath());
        filereq->setEnabled(false);
    }
    else
    {
        filereq->setEnabled(true);
    }
}


}
}

#include "mk4confwidget.moc"
