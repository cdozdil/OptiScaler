/*********************************************************************************************************\
|*                                                                                                        *|
|* SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.  *|
|* SPDX-License-Identifier: MIT                                                                           *|
|*                                                                                                        *|
|* Permission is hereby granted, free of charge, to any person obtaining a                                *|
|* copy of this software and associated documentation files (the "Software"),                             *|
|* to deal in the Software without restriction, including without limitation                              *|
|* the rights to use, copy, modify, merge, publish, distribute, sublicense,                               *|
|* and/or sell copies of the Software, and to permit persons to whom the                                  *|
|* Software is furnished to do so, subject to the following conditions:                                   *|
|*                                                                                                        *|
|* The above copyright notice and this permission notice shall be included in                             *|
|* all copies or substantial portions of the Software.                                                    *|
|*                                                                                                        *|
|* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                             *|
|* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                               *|
|* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL                               *|
|* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                             *|
|* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING                                *|
|* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER                                    *|
|* DEALINGS IN THE SOFTWARE.                                                                              *|
|*                                                                                                        *|
|*                                                                                                        *|
\*********************************************************************************************************/

// ====================================================
// SAL related support
// ====================================================

#ifndef __ecount
    #define __nvapi_undef__ecount
    #define __ecount(size)
#endif
#ifndef __bcount
    #define __nvapi_undef__bcount
    #define __bcount(size)
#endif
#ifndef __in
    #define __nvapi_undef__in
    #define __in
#endif
#ifndef __in_ecount
    #define __nvapi_undef__in_ecount
    #define __in_ecount(size)
#endif
#ifndef __in_bcount
    #define __nvapi_undef__in_bcount
    #define __in_bcount(size)
#endif
#ifndef __in_z
    #define __nvapi_undef__in_z
    #define __in_z
#endif
#ifndef __in_ecount_z
    #define __nvapi_undef__in_ecount_z
    #define __in_ecount_z(size)
#endif
#ifndef __in_bcount_z
    #define __nvapi_undef__in_bcount_z
    #define __in_bcount_z(size)
#endif
#ifndef __in_nz
    #define __nvapi_undef__in_nz
    #define __in_nz
#endif
#ifndef __in_ecount_nz
    #define __nvapi_undef__in_ecount_nz
    #define __in_ecount_nz(size)
#endif
#ifndef __in_bcount_nz
    #define __nvapi_undef__in_bcount_nz
    #define __in_bcount_nz(size)
#endif
#ifndef __out
    #define __nvapi_undef__out
    #define __out
#endif
#ifndef __out_ecount
    #define __nvapi_undef__out_ecount
    #define __out_ecount(size)
#endif
#ifndef __out_bcount
    #define __nvapi_undef__out_bcount
    #define __out_bcount(size)
#endif
#ifndef __out_ecount_part
    #define __nvapi_undef__out_ecount_part
    #define __out_ecount_part(size,length)
#endif
#ifndef __out_bcount_part
    #define __nvapi_undef__out_bcount_part
    #define __out_bcount_part(size,length)
#endif
#ifndef __out_ecount_full
    #define __nvapi_undef__out_ecount_full
    #define __out_ecount_full(size)
#endif
#ifndef __out_bcount_full
    #define __nvapi_undef__out_bcount_full
    #define __out_bcount_full(size)
#endif
#ifndef __out_z
    #define __nvapi_undef__out_z
    #define __out_z
#endif
#ifndef __out_z_opt
    #define __nvapi_undef__out_z_opt
    #define __out_z_opt
#endif
#ifndef __out_ecount_z
    #define __nvapi_undef__out_ecount_z
    #define __out_ecount_z(size)
#endif
#ifndef __out_bcount_z
    #define __nvapi_undef__out_bcount_z
    #define __out_bcount_z(size)
#endif
#ifndef __out_ecount_part_z
    #define __nvapi_undef__out_ecount_part_z
    #define __out_ecount_part_z(size,length)
