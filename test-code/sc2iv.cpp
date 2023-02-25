/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/draggers/SoDragger.h>
#include <Inventor/SoDB.h>
#include <SmallChange/misc/Init.h>
#include <SmallChange/nodes/SmTrack.h>

#include <string>
#include <Inventor/SoInteraction.h>

// *************************************************************************

SoNode*
getSmallChange(const std::string& smallChangeObject) {
    SoNode * out=0;
    if(smallChangeObject == "SmTrack") {
        out = new SmTrack();
    }

    out->ref();
    return (out);
}

int
main(int argc,
     char ** argv )
{

    if(argc<3) {
        printf("usage is:\n%s <smallchange object> <inventor.iv>\n", argv[0]);
        return (0);
    }

    SoDB::init();
    smallchange_init();

    SoOutput file;
    assert(file.openFile(argv[2]));

    SoNode * root = getSmallChange(argv[1]);
    // Write the graph to stdout
    SoWriteAction wa(&file);
    wa.apply(root);

    SoDB::finish();

    return 0;
} // main()

// *************************************************************************
