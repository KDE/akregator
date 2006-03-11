#!/usr/bin/ruby

# This file is part of libsyndication
#
# Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; see the file COPYING.LIB.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

SUBDIRS = [ 'rss2', 'rdf', 'atom' ]

numTotal = 0
numErrors = 0

files = Array.new

SUBDIRS.each do |dir|
    Dir.foreach(dir) do |i|
        files.push(dir + "/" + i) if i =~ /.*\.xml\Z/
    end
end

files.each do |file|
    expectedfn = file + ".expected"
    if File.exist?(expectedfn)
        expFile = File.open(expectedfn, "r")
        expected = expFile.read
        expFile.close
        
        system("./testlibsyndication #{file} > testfeeds-output.tmp")
        actFile = File.open("testfeeds-output.tmp")
        actual = actFile.read
        actFile.close
        
        numTotal += 1
        if actual != expected
            puts "#{file} parsed incorrectly."
            # TODO: add diff to log
            numErrors += 1
        end
        
    end
end
# TODO print more verbose output
exit(numErrors)