#endif
#ifndef __out_bcount_part_z
    #define __nvapi_undef__out_bcount_part_z
    #define __out_bcount_part_z(size,length)
#endif
#ifndef __out_ecount_full_z
    #define __nvapi_undef__out_ecount_full_z
    #define __out_ecount_full_z(size)
#endif
#ifndef __out_bcount_full_z
    #define __nvapi_undef__out_bcount_full_z
    #define __out_bcount_full_z(size)
#endif
#ifndef __out_nz
    #define __nvapi_undef__out_nz
    #define __out_nz
#endif
#ifndef __out_nz_opt
    #define __nvapi_undef__out_nz_opt
    #define __out_nz_opt
#endif
#ifndef __out_ecount_nz
    #define __nvapi_undef__out_ecount_nz
    #define __out_ecount_nz(size)
#endif
#ifndef __out_bcount_nz
    #define __nvapi_undef__out_bcount_nz
    #define __out_bcount_nz(size)
#endif
#ifndef __inout
    #define __nvapi_undef__inout
    #define __inout
#endif
#ifndef __inout_ecount
    #define __nvapi_undef__inout_ecount
    #define __inout_ecount(size)
#endif
#ifndef __inout_bcount
    #define __nvapi_undef__inout_bcount
    #define __inout_bcount(size)
#endif
#ifndef __inout_ecount_part
    #define __nvapi_undef__inout_ecount_part
    #define __inout_ecount_part(size,length)
#endif
#ifndef __inout_bcount_part
    #define __nvapi_undef__inout_bcount_part
    #define __inout_bcount_part(size,length)
#endif
#ifndef __inout_ecount_full
    #define __nvapi_undef__inout_ecount_full
    #define __inout_ecount_full(size)
#endif
#ifndef __inout_bcount_full
    #define __nvapi_undef__inout_bcount_full
    #define __inout_bcount_full(size)
#endif
#ifndef __inout_z
    #define __nvapi_undef__inout_z
    #define __inout_z
#endif
#ifndef __inout_ecount_z
    #define __nvapi_undef__inout_ecount_z
    #define __inout_ecount_z(size)
#endif
#ifndef __inout_bcount_z
    #define __nvapi_undef__inout_bcount_z
    #define __inout_bcount_z(size)
#endif
#ifndef __inout_nz
    #define __nvapi_undef__inout_nz
    #define __inout_nz
#endif
#ifndef __inout_ecount_nz
    #define __nvapi_undef__inout_ecount_nz
    #define __inout_ecount_nz(size)
#endif
#ifndef __inout_bcount_nz
    #define __nvapi_undef__inout_bcount_nz
    #define __inout_bcount_nz(size)
#endif
#ifndef __ecount_opt
    #define __nvapi_undef__ecount_opt
    #define __ecount_opt(size)
#endif
#ifndef __bcount_opt
    #define __nvapi_undef__bcount_opt
    #define __bcount_opt(size)
#endif
#ifndef __in_opt
    #define __nvapi_undef__in_opt
    #define __in_opt
#endif
#ifndef __in_ecount_opt
    #define __nvapi_undef__in_ecount_opt
    #define __in_ecount_opt(size)
#endif
#ifndef __in_bcount_opt
    #define __nvapi_undef__in_bcount_opt
    #define __in_bcount_opt(size)
#endif
#ifndef __in_z_opt
    #define __nvapi_undef__in_z_opt
    #define __in_z_opt
#endif
#ifndef __in_ecount_z_opt
    #define __nvapi_undef__in_ecount_z_opt
    #define __in_ecount_z_opt(size)
#endif
#ifndef __in_bcount_z_opt
    #define __nvapi_undef__in_bcount_z_opt
    #define __in_bcount_z_opt(size)
#endif
#ifndef __in_nz_opt
    #define __nvapi_undef__in_nz_opt
    #define __in_nz_opt
#endif
#ifndef __in_ecount_nz_opt
    #define __nvapi_undef__in_ecount_nz_opt
    #define __in_ecount_nz_opt(size)
