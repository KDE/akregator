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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef LIBSYNDICATION_RSS2_IMAGE_H
#define LIBSYNDICATION_RSS2_IMAGE_H

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

class Image
{
    public:

    static const Image& null();

    static Image fromXML(const QDomElement& e);
    QDomElement toXML(QDomDocument document) const;

    Image();
    Image(const QString& url, const QString& title, const QString& link);
    Image(const Image& other);
    virtual ~Image();

    Image& operator=(const Image& other);
    bool operator==(const Image& other) const;

    bool isNull() const;

    void setURL(const QString& url);
    QString url() const;

    void setTitle(const QString& title);
    QString title() const;

    void setLink(const QString& title);
    QString link() const;

    void setWidth(int width);
    int width() const;

    void setHeight(int height);
    int height() const;

    void setDescription(const QString& description);
    QString description() const;

/*
    QString dcTitle() const;
    void setDcTitle(const QString& dcTitle);
    QString dcCreator() const;
    void setDcCreator(const QString& dcCreator);
    QString dcSubject() const;
    void setDcSubject(const QString& dcSubject);
    QString dcDescription() const;
    void setDcDescription(const QString& dcDescription);
    QString dcPublisher() const;
    void setDcPublisher(const QString& dcPublisher);
    QString dcContributor() const;
    void setDcContributor(const QString& dcContributor);
    QDateTime dcDate() const;
    void setDcDate(const QDatetime& dcDate);
    QString dcType() const;
    void setDcType(const QString& dcType);
    QString dcFormat() const;
    void setDcFormat(const QString& dcFormat);
    QString dcIdentifier() const;
    void setDcIdentifier(const QString& dcIdentifier);
    QString dcSource() const;
    void setDcSource(const QString& dcSource);
    QString dcLanguage() const;
    void setDcLanguage(const QString& dcLanguage);
    QString dcRelation() const;
    void setDcRelation(const QString& dcRelation);
    QString dcCoverage() const;
    void setDcCoverage(const QString& dcCoverage);
    QString dcRights() const;
    void setDcRights(const QString& dcRights);
*/

    QString debugInfo() const;

    private:

    static Image* m_null;

    class ImagePrivate;
    ImagePrivate* d;
};

} // namespace RSS2
}  // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_IMAGE_H
