#ifndef SMALLCHANGE_SOTCBCURVEKIT_H
#define SMALLCHANGE_SOTCBCURVEKIT_H

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

#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/nodekits/SoShapeKit.h>
#include <Inventor/nodes/SoEventCallback.h>
#include "../curve/SoTCBCurve.h"
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/draggers/SoDragPointDragger.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/fields/SoSFInt32.h>


class SoTCBCurveKit : public SoBaseKit
{
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SoTCBCurveKit);


/*
TODO: 

  rydd opp i konstruktør
  gjør mange av partene ikke-public
  putt .iv-filer i header-opplegg
  lag pimpl-klasser
    SoTCBCurveKit
  dokumenter
    SoTCBCurveKit
  putt inn flagg/node for å ha synlig feedbackmodell
  lagring til fil
  loading fra fil
  engine for animering?
*/


/*
root  eventCallback

  
      draggerSeparator        draggerPick
                              draggerDraw
                              draggerTransform
                              draggerSwitch             timeDragger
                                                        posDragger
                                                        orDragger       headingMaterial
                                                                        heading
                                                                        pitchMaterial
                                                                        pitchRotate
                                                                        pitch
                                                                        bankMaterial
                                                                        bankRotate
                                                                        bank
      

      feedbackSeparator       coordinates
                              tagSeparator              tagDrawStyle
                                                        tags                        
                              curveSeparator            curveDrawStyle
                                                        controlPoints  
                                                        curve                              
                              objectSeparator           objectDrawStyle
                                                        objectTransform                                                        
                                                        objectModel



*/



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
  SoSFBool    timestampEnabled;
  SoSFBool    timestampVisible;
  SoSFFloat   timeScale; 

  SoMFVec3f   position;
  SoMFVec3f   orientation;
  SoMFTime    timestamp;
  SoSFInt32   activePoint;
  SoSFEnum    editMode;


public:


  SoTCBCurveKit();
  static void initClass();


  void  flipEditmode();
  void  setEditmode(Editmode mode);

  int   setTimestamp(int idx, const SbTime &time);
  int   setTimestamp(int idx, float time);
  void  setPosition(int idx, const SbVec3f &pos);
  void  setOrientation(int idx, const SbVec3f &or);
  int   setActiveTimestamp(const SbTime &time);
  void  setActivePosition(const SbVec3f &pos);
  void  setActiveOrientation(const SbVec3f &or);

  void  insertControlpoint(int idx, const SbVec3f &pos, const SbVec3f &or);                     
  int   insertControlpoint(const SbVec3f &pos, const SbVec3f &or, const SbTime &time);          
  void  insertPosition(int idx, const SbVec3f &pos);                                            
  int   insertPosition(const SbVec3f &pos, const SbTime &time);                                 
  void  insertOrientation(int idx, const SbVec3f &or);                                          
  int   insertOrientation(const SbVec3f &or, const SbTime &time);                               
  int   insertControlpoint(const SbTime &time);                                                 

  void  deleteControlpoint(int idx);
  void  deleteActivePoint();



private:
  virtual ~SoTCBCurveKit();
  
  static void pickCallback(void *, SoEventCallback *);
  static void posCallback(void *, SoSensor *);
  static void timeCallback(void *, SoSensor *);
  static void orCallback(void *, SoSensor *);
  static void keyCallback(void *, SoEventCallback *);

  static void dragtimeFinishCB(void *, SoDragger *);
  static void dragtimeStartCB(void *, SoDragger *);
  static void dragposFinishCB(void *, SoDragger *);
  static void dragposStartCB(void *, SoDragger *);

  void setControlpoint(int idx, const SbVec3f &pos, const SbVec3f &or, const SbTime &time);

  static void activePointCB(void *, SoSensor *);

  void  updateDraggers();

  void  buildTag(int tagIdx);
  void  buildTags();

  int       numControlpoints;

  SbTime    dragStarttime;
};

#endif // !SMALLCHANGE_SOTCBCURVEKIT_H