#endif
#ifndef __in_bcount_nz_opt
    #define __nvapi_undef__in_bcount_nz_opt
    #define __in_bcount_nz_opt(size)
#endif
#ifndef __out_opt
    #define __nvapi_undef__out_opt
    #define __out_opt
#endif
#ifndef __out_ecount_opt
    #define __nvapi_undef__out_ecount_opt
    #define __out_ecount_opt(size)
#endif
#ifndef __out_bcount_opt
    #define __nvapi_undef__out_bcount_opt
    #define __out_bcount_opt(size)
#endif
#ifndef __out_ecount_part_opt
    #define __nvapi_undef__out_ecount_part_opt
    #define __out_ecount_part_opt(size,length)
#endif
#ifndef __out_bcount_part_opt
    #define __nvapi_undef__out_bcount_part_opt
    #define __out_bcount_part_opt(size,length)
#endif
#ifndef __out_ecount_full_opt
    #define __nvapi_undef__out_ecount_full_opt
    #define __out_ecount_full_opt(size)
#endif
#ifndef __out_bcount_full_opt
    #define __nvapi_undef__out_bcount_full_opt
    #define __out_bcount_full_opt(size)
#endif
#ifndef __out_ecount_z_opt
    #define __nvapi_undef__out_ecount_z_opt
    #define __out_ecount_z_opt(size)
#endif
#ifndef __out_bcount_z_opt
    #define __nvapi_undef__out_bcount_z_opt
    #define __out_bcount_z_opt(size)
#endif
#ifndef __out_ecount_part_z_opt
    #define __nvapi_undef__out_ecount_part_z_opt
    #define __out_ecount_part_z_opt(size,length)
#endif
#ifndef __out_bcount_part_z_opt
    #define __nvapi_undef__out_bcount_part_z_opt
    #define __out_bcount_part_z_opt(size,length)
#endif
#ifndef __out_ecount_full_z_opt
    #define __nvapi_undef__out_ecount_full_z_opt
    #define __out_ecount_full_z_opt(size)
#endif
#ifndef __out_bcount_full_z_opt
    #define __nvapi_undef__out_bcount_full_z_opt
    #define __out_bcount_full_z_opt(size)
#endif
#ifndef __out_ecount_nz_opt
    #define __nvapi_undef__out_ecount_nz_opt
    #define __out_ecount_nz_opt(size)
#endif
#ifndef __out_bcount_nz_opt
    #define __nvapi_undef__out_bcount_nz_opt
    #define __out_bcount_nz_opt(size)
#endif
#ifndef __inout_opt
    #define __nvapi_undef__inout_opt
    #define __inout_opt
#endif
#ifndef __inout_ecount_opt
    #define __nvapi_undef__inout_ecount_opt
    #define __inout_ecount_opt(size)
#endif
#ifndef __inout_bcount_opt
    #define __nvapi_undef__inout_bcount_opt
    #define __inout_bcount_opt(size)
#endif
#ifndef __inout_ecount_part_opt
    #define __nvapi_undef__inout_ecount_part_opt
    #define __inout_ecount_part_opt(size,length)
#endif
#ifndef __inout_bcount_part_opt
    #define __nvapi_undef__inout_bcount_part_opt
    #define __inout_bcount_part_opt(size,length)
#endif
#ifndef __inout_ecount_full_opt
    #define __nvapi_undef__inout_ecount_full_opt
    #define __inout_ecount_full_opt(size)
#endif
#ifndef __inout_bcount_full_opt
    #define __nvapi_undef__inout_bcount_full_opt
    #define __inout_bcount_full_opt(size)
#endif
#ifndef __inout_z_opt
    #define __nvapi_undef__inout_z_opt
    #define __inout_z_opt
#endif
#ifndef __inout_ecount_z_opt
    #define __nvapi_undef__inout_ecount_z_opt
    #define __inout_ecount_z_opt(size)
