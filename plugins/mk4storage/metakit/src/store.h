// store.h --
// This is part of Metakit, see http://www.equi4.com/metakit.html

/** @file
 * Definition of auxiliary storage management classes
 */

#ifndef __STORE_H__
#define __STORE_H__

/////////////////////////////////////////////////////////////////////////////

class c4_Dependencies
{
    c4_PtrArray _refs;

public:
    c4_Dependencies();
    ~c4_Dependencies();

    void Add(c4_Sequence *seq_);
    bool Remove(c4_Sequence *seq_);

    friend class c4_Notifier;
};

/////////////////////////////////////////////////////////////////////////////

class c4_Notifier
{
    c4_Sequence *_origin;
    c4_Notifier *_chain;
    c4_Notifier *_next;

public:
    enum {
        kNone, kSetAt, kInsertAt, kRemoveAt, kMove, kSet, kLimit
    };

    c4_Notifier(c4_Sequence *origin_);
    ~c4_Notifier();

    bool HasDependents()const;

    void StartSetAt(int index_, c4_Cursor &cursor_);
    void StartInsertAt(int index_, c4_Cursor &cursor_, int count_);
    void StartRemoveAt(int index_, int count_);
    void StartMove(int from_, int to_);
    void StartSet(int index_, int propId_, const c4_Bytes &buf_);

    int _type;
    int _index;
    int _propId;
    int _count;
    c4_Cursor *_cursor;
    const c4_Bytes *_bytes;

private:
    void Notify();
};

/////////////////////////////////////////////////////////////////////////////

class c4_DerivedSeq: public c4_Sequence
{
protected:
    c4_Sequence &_seq;

protected:
    c4_DerivedSeq(c4_Sequence &seq_);
    virtual ~c4_DerivedSeq();

public:
    int RemapIndex(int, const c4_Sequence *)const Q_DECL_OVERRIDE;

    int NumRows()const Q_DECL_OVERRIDE;
    void SetNumRows(int size_) Q_DECL_OVERRIDE;

    int NumHandlers()const Q_DECL_OVERRIDE;
    c4_Handler &NthHandler(int)const Q_DECL_OVERRIDE;
    const c4_Sequence *HandlerContext(int)const Q_DECL_OVERRIDE;
    int AddHandler(c4_Handler *) Q_DECL_OVERRIDE;
    c4_Handler *CreateHandler(const c4_Property &) Q_DECL_OVERRIDE;

    c4_Notifier *PreChange(c4_Notifier &nf_) Q_DECL_OVERRIDE;
};

/////////////////////////////////////////////////////////////////////////////

class c4_StreamStrategy: public c4_Strategy
{
    c4_Stream *_stream;
    t4_byte *_buffer;
    t4_i32 _buflen;
    t4_i32 _position;
public:
    c4_StreamStrategy(t4_i32 buflen_);
    c4_StreamStrategy(c4_Stream *stream_);
    virtual ~c4_StreamStrategy();

    bool IsValid()const Q_DECL_OVERRIDE;
    int DataRead(t4_i32 pos_, void *buffer_, int length_) Q_DECL_OVERRIDE;
    void DataWrite(t4_i32 pos_, const void *buffer_, int length_) Q_DECL_OVERRIDE;
    t4_i32 FileSize() Q_DECL_OVERRIDE;
};

/////////////////////////////////////////////////////////////////////////////

#if q4_INLINE
#include "store.inl"
#endif

/////////////////////////////////////////////////////////////////////////////

#endif
