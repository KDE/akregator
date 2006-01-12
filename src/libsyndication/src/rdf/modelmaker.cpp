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

#include "literal.h"
#include "model.h"
#include "modelmaker.h"
#include "property.h"
#include "rdfvocab.h"
#include "resource.h"
#include "sequence.h"
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

ResourcePtr ModelMaker::readResource(Model& model, const QDomElement& el)
{
    QString rdfns = RDFVocab::self()->namespaceURI();
    QString about = QString::fromLatin1("about");
    QString resource = QString::fromLatin1("resource");
    QString descriptionStr = QString::fromLatin1("Description");

    ResourcePtr res;

    ResourcePtr type = model.createResource(el.namespaceURI() + el.localName());

    if (*type == *(RDFVocab::self()->seq()))
    {
        SequencePtr seq = model.createSequence(el.attributeNS(rdfns, about));
        res = ResourcePtr::staticCast(seq);
    }
    else
    {
        res = model.createResource(el.attributeNS(rdfns, about));
    }

    model.addStatement(res, RDFVocab::self()->type(), NodePtr::staticCast(type));

    QDomNodeList children = el.childNodes();

    bool isSeq = res->isSequence();
    
    for (uint i = 0; i < children.length(); ++i)
    {
        if (children.item(i).isElement())
        {
            QDomElement ce = children.item(i).toElement();
        
            PropertyPtr pred = model.createProperty(ce.namespaceURI() + ce.localName());
        
            if (ce.hasAttributeNS(rdfns, resource)) // referenced Resource via rdf:resource
            {
                NodePtr obj = NodePtr::staticCast(model.createResource(ce.attributeNS(rdfns, resource)));
                
                if (isSeq && *pred == *(RDFVocab::self()->li()))
                    SequencePtr::staticCast(res)->append(obj);
                else
                    model.addStatement(res, pred, obj);
            }
            else if (!ce.text().isEmpty()) // Literal
            {
                NodePtr obj = NodePtr::staticCast(model.createLiteral(ce.text()));
                
                if (isSeq && *pred == *(RDFVocab::self()->li()))
                    SequencePtr::staticCast(res)->append(obj);
                else
                    model.addStatement(res, pred, obj);
            }
            else if (ce.hasAttributeNS(rdfns, about)) // embedded description
            {
                QString uri = ce.attributeNS(rdfns, about);
                
                // read recursively
                NodePtr obj = NodePtr::staticCast(readResource(model, ce));
                
                if (isSeq && *pred == *(RDFVocab::self()->li()))
                    SequencePtr::staticCast(res)->append(obj);
                else
                    model.addStatement(res, pred, obj);

            }
        
        //TODO: bag, reification (nice to have, but not important for basic RSS 1.0)
        }
    }
    
    return res;
}

} // namespace RDF
} // namespace LibSyndication