#endif
#ifndef __inout_ecount_z_opt
    #define __nvapi_undef__inout_ecount_z_opt
    #define __inout_ecount_z_opt(size)
#endif
#ifndef __inout_bcount_z_opt
    #define __nvapi_undef__inout_bcount_z_opt
    #define __inout_bcount_z_opt(size)
#endif
#ifndef __inout_nz_opt
    #define __nvapi_undef__inout_nz_opt
    #define __inout_nz_opt
#endif
#ifndef __inout_ecount_nz_opt
    #define __nvapi_undef__inout_ecount_nz_opt
    #define __inout_ecount_nz_opt(size)
#endif
#ifndef __inout_bcount_nz_opt
    #define __nvapi_undef__inout_bcount_nz_opt
    #define __inout_bcount_nz_opt(size)
#endif
#ifndef __deref_ecount
    #define __nvapi_undef__deref_ecount
    #define __deref_ecount(size)
#endif
#ifndef __deref_bcount
    #define __nvapi_undef__deref_bcount
    #define __deref_bcount(size)
#endif
#ifndef __deref_out
    #define __nvapi_undef__deref_out
    #define __deref_out
#endif
#ifndef __deref_out_ecount
    #define __nvapi_undef__deref_out_ecount
    #define __deref_out_ecount(size)
#endif
#ifndef __deref_out_bcount
    #define __nvapi_undef__deref_out_bcount
    #define __deref_out_bcount(size)
#endif
#ifndef __deref_out_ecount_part
    #define __nvapi_undef__deref_out_ecount_part
    #define __deref_out_ecount_part(size,length)
#endif
#ifndef __deref_out_bcount_part
    #define __nvapi_undef__deref_out_bcount_part
    #define __deref_out_bcount_part(size,length)
#endif
#ifndef __deref_out_ecount_full
    #define __nvapi_undef__deref_out_ecount_full
    #define __deref_out_ecount_full(size)
#endif
#ifndef __deref_out_bcount_full
    #define __nvapi_undef__deref_out_bcount_full
    #define __deref_out_bcount_full(size)
#endif
#ifndef __deref_out_z
    #define __nvapi_undef__deref_out_z
    #define __deref_out_z
#endif
#ifndef __deref_out_ecount_z
    #define __nvapi_undef__deref_out_ecount_z
    #define __deref_out_ecount_z(size)
#endif
#ifndef __deref_out_bcount_z
    #define __nvapi_undef__deref_out_bcount_z
    #define __deref_out_bcount_z(size)
#endif
#ifndef __deref_out_nz
    #define __nvapi_undef__deref_out_nz
    #define __deref_out_nz
#endif
#ifndef __deref_out_ecount_nz
    #define __nvapi_undef__deref_out_ecount_nz
    #define __deref_out_ecount_nz(size)
#endif
#ifndef __deref_out_bcount_nz
    #define __nvapi_undef__deref_out_bcount_nz
    #define __deref_out_bcount_nz(size)
#endif
#ifndef __deref_inout
    #define __nvapi_undef__deref_inout
    #define __deref_inout
#endif
#ifndef __deref_inout_z
    #define __nvapi_undef__deref_inout_z
    #define __deref_inout_z
#endif
#ifndef __deref_inout_ecount
    #define __nvapi_undef__deref_inout_ecount
    #define __deref_inout_ecount(size)
#endif
#ifndef __deref_inout_bcount
    #define __nvapi_undef__deref_inout_bcount
    #define __deref_inout_bcount(size)
#endif
#ifndef __deref_inout_ecount_part
    #define __nvapi_undef__deref_inout_ecount_part
    #define __deref_inout_ecount_part(size,length)
#endif
#ifndef __deref_inout_bcount_part
    #define __nvapi_undef__deref_inout_bcount_part
    #define __deref_inout_bcount_part(size,length)
#endif
#ifndef __deref_inout_ecount_full
    #define __nvapi_undef__deref_inout_ecount_full
    #define __deref_inout_ecount_full(size)
