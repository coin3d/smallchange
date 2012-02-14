#ifndef SMALLCHANGE_SOCAMERAPATHEDITKIT_H
#define SMALLCHANGE_SOCAMERAPATHEDITKIT_H

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

#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/nodekits/SoShapeKit.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/draggers/SoDragPointDragger.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFTime.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SoCameraPathEditKit : public SoBaseKit
{
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SoCameraPathEditKit);

  SO_KIT_CATALOG_ENTRY_HEADER(rootSeparator);

  SO_KIT_CATALOG_ENTRY_HEADER(eventCallback);
  SO_KIT_CATALOG_ENTRY_HEADER(draggerSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(feedbackSeparator);

  SO_KIT_CATALOG_ENTRY_HEADER(draggerPick);
  SO_KIT_CATALOG_ENTRY_HEADER(draggerDraw);
  SO_KIT_CATALOG_ENTRY_HEADER(draggerTransform);
  SO_KIT_CATALOG_ENTRY_HEADER(draggerSwitch);

  SO_KIT_CATALOG_ENTRY_HEADER(timeDragger);
  SO_KIT_CATALOG_ENTRY_HEADER(posDragger);
  SO_KIT_CATALOG_ENTRY_HEADER(orDragger);

  SO_KIT_CATALOG_ENTRY_HEADER(orLightModel);
  SO_KIT_CATALOG_ENTRY_HEADER(headingMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(heading);
  SO_KIT_CATALOG_ENTRY_HEADER(pitchMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(pitchRotate);
  SO_KIT_CATALOG_ENTRY_HEADER(pitch);
  SO_KIT_CATALOG_ENTRY_HEADER(bankMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(bankRotate);
  SO_KIT_CATALOG_ENTRY_HEADER(bank);

  SO_KIT_CATALOG_ENTRY_HEADER(coordinates);
  SO_KIT_CATALOG_ENTRY_HEADER(tagSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(tagDrawStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(tags);
  SO_KIT_CATALOG_ENTRY_HEADER(curveSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(curveDrawStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(controlpoints);
  SO_KIT_CATALOG_ENTRY_HEADER(curve);
  SO_KIT_CATALOG_ENTRY_HEADER(objectSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(objectTransform);
  SO_KIT_CATALOG_ENTRY_HEADER(objectDrawStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(objectModel);

  enum Editmode {
    POSITION = 1,
    ORIENTATION,
    TIME
  };


  // Fields
  SoSFBool timestampEnabled;
  SoSFBool timestampVisible;
  SoSFFloat timeScale;

  SoMFVec3f position;
  SoMFVec3f orientation;
  SoMFTime timestamp;
  SoSFInt32 activePoint;
  SoSFEnum editMode;


public:

  SoCameraPathEditKit(void);
  static void initClass(void);


  void flipEditmode(void);
  void setEditmode(Editmode mode);

  int setTimestamp(int idx, const SbTime &time);
  int setTimestamp(int idx, float time);
  void setPosition(int idx, const SbVec3f &pos);
  void setOrientation(int idx, const SbVec3f &orientation);
  int setActiveTimestamp(const SbTime &time);
  void setActivePosition(const SbVec3f &pos);
  void setActiveOrientation(const SbVec3f &orientation);

  void insertControlpoint(int idx, const SbVec3f &pos, const SbVec3f &orientation);
  int insertControlpoint(const SbVec3f &pos, const SbVec3f &orientation, const SbTime &time);
  void insertPosition(int idx, const SbVec3f &pos);
  int insertPosition(const SbVec3f &pos, const SbTime &time);
  void insertOrientation(int idx, const SbVec3f &orientation);
  int insertOrientation(const SbVec3f &orientation, const SbTime &time);
  int insertControlpoint(const SbTime &time);

  void deleteControlpoint(int idx);
  void deleteActivePoint(void);



private:
  virtual ~SoCameraPathEditKit();

  static void pickCallback(void *, SoEventCallback *);
  static void posCallback(void *, SoSensor *);
  static void timeCallback(void *, SoSensor *);
  static void orCallback(void *, SoSensor *);
  static void keyCallback(void *, SoEventCallback *);

  static void dragtimeFinishCB(void *, SoDragger *);
  static void dragtimeStartCB(void *, SoDragger *);
  static void dragposFinishCB(void *, SoDragger *);
  static void dragposStartCB(void *, SoDragger *);

  void setControlpoint(int idx, const SbVec3f &pos, const SbVec3f &orient, const SbTime &time);

  static void activePointCB(void *, SoSensor *);

  void updateDraggers(void);

  void buildTag(int tagIdx);
  void buildTags(void);

  int numControlpoints;

  SbTime dragStarttime;
};

#endif // !SMALLCHANGE_SOCAMERAPATHEDITKIT_H
