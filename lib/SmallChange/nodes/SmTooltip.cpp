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

/*!
  \class SmTooltip SmTooltip.h
  \brief The SmTooltip class is used to set tooltips for shapes.

  You'd normally insert a SmTooltipKit node in your scene graph and
  set its autoTrigger field to TRUE when using this node.

  Below is an example scene graph that shows how to use this node:

  \verbatim

  #Inventor V2.1 ascii

  Separator {
    SmTooltip { description "Cube" }
    Cube { }

    Translation { translation 4 0 0 }
    SmTooltip { description "Sphere" }
    Sphere { }

    Translation { translation 4 0 0 }
    SmTooltip { description "Cone" }
    Cone { }

    Translation { translation 4 0 0 }
    SmTooltip { description "Cylinder" }
    Cylinder { }

    SmTooltipKit { autoTrigger TRUE }
  }
  \endverbatim


  FIXME: more doc
*/


/*!
  \var SoMFString SmTooltip::description

  The tooltip string.
*/

#include "SmTooltip.h"

SO_NODE_SOURCE(SmTooltip);

/*!
  Constructor.
*/
SmTooltip::SmTooltip(void)
{
  SO_NODE_CONSTRUCTOR(SmTooltip);

  SO_NODE_ADD_FIELD(description, (""));
}

/*!
  Destructor.
*/
SmTooltip::~SmTooltip()
{
}

// Documented in superclass
void
SmTooltip::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(SmTooltip, SoNode, "Node");
  }
}
