#ifndef SMALLCHANGE_SCENERY_H
#define SMALLCHANGE_SCENERY_H

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

#include <Inventor/SbBasic.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFShort.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoMFFloat.h>

#include <SmallChange/basic.h>

typedef struct ss_system ss_system;
typedef struct ss_render_pre_cb_info ss_render_pre_cb_info;

typedef uint32_t SmSceneryTexture2CB(void * closure, double * xypos, float elevation, double * spacing);

class SceneryP;

class SMALLCHANGE_DLL_API SmScenery : public SoShape {
  typedef SoShape inherited;
  SO_NODE_HEADER(SmScenery);

public:
  static void initClass(void);

  static SmScenery * createInstance(double * origo, double * spacing, int * elements, float * values, const float undefval = 999999.0f);
  static SmScenery * createInstance(const int cols, const int rows, double * xyzgrid, const float undefz = 999999.0f);
  static SmScenery * createInstance(const int points, double * xyzvals, const float reach = -1.0f);

  static SmScenery * createCrossAndLineInstance(double * min, double * spacing, int * elements);

  SmScenery(void);

  SoSFString filename;
  SoSFFloat blockRottger;
  SoSFFloat loadRottger;

  SoMFInt32 renderSequence;

  enum ColorTexturing {
    DISABLED,
    INTERPOLATED,
    DISCRETE
  };

  SoSFEnum colorTexturing;
  SoMFFloat colorMap; // r, g, b, a values
  SoMFFloat colorElevation;

  SoSFBool elevationLines;
  SoSFFloat elevationLineDistance;
  SoSFFloat elevationLineOffset;
  SoSFShort elevationLineEmphasis;
  SoSFFloat elevationLineThickness;

  SoSFBool visualDebug;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);

  void preFrame(uint32_t glcontextid);
  int postFrame(uint32_t glcontextid);

  void setBlockRottger(const float c);
  float getBlockRottger(void) const;
  void setLoadRottger(const float c);
  float getLoadRottger(void) const;

  void setVertexArraysRendering(const SbBool onoff);
  SbBool getVertexArraysRendering(void) const;

  SbVec3f getRenderCoordinateOffset(void) const;
  SbVec2f getElevationRange(void) const;
  SbVec2f getDatasetElevationRange(int dataset) const;

  void set2DColorationTextureCB(SmSceneryTexture2CB * callback, void * closure);

  // dynamic texture callbacks
  static uint32_t colortexture_cb(void * node, double * xypos, float elevation, double * spacing);

  // action callbacks
  static SoCallbackAction::Response evaluateS(void * userdata, SoCallbackAction * action, const SoNode * node);

  SbBool getElevation(const double x, const double y, float & elev);

  void getSpacingForLodlevel(int lodlevel, double * spacing) const;
  float getUndefElevationValue(void) const;

  int addElevationDataset(const char * name);
  int addMaterialDataset(const char * name, uint32_t color);
  int addTextureDataset(int elevationdataset, const char * name, SmSceneryTexture2CB * cb, void * closure);

  int deleteElevationDataset(int datasetid);
  void setCrossAndLineElevationData(int datasetid, int lodlevel, int startcross, int startline, int numcross, int numline, float * elevationvalues);

  void changeDatasetProximity(int datasetid, int numdatasets, int * datasets, float epsilon, float newval);
  void cullDatasetAbove(int datasetid, int numdatasets, int * datasets, float distance);
  void cullDatasetBelow(int datasetid, int numdatasets, int * datasets, float distance);
  void oversampleDataset(int datasetid);
  void smoothDataset(int datasetid);
  void stripVerticals(int datasetid, float dropsize);
  void stripHorizontals(int datasetid, float maxskew);

  void refreshTextures(const int id);
  void * getScenerySystemHandle();

protected:
  virtual ~SmScenery(void);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);
  virtual void evaluate(SoAction * action);


private:
  SmScenery(ss_system * system);

  SoSFBool colorTexture;

  SceneryP * pimpl;
  friend class SceneryP;

};

#endif // !SMALLCHANGE_SCENERY_H