#endif
#ifndef __deref_inout_bcount_full
    #define __nvapi_undef__deref_inout_bcount_full
    #define __deref_inout_bcount_full(size)
#endif
#ifndef __deref_inout_z
    #define __nvapi_undef__deref_inout_z
    #define __deref_inout_z
#endif
#ifndef __deref_inout_ecount_z
    #define __nvapi_undef__deref_inout_ecount_z
    #define __deref_inout_ecount_z(size)
#endif
#ifndef __deref_inout_bcount_z
    #define __nvapi_undef__deref_inout_bcount_z
    #define __deref_inout_bcount_z(size)
#endif
#ifndef __deref_inout_nz
    #define __nvapi_undef__deref_inout_nz
    #define __deref_inout_nz
#endif
#ifndef __deref_inout_ecount_nz
    #define __nvapi_undef__deref_inout_ecount_nz
    #define __deref_inout_ecount_nz(size)
#endif
#ifndef __deref_inout_bcount_nz
    #define __nvapi_undef__deref_inout_bcount_nz
    #define __deref_inout_bcount_nz(size)
#endif
#ifndef __deref_ecount_opt
    #define __nvapi_undef__deref_ecount_opt
    #define __deref_ecount_opt(size)
#endif
#ifndef __deref_bcount_opt
    #define __nvapi_undef__deref_bcount_opt
    #define __deref_bcount_opt(size)
#endif
#ifndef __deref_out_opt
    #define __nvapi_undef__deref_out_opt
    #define __deref_out_opt
#endif
#ifndef __deref_out_ecount_opt
    #define __nvapi_undef__deref_out_ecount_opt
    #define __deref_out_ecount_opt(size)
#endif
#ifndef __deref_out_bcount_opt
    #define __nvapi_undef__deref_out_bcount_opt
    #define __deref_out_bcount_opt(size)
#endif
#ifndef __deref_out_ecount_part_opt
    #define __nvapi_undef__deref_out_ecount_part_opt
    #define __deref_out_ecount_part_opt(size,length)
#endif
#ifndef __deref_out_bcount_part_opt
    #define __nvapi_undef__deref_out_bcount_part_opt
    #define __deref_out_bcount_part_opt(size,length)
#endif
#ifndef __deref_out_ecount_full_opt
    #define __nvapi_undef__deref_out_ecount_full_opt
    #define __deref_out_ecount_full_opt(size)
#endif
#ifndef __deref_out_bcount_full_opt
    #define __nvapi_undef__deref_out_bcount_full_opt
    #define __deref_out_bcount_full_opt(size)
#endif
#ifndef __deref_out_z_opt
    #define __nvapi_undef__deref_out_z_opt
    #define __deref_out_z_opt
#endif
#ifndef __deref_out_ecount_z_opt
    #define __nvapi_undef__deref_out_ecount_z_opt
    #define __deref_out_ecount_z_opt(size)
#endif
#ifndef __deref_out_bcount_z_opt
    #define __nvapi_undef__deref_out_bcount_z_opt
    #define __deref_out_bcount_z_opt(size)
#endif
#ifndef __deref_out_nz_opt
    #define __nvapi_undef__deref_out_nz_opt
    #define __deref_out_nz_opt
#endif
#ifndef __deref_out_ecount_nz_opt
    #define __nvapi_undef__deref_out_ecount_nz_opt
    #define __deref_out_ecount_nz_opt(size)
#endif
#ifndef __deref_out_bcount_nz_opt
    #define __nvapi_undef__deref_out_bcount_nz_opt
    #define __deref_out_bcount_nz_opt(size)
#endif
#ifndef __deref_inout_opt
    #define __nvapi_undef__deref_inout_opt
    #define __deref_inout_opt
#endif
#ifndef __deref_inout_ecount_opt
    #define __nvapi_undef__deref_inout_ecount_opt
    #define __deref_inout_ecount_opt(size)
