// store.inl --
// This is part of Metakit, the homepage is http://www.equi4.com/metakit.html

/** @file
 * Inlined members of the storage management classes
 */

/////////////////////////////////////////////////////////////////////////////
// c4_Notifier

d4_inline c4_Notifier::c4_Notifier (c4_Sequence* origin_)
  : _origin (origin_), _chain (nullptr), _next (nullptr),
    _type (kNone), _index (0), _propId (0), _count (0), 
    _cursor (nullptr), _bytes (nullptr)
{
  d4_assert(_origin != 0);
}

/////////////////////////////////////////////////////////////////////////////
