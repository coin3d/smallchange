#ifndef SMALLCHANGE_AUTOFILE_H
#define SMALLCHANGE_AUTOFILE_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoFile.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFInt32.h>

class AutoFile : public SoFile {
  typedef SoFile inherited;

  SO_NODE_HEADER(AutoFile);

public:
  static void initClass(void);
  AutoFile(void);

  SoSFFloat interval;
  SoSFFloat delay;
  SoSFInt32 priority;
  SoSFBool active;
  SoSFBool stripTopSeparator;

  virtual void search(SoSearchAction * action);
  virtual void doAction(SoAction * action);
  
protected:
  virtual ~AutoFile();
  virtual void notify(SoNotList * list);
  virtual SbBool readNamedFile(SoInput * in);
private:
  class AutoFileP * pimpl;
  friend class AutoFileP;
};

#endif // !SMALLCHANGE_AUTOFILE_H
