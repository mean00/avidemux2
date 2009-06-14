
/*
 *  outputstream.h: Base Class for output
 *
 *
 *  Copyright (C) 2001 Andrew Stevens <andrew.stevens@philips.com>
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU General Public License
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef __OUTPUTSTRM_H__
#define __OUTPUTSTRM_H__


class OutputStream
{
public:
    OutputStream() :
        segment_num( 1 )
        {}
    virtual int  Open( ) = 0;
    virtual void Close() = 0;
    virtual off_t SegmentSize( ) = 0;
    virtual void NextSegment() = 0;
    virtual void Write(uint8_t *data, unsigned int len) = 0;
protected:
    int segment_num;
};

#endif /* __OUTPUTSTRM_H__ */
