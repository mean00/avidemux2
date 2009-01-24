/*
  Synchrolib - various useful synchronisation primitives

  (C) 2001 Andrew Stevens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   Damn: shouldn't have to write this - but whenever you need
   something, well *that's* when you don't have Internet access.

   Bloody annyong that pthread_mutex's lock/unlock is only supposed to
   work properly if the same thread does the locking and unlocking. Gaah!
 */

// MEANX : removed, not used & cause problem on cygwin