#endif
#ifndef __deref_inout_bcount_opt
    #define __nvapi_undef__deref_inout_bcount_opt
    #define __deref_inout_bcount_opt(size)
#endif
#ifndef __deref_inout_ecount_part_opt
    #define __nvapi_undef__deref_inout_ecount_part_opt
    #define __deref_inout_ecount_part_opt(size,length)
#endif
#ifndef __deref_inout_bcount_part_opt
    #define __nvapi_undef__deref_inout_bcount_part_opt
    #define __deref_inout_bcount_part_opt(size,length)
#endif
#ifndef __deref_inout_ecount_full_opt
    #define __nvapi_undef__deref_inout_ecount_full_opt
    #define __deref_inout_ecount_full_opt(size)
#endif
#ifndef __deref_inout_bcount_full_opt
    #define __nvapi_undef__deref_inout_bcount_full_opt
    #define __deref_inout_bcount_full_opt(size)
#endif
#ifndef __deref_inout_z_opt
    #define __nvapi_undef__deref_inout_z_opt
    #define __deref_inout_z_opt
#endif
#ifndef __deref_inout_ecount_z_opt
    #define __nvapi_undef__deref_inout_ecount_z_opt
    #define __deref_inout_ecount_z_opt(size)
#endif
#ifndef __deref_inout_bcount_z_opt
    #define __nvapi_undef__deref_inout_bcount_z_opt
    #define __deref_inout_bcount_z_opt(size)
#endif
#ifndef __deref_inout_nz_opt
    #define __nvapi_undef__deref_inout_nz_opt
    #define __deref_inout_nz_opt
#endif
#ifndef __deref_inout_ecount_nz_opt
    #define __nvapi_undef__deref_inout_ecount_nz_opt
    #define __deref_inout_ecount_nz_opt(size)
#endif
#ifndef __deref_inout_bcount_nz_opt
    #define __nvapi_undef__deref_inout_bcount_nz_opt
    #define __deref_inout_bcount_nz_opt(size)
#endif
#ifndef __deref_opt_ecount
    #define __nvapi_undef__deref_opt_ecount
    #define __deref_opt_ecount(size)
#endif
#ifndef __deref_opt_bcount
    #define __nvapi_undef__deref_opt_bcount
    #define __deref_opt_bcount(size)
#endif
#ifndef __deref_opt_out
    #define __nvapi_undef__deref_opt_out
    #define __deref_opt_out
#endif
#ifndef __deref_opt_out_z
    #define __nvapi_undef__deref_opt_out_z
    #define __deref_opt_out_z
#endif
#ifndef __deref_opt_out_ecount
    #define __nvapi_undef__deref_opt_out_ecount
    #define __deref_opt_out_ecount(size)
#endif
#ifndef __deref_opt_out_bcount
    #define __nvapi_undef__deref_opt_out_bcount
    #define __deref_opt_out_bcount(size)
#endif
#ifndef __deref_opt_out_ecount_part
    #define __nvapi_undef__deref_opt_out_ecount_part
    #define __deref_opt_out_ecount_part(size,length)
#endif
#ifndef __deref_opt_out_bcount_part
    #define __nvapi_undef__deref_opt_out_bcount_part
    #define __deref_opt_out_bcount_part(size,length)
#endif
#ifndef __deref_opt_out_ecount_full
    #define __nvapi_undef__deref_opt_out_ecount_full
    #define __deref_opt_out_ecount_full(size)
#endif
#ifndef __deref_opt_out_bcount_full
    #define __nvapi_undef__deref_opt_out_bcount_full
    #define __deref_opt_out_bcount_full(size)
#endif
#ifndef __deref_opt_inout
    #define __nvapi_undef__deref_opt_inout
    #define __deref_opt_inout
#endif
#ifndef __deref_opt_inout_ecount
    #define __nvapi_undef__deref_opt_inout_ecount
    #define __deref_opt_inout_ecount(size)
#endif
#ifndef __deref_opt_inout_bcount
    #define __nvapi_undef__deref_opt_inout_bcount
    #define __deref_opt_inout_bcount(size)
