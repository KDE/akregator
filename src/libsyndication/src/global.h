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
#ifndef LIBSYNDICATION_GLOBAL_H
#define LIBSYNDICATION_GLOBAL_H

#define LIBSYNDICATION_VERSION "0.1"

namespace LibSyndication {

/**
 * error code indicating fetching or parsing errors
 */
enum ErrorCode
{
    Success = 0, /**< No error occurred, feed was fetched and parsed
                  * successfully 
                  */
    Aborted = 1, /**< file downloading/parsing was aborted by the user */
    Timeout = 2, /**< file download timed out */
    
    UnknownHost = 3, /**< The hostname couldn't get resolved to an IP address */
    
    FileNotFound = 4, /**< the host was contacted successfully, but reported a
                       * 404 error 
                       */
    OtherRetrieverError = 5, /**< retriever error not covered by the error codes
                              * above. This is returned if a custom
                              * DataRetriever was used. See the
                              * retriever-specific status byte for more
                              * information on the occurred error. */
    InvalidXml = 6, /**< The XML is invalid. This is returned if no parser
                     * accepts the source and the DOM document can't be parsed.
                     * It is not returned if the source is not valid XML but a
                     * (non-XML) parser accepts it.
                     */
    XmlNotAccepted = 7, /**< The source is valid XML, but no parser accepted the
                         * it.
                         */
    InvalidFormat = 8, /**< the source was accepted by a parser, but the actual
                        * parsing failed. As our parser implementations
                        * currently do not validate the source ("parse what you
                        * can get"), this code will be rarely seen.
                        */
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_GLOBAL_H
