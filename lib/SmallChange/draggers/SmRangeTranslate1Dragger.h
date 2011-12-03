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

#ifndef SMALLCHANGE_RANGETRANSLATE1DRAGGER_H
#define SMALLCHANGE_RANGETRANSLATE1DRAGGER_H

#include <Inventor/draggers/SoDragger.h>
#include <Inventor/fields/SoSFVec2f.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SmRangeTranslate1Dragger : public SoDragger {

  typedef SoDragger inherited;
  SO_KIT_HEADER(SmRangeTranslate1Dragger);
  SO_KIT_CATALOG_ENTRY_HEADER(topSep);

public:
  SmRangeTranslate1Dragger(void);
  static void initClass(void);
  virtual SbBool affectsState(void) const;

  SoSFVec2f range;
  // FIXME: shouldn't this rather be an SoSFFloat? 20031023 mortene.
  SoSFVec3f translation;

protected:
  virtual ~SmRangeTranslate1Dragger();

private:
  friend class SmRangeTranslate1DraggerP;
  class SmRangeTranslate1DraggerP * pimpl;
  void initSmRangeTranslate1Dragger();

};

#endif // ! SMALLCHANGE_RANGETRANSLATE1DRAGGER_H
