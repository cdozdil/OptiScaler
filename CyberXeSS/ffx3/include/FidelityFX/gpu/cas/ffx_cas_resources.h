// This file is part of the FidelityFX SDK.
// 
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#ifndef FFX_CAS_RESOURCES_H
#define FFX_CAS_RESOURCES_H

#if defined(FFX_CPU) || defined(FFX_GPU)

#define FFX_CAS_RESOURCE_IDENTIFIER_NULL                       0
#define FFX_CAS_RESOURCE_IDENTIFIER_INPUT_COLOR                1
#define FFX_CAS_RESOURCE_IDENTIFIER_OUTPUT_COLOR               2

#define FFX_CAS_RESOURCE_IDENTIFIER_COUNT                      3

// CBV resource definitions
#define FFX_CAS_CONSTANTBUFFER_IDENTIFIER_CAS                  0

#endif  // #if defined(FFX_CPU) || defined(FFX_GPU)

#endif  // FFX_CAS_RESOURCES_H
