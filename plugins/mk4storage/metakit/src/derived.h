// derived.h --
// This is part of Metakit, the homepage is http://www.equi4.com/metakit.html

/** @file
 * Encapsulation of derived view classes
 */

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Declarations in this file

class c4_Cursor; // not defined here
class c4_Sequence; // not defined here

extern c4_Sequence *f4_CreateFilter(c4_Sequence &, c4_Cursor, c4_Cursor);
extern c4_Sequence *f4_CreateSort(c4_Sequence &, c4_Sequence * = nullptr);
extern c4_Sequence *f4_CreateProject(c4_Sequence &, c4_Sequence &, bool, c4_Sequence * = nullptr);

/////////////////////////////////////////////////////////////////////////////

