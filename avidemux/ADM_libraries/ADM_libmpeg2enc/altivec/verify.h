/* verify.h, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define ALTIVEC_TEST_VERIFY(name,ret,defargs,pfmt,args)                      \
ret name##_altivec_verify defargs {                                          \
  ret correct, verify;                                                       \
  correct = ALTIVEC_TEST_WITH(name)(args);                                   \
  verify  = name##_altivec(args);                                            \
  if (verify != correct)                                                     \
    mjpeg_debug(AVFMT(ret) " != " AVFMT(ret) "=" #name "(" pfmt ")",         \
                verify, correct, args);                                      \
  return correct;                                                            \
}

