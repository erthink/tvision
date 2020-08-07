/*------------------------------------------------------------------------*/
/* filename - drivers.cpp                                                 */
/*                                                                        */
/* function(s)                                                            */
/*        moveBuf  --   moves a buffer of char/attribute pairs            */
/*        moveChar --   sets a buffer with a char/attribute pair          */
/*        moveCStr --   moves a char array into a buffer & adds an        */
/*                      attribute to each char                            */
/*------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TDrawBuffer
#define Uses_TScreen
#include <tvision/tv.h>

#if !defined( __DOS_H )
#include <dos.h>
#endif  // __DOS_H

#ifndef __BORLANDC__
#include <cstring>
#define register
#endif

#pragma warn -asc

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveBuf                                                  */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent - character position within the buffer where the data      */
/*               is to go                                                 */
/*                                                                        */
/*      source - far pointer to an array of character/attribute pairs     */
/*                                                                        */
/*      attr   - attribute to be used for all characters (0 to retain     */
/*               the attribute from 'source')                             */
/*                                                                        */
/*      count   - number of character/attribute pairs to move             */
/*                                                                        */
/*  Comments:                                                             */
/*                                                                        */
/*      'source' is actually treated like a string...                     */
/*                                                                        */
/*------------------------------------------------------------------------*/

void TDrawBuffer::moveBuf( ushort indent, const void _FAR *source,
                           ushort attr, ushort count )

