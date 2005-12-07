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

#include <ksharedptr.h>

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

    Image();
    Image(const Image& other);
    virtual ~Image();

    Image& operator=(const Image& other);
    bool operator==(const Image& other) const;

    bool isNull() const;

    /**
     * the URL of a GIF, JPEG or PNG image 
     *
     * @return TODO
     */
    QString url() const;

    /** 
     * Describes the image, can be used in the ALT attribute of the 
     * HTML <img> tag when the channel is rendered in HTML.
     *
     * @return TODO
     */
    QString title() const;

    /**
     * The URL of the site, when the channel is rendered, the image should be
     * a link to the site.
     *
     * @return TODO
     */
    QString link() const;

    /**
     * The width of the image. If the feed itself doesn't specify a width,
     * this method returns 88, the default value.
     *
     * @return image width in pixels.
     */
    int width() const;

    /**
     * The height of the image. If the feed itself doesn't specify a height,
     * this method returns 31, the default value.
     *
     * @return image height in pixels.
     */
    int height() const;

    /**
     * optional text that can be included in the TITLE attribute of the link
     * formed around the image in HTML rendering.
     *
     * @return TODO
     */
    QString description() const;

    /**
     * Returns a description of the object for debugging purposes.
     *
     * @return debug string
     */
    QString debugInfo() const;

    private:

    Image(const QString& url, const QString& title, const QString& link, const QString& description, int width, int height);

    static Image* m_null;

    class ImagePrivate;
    KSharedPtr<ImagePrivate> d;
};

} // namespace RSS2
}  // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_IMAGE_H