#endif
#ifndef __deref_opt_inout_ecount_part
    #define __nvapi_undef__deref_opt_inout_ecount_part
    #define __deref_opt_inout_ecount_part(size,length)
#endif
#ifndef __deref_opt_inout_bcount_part
    #define __nvapi_undef__deref_opt_inout_bcount_part
    #define __deref_opt_inout_bcount_part(size,length)
#endif
#ifndef __deref_opt_inout_ecount_full
    #define __nvapi_undef__deref_opt_inout_ecount_full
    #define __deref_opt_inout_ecount_full(size)
#endif
#ifndef __deref_opt_inout_bcount_full
    #define __nvapi_undef__deref_opt_inout_bcount_full
    #define __deref_opt_inout_bcount_full(size)
#endif
#ifndef __deref_opt_inout_z
    #define __nvapi_undef__deref_opt_inout_z
    #define __deref_opt_inout_z
#endif
#ifndef __deref_opt_inout_ecount_z
    #define __nvapi_undef__deref_opt_inout_ecount_z
    #define __deref_opt_inout_ecount_z(size)
#endif
#ifndef __deref_opt_inout_bcount_z
    #define __nvapi_undef__deref_opt_inout_bcount_z
    #define __deref_opt_inout_bcount_z(size)
#endif
#ifndef __deref_opt_inout_nz
    #define __nvapi_undef__deref_opt_inout_nz
    #define __deref_opt_inout_nz
#endif
#ifndef __deref_opt_inout_ecount_nz
    #define __nvapi_undef__deref_opt_inout_ecount_nz
    #define __deref_opt_inout_ecount_nz(size)
#endif
#ifndef __deref_opt_inout_bcount_nz
    #define __nvapi_undef__deref_opt_inout_bcount_nz
    #define __deref_opt_inout_bcount_nz(size)
#endif
#ifndef __deref_opt_ecount_opt
    #define __nvapi_undef__deref_opt_ecount_opt
    #define __deref_opt_ecount_opt(size)
#endif
#ifndef __deref_opt_bcount_opt
    #define __nvapi_undef__deref_opt_bcount_opt
    #define __deref_opt_bcount_opt(size)
#endif
#ifndef __deref_opt_out_opt
    #define __nvapi_undef__deref_opt_out_opt
    #define __deref_opt_out_opt
#endif
#ifndef __deref_opt_out_ecount_opt
    #define __nvapi_undef__deref_opt_out_ecount_opt
    #define __deref_opt_out_ecount_opt(size)
#endif
#ifndef __deref_opt_out_bcount_opt
    #define __nvapi_undef__deref_opt_out_bcount_opt
    #define __deref_opt_out_bcount_opt(size)
#endif
#ifndef __deref_opt_out_ecount_part_opt
    #define __nvapi_undef__deref_opt_out_ecount_part_opt
    #define __deref_opt_out_ecount_part_opt(size,length)
#endif
#ifndef __deref_opt_out_bcount_part_opt
    #define __nvapi_undef__deref_opt_out_bcount_part_opt
    #define __deref_opt_out_bcount_part_opt(size,length)
#endif
#ifndef __deref_opt_out_ecount_full_opt
    #define __nvapi_undef__deref_opt_out_ecount_full_opt
    #define __deref_opt_out_ecount_full_opt(size)
#endif
#ifndef __deref_opt_out_bcount_full_opt
    #define __nvapi_undef__deref_opt_out_bcount_full_opt
    #define __deref_opt_out_bcount_full_opt(size)
#endif
#ifndef __deref_opt_out_z_opt
    #define __nvapi_undef__deref_opt_out_z_opt
    #define __deref_opt_out_z_opt
#endif
#ifndef __deref_opt_out_ecount_z_opt
    #define __nvapi_undef__deref_opt_out_ecount_z_opt
    #define __deref_opt_out_ecount_z_opt(size)
