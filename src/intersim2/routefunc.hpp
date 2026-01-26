// $Id: routefunc.hpp 5188 2012-08-30 00:31:31Z dub $

/*
 Copyright (c) 2007-2012, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _ROUTEFUNC_HPP_
#define _ROUTEFUNC_HPP_

#include "flit.hpp"
#include "outputset.hpp"
#include "config_utils.hpp"

/* Global information used by routing functions */

class RoutingContext{
  public:
    int gNumVCs;

    /* Add more functions here
    *
    */

    // ============================================================
    //  Balfour-Schultz
    int gReadReqBeginVC, gReadReqEndVC;
    int gWriteReqBeginVC, gWriteReqEndVC;
    int gReadReplyBeginVC, gReadReplyEndVC;
    int gWriteReplyBeginVC, gWriteReplyEndVC;
  
  RoutingContext() : gNumVCs(0),
                     gReadReqBeginVC(0), gReadReqEndVC(0),
                     gWriteReqBeginVC(0), gWriteReqEndVC(0),
                     gReadReplyBeginVC(0), gReadReplyEndVC(0),
                     gWriteReplyBeginVC(0), gWriteReplyEndVC(0)
    {}
};

class Router;

typedef void (*tRoutingFunction)( const RoutingContext*, const Router *, const Flit *, int in_channel, OutputSet *, bool );

RoutingContext* InitializeRoutingMap( const Configuration & config );

extern map<string, tRoutingFunction> gRoutingFunctionMap;

#endif
