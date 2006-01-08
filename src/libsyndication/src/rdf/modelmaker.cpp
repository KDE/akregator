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

#include "model.h"
#include "modelmaker.h"
#include "property.h"
#include "literal.h"
#include "rdfvocab.h"
#include "resource.h"
#include "statement.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace RDF {

     

ModelMaker::~ModelMaker()
{
}

Model ModelMaker::createFromXML(const QDomDocument& doc)
{
    Model model;

    if (doc.isNull())
        return model;

    QDomElement rdfNode = doc.documentElement();

    QDomNodeList list = rdfNode.childNodes();

    for (uint i = 0; i < list.length(); ++i)
    {
        if (list.item(i).isElement())
        {
            QDomElement el = list.item(i).toElement();
            readResource(model, el);
        }
    }

    return model;
}

void ModelMaker::readResource(Model& model, const QDomElement& el)
{
    QString about = QString::fromLatin1("about");
    QString resource = QString::fromLatin1("resource");
    QString descriptionStr = QString::fromLatin1("Description");

    Resource res(el.attributeNS(RDFVocab::namespaceURI(), about), model); 

    if (el.localName() != descriptionStr || el.namespaceURI() != RDFVocab::namespaceURI())
    {
        Resource type(el.namespaceURI() + el.localName(), model);
        model.addStatement(res, RDFVocab::type(), type);
    }

    QDomNodeList children = el.childNodes();

    for (uint i = 0; i < children.length(); ++i)
    {
        if (children.item(i).isElement())
        {
            QDomElement ce = children.item(i).toElement();
        
            Property pred(ce.namespaceURI() + ce.localName());
        
            if (ce.hasAttributeNS(RDFVocab::namespaceURI(), resource)) // referenced Resource via rdf:resource
            {
                Resource obj(ce.attributeNS(RDFVocab::namespaceURI(), resource), model);
                model.addStatement(res, pred, obj);
            }
            else if (ce.hasAttributeNS(RDFVocab::namespaceURI(), about)) // embedded description using rdf:about
            {
                Resource obj(ce.attributeNS(RDFVocab::namespaceURI(), about), model);
                model.addStatement(res, pred, obj);
                readResource(model, ce); // read recursively
            }
            else if (!ce.text().isEmpty()) // Literal
            {
                Literal obj(ce.text());
                model.addStatement(res, pred, obj);
            }
        
        //TODO: sequence [and bag, anonymous, reification, nice to have, but not important for basic RSS 1.0]
        }
    }
}

} // namespace RDF
} // namespace LibSyndication
