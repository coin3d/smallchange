/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SmallChange with software that can not be combined with the
 *  GNU GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
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


/*
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