{
#if !defined( __FLAT__ )

I   MOV     CX, count
I   JCXZ    __5
I   PUSH    DS

    _ES = FP_SEG( &data[indent] );
    _DI = FP_OFF( &data[indent] );

//    _DS = FP_SEG( source );
//    _SI = FP_OFF( source );
I   LDS     SI, source

I   MOV     AH, [BYTE PTR attr]
I   CLD
I   OR      AH, AH
I   JE      __3

__1:

I   LODSB
I   STOSW
I   LOOP    __1
I   JMP     __4

__2:

I   INC     DI

__3:

I   MOVSB
I   LOOP    __2

__4:

I   POP     DS

__5:
        ;
#elif defined(__BORLANDC__)

    register ushort *dest = &data[indent];
    ushort *limit = &data[dataLength];
    register uchar _FAR *s = (uchar _FAR *)source;

    if (attr != 0)
        for (; dest < limit && count; --count, ++s, ++dest)
        {
            ((uchar*)dest)[0] = *s;
            ((uchar*)dest)[1] = (uchar)attr;
        }
    else
        while (dest < limit && count--)
            *(uchar *)dest++ = *s++;

#else
    return moveStr(indent, TStringView {(const char*) source, count}, attr);
#endif
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveChar                                                 */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      c       - character to be put into the buffer                     */
/*                                                                        */
/*      attr    - attribute to be put into the buffer                     */
/*                                                                        */
/*      count   - number of character/attribute pairs to put into the     */
/*                buffer                                                  */
/*                                                                        */
/*------------------------------------------------------------------------*/

void TDrawBuffer::moveChar( ushort indent, char c, ushort attr, ushort count )
{
#if !defined( __FLAT__ )
I   MOV     CX,count
I   JCXZ    __4

    _ES = FP_SEG( &data[indent] );
    _DI = FP_OFF( &data[indent] );

I   MOV     AL,c
I   MOV     AH,[BYTE PTR attr]
I   CLD
I   OR      AL,AL
I   JE      __1
I   OR      AH,AH
I   JE      __3
I   REP     STOSW
I   JMP     __4

__1:

I   MOV     AL,AH

__2:

I   INC     DI

__3:

I   STOSB
I   LOOP    __2

__4:
    ;

#elif defined(__BORLANDC__)
    register ushort *dest = &data[indent];
    ushort *limit = &data[dataLength];

    if (attr != 0)
        for (; dest < limit && count; --count, ++dest)
            {
            if (c) ((uchar*)dest)[0] = c;
            ((uchar*)dest)[1] = (uchar)attr;
            }
    else
        while (count--)
            *(uchar *)dest++ = c;

#else
    TScreenCell *dest = &data[indent];
    TScreenCell *limit = &data[dataLength];

    if (attr)
        for (; dest < limit && count; --count, ++dest)
        {
            auto cell = *dest;
            if (c)
            {
                cell.Char = (uchar) c;
                cell.extraWidth = 0;
            }
            cell.Attr = (uchar) attr;
            *dest = cell;
        }
    else
        while (dest < limit && count--)
        {
            auto cell = *dest;
            cell.Char = (uchar) c;
            cell.extraWidth = 0;
            *dest++ = cell;
        }
#endif
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveCStr                                                 */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      str     - pointer to a 0-terminated string of characters to       */
/*                be moved into the buffer                                */
/*                                                                        */
/*      attrs   - pair of text attributes to be put into the buffer       */
/*                with each character in the string.  Initially the       */
/*                low byte is used, and a '~' in the string toggles       */
/*                between the low byte and the high byte.                 */
/*                                                                        */
/*------------------------------------------------------------------------*/

void TDrawBuffer::moveCStr( ushort indent, const char _FAR *str, ushort attrs)
{
#if !defined ( __FLAT__ )
I   PUSH    DS
I   LDS     SI,str
I   CLD

    _ES = FP_SEG( &data[indent] );
    _DI = FP_OFF( &data[indent] );

I   MOV     BX,attrs
I   MOV     AH,BL

__1:

I   LODSB
I   CMP     AL,'~'
I   JE      __2
I   CMP     AL,0
I   JE      __3
I   STOSW
I   JMP     __1

__2:

I   XCHG    AH,bH
I   JMP     __1

__3:

I   POP     DS

#elif defined(__BORLANDC__)
    register ushort *dest = &data[indent];
    ushort *limit = &data[dataLength];
    int toggle;
    uchar c, curAttr;

    for (curAttr=((uchar *)&attrs)[0], toggle=1; dest < limit && (c=*str) != 0; str++)
        {
        if (c == '~')
            {
            curAttr = ((uchar *)&attrs)[toggle];
            toggle = 1-toggle;
            }
        else
            {
            ((uchar*)dest)[0] = c;
            ((uchar*)dest)[1] = curAttr;
            dest++;
            }
        }
#else
    return moveCStr(indent, TStringView {str}, attrs);
#endif
}

#ifndef __BORLANDC__
void TDrawBuffer::moveCStr( ushort indent, TStringView str, ushort attrs )
{
    size_t i = indent, j = 0;
    int toggle = 1;
    uchar curAttr = ((uchar *)&attrs)[0];

    while (i < dataLength && j < str.size())
        if (str[j] == '~')
        {
            curAttr = ((uchar *) &attrs)[toggle];
            toggle = 1 - toggle;
            ++j;
        }
        else
        {
            data[i].Attr = curAttr;
            TText::eat(&data[i], dataLength - i, i, str.substr(j, str.size() - j), j);
        }
}
#endif

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveStr                                                  */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      str     - pointer to a 0-terminated string of characters to       */
/*                be moved into the buffer                                */
/*                                                                        */
/*      attr    - text attribute to be put into the buffer with each      */
/*                character in the string.                                */
/*                                                                        */
/*------------------------------------------------------------------------*/

void TDrawBuffer::moveStr( ushort indent, const char _FAR *str, ushort attr )
{
#if !defined ( __FLAT__ )
I   PUSH    DS
I   LDS     SI,str
I   CLD

    _ES = FP_SEG( &data[indent] );
    _DI = FP_OFF( &data[indent] );

I   MOV     AH, byte ptr attr
I   OR      AH, AH
I   JZ      __2

__1:

I   LODSB
I   OR      AL,AL
I   JZ      __3
I   STOSW
I   JMP     __1

__2:

I   LODSB
I   OR      AL,AL
I   JZ      __3
I   STOSB
I   INC            DI
I   JMP     __2

__3:

I   POP     DS

#elif defined(__BORLANDC__)

    register ushort *dest = &data[indent];
    ushort *limit = &data[dataLength];
    uchar c;

    if (attr != 0)
        for (;dest < limit && (c=*str) != 0; ++str, ++dest)
            {
            ((uchar*)dest)[0] = c;
            ((uchar*)dest)[1] = (uchar)attr;
            }
        else
            while (dest < limit && *str)
                *(uchar *)dest++ = *str++;
#else
    return moveStr(indent, TStringView {str}, attr);
#endif
}

#ifndef __BORLANDC__
void TDrawBuffer::moveStr( ushort indent, TStringView str, ushort attr )
{
    size_t i = indent, j = 0;

    if (attr)
        while (i < dataLength && j < str.size())
        {
            data[i].Attr = (uchar) attr;
            TText::eat(&data[i], dataLength - i, i, str.substr(j, str.size() - j), j);
        }
    else
        while (i < dataLength && j < str.size())
            TText::eat(&data[i], dataLength - i, i, str.substr(j, str.size() - j), j);
}
#endif

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  TDrawBuffer::moveStr (2)                                              */
/*                                                                        */
/*  arguments:                                                            */
/*                                                                        */
/*      indent  - character position within the buffer where the data     */
/*                is to go                                                */
/*                                                                        */
/*      str     - pointer to a 0-terminated string of characters to       */
/*                be moved into the buffer                                */
/*                                                                        */
/*      attr    - text attribute to be put into the buffer with each      */
/*                character in the string.                                */
/*                                                                        */
/*      width   - number of display columns to be copied from str.        */
/*                                                                        */
/*      begin   - initial display column in str where to start counting.  */
/*                                                                        */
/*------------------------------------------------------------------------*/

void TDrawBuffer::moveStr( ushort indent, TStringView str, ushort attr, ushort width, ushort begin )
{
#ifdef __BORLANDC__
    int len = min(begin + width, str.size());
    len -= begin;
    if (len > 0)
        moveBuf(indent, &str[begin], attr, min(width, len));
#else
    size_t s = 0, remainder = 0;
    TText::wseek(str, s, remainder, begin);
    if (remainder)
        moveChar(indent, ' ', attr, remainder);
    size_t d = indent + remainder;
    size_t limit = std::min(dataLength, d + width);
    while (d < limit && s < str.size())
    {
        if (attr)
            data[d].Attr = (uchar) attr;
        TText::eat(&data[d], dataLength - d, d, {&str[s], str.size() - s}, s);
    }
#endif
}

#ifdef __FLAT__
TDrawBuffer::TDrawBuffer() {
    // This makes it possible to create TDrawBuffers for big screen widths.
    // This does not work nor is necessary in non-Flat builds.
    // Some views assume that width > height when drawing themselves (e.g. TScrollBar).
    dataLength = max(TScreen::screenWidth, TScreen::screenHeight);
    data = new TScreenCell[dataLength];
#ifndef __BORLANDC__
    // We need this as the TScreenCell struct has unused bits.
    memset(data, 0, dataLength*sizeof(TScreenCell));
#endif
}

TDrawBuffer::~TDrawBuffer() {
    delete[] data;
}
#endif

#pragma warn .asc
