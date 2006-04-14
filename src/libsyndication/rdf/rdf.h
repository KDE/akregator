/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef LIBSYNDICATION_RDF_RDF_H
#define LIBSYNDICATION_RDF_RDF_H

#include "contentvocab.h"
#include "document.h"
#include "dublincore.h"
#include "dublincorevocab.h"
#include "image.h"
#include "item.h"
#include "literal.h"
#include "model.h"
#include "modelmaker.h"
#include "node.h"
#include "nodevisitor.h"
#include "parser.h"
#include "property.h"
#include "rdfvocab.h"
#include "resource.h"
#include "resourcewrapper.h"
#include "rssvocab.h"
#include "sequence.h"
#include "statement.h"
#include "syndication.h"
#include "syndicationvocab.h"
#include "textinput.h"

namespace LibSyndication {

/** 
 * LibSyndication's parser for the RDF-based 
 * RSS 0.9 and RSS 1.0 formats
 */
namespace RDF {}

}

#endif // LIBSYNDICATION_RDF_RDF_H