#endif
#ifndef __deref_opt_out_bcount_z_opt
    #define __nvapi_undef__deref_opt_out_bcount_z_opt
    #define __deref_opt_out_bcount_z_opt(size)
#endif
#ifndef __deref_opt_out_nz_opt
    #define __nvapi_undef__deref_opt_out_nz_opt
    #define __deref_opt_out_nz_opt
#endif
#ifndef __deref_opt_out_ecount_nz_opt
    #define __nvapi_undef__deref_opt_out_ecount_nz_opt
    #define __deref_opt_out_ecount_nz_opt(size)
#endif
#ifndef __deref_opt_out_bcount_nz_opt
    #define __nvapi_undef__deref_opt_out_bcount_nz_opt
    #define __deref_opt_out_bcount_nz_opt(size)
#endif
#ifndef __deref_opt_inout_opt
    #define __nvapi_undef__deref_opt_inout_opt
    #define __deref_opt_inout_opt
#endif
#ifndef __deref_opt_inout_ecount_opt
    #define __nvapi_undef__deref_opt_inout_ecount_opt
    #define __deref_opt_inout_ecount_opt(size)
#endif
#ifndef __deref_opt_inout_bcount_opt
    #define __nvapi_undef__deref_opt_inout_bcount_opt
    #define __deref_opt_inout_bcount_opt(size)
#endif
#ifndef __deref_opt_inout_ecount_part_opt
    #define __nvapi_undef__deref_opt_inout_ecount_part_opt
    #define __deref_opt_inout_ecount_part_opt(size,length)
#endif
#ifndef __deref_opt_inout_bcount_part_opt
    #define __nvapi_undef__deref_opt_inout_bcount_part_opt
    #define __deref_opt_inout_bcount_part_opt(size,length)
#endif
#ifndef __deref_opt_inout_ecount_full_opt
    #define __nvapi_undef__deref_opt_inout_ecount_full_opt
    #define __deref_opt_inout_ecount_full_opt(size)
#endif
#ifndef __deref_opt_inout_bcount_full_opt
    #define __nvapi_undef__deref_opt_inout_bcount_full_opt
    #define __deref_opt_inout_bcount_full_opt(size)
#endif
#ifndef __deref_opt_inout_z_opt
    #define __nvapi_undef__deref_opt_inout_z_opt
    #define __deref_opt_inout_z_opt
#endif
#ifndef __deref_opt_inout_ecount_z_opt
    #define __nvapi_undef__deref_opt_inout_ecount_z_opt
    #define __deref_opt_inout_ecount_z_opt(size)
#endif
#ifndef __deref_opt_inout_bcount_z_opt
    #define __nvapi_undef__deref_opt_inout_bcount_z_opt
    #define __deref_opt_inout_bcount_z_opt(size)
#endif
#ifndef __deref_opt_inout_nz_opt
    #define __nvapi_undef__deref_opt_inout_nz_opt
    #define __deref_opt_inout_nz_opt
#endif
#ifndef __deref_opt_inout_ecount_nz_opt
    #define __nvapi_undef__deref_opt_inout_ecount_nz_opt
    #define __deref_opt_inout_ecount_nz_opt(size)
#endif
#ifndef __deref_opt_inout_bcount_nz_opt
    #define __nvapi_undef__deref_opt_inout_bcount_nz_opt
    #define __deref_opt_inout_bcount_nz_opt(size)
#endif
#ifndef __success
    #define __nvapi_success
    #define __success(epxr)
#endif
#ifndef _Ret_notnull_
    #define __nvapi__Ret_notnull_
    #define _Ret_notnull_
#endif
#ifndef _Post_writable_byte_size_
    #define __nvapi__Post_writable_byte_size_
    #define _Post_writable_byte_size_(n)
#endif
#ifndef _Outptr_ 
    #define __nvapi_Outptr_ 
    #define _Outptr_ 
#endif


#define NVAPI_INTERFACE extern __success(return == NVAPI_OK) NvAPI_Status __cdecl
