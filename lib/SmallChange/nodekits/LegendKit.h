#ifndef SMALLCHANGE_LEGENDKIT_H
#define SMALLCHANGE_LEGENDKIT_H

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

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFVec2f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoMFString.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

#include <SmallChange/basic.h>

class SbViewport;
class SoState;
class SbColor;
class SbVec2s;


class SMALLCHANGE_DLL_API LegendKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(LegendKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(viewport);
  SO_KIT_CATALOG_ENTRY_HEADER(resetTransform);
  SO_KIT_CATALOG_ENTRY_HEADER(position);
  SO_KIT_CATALOG_ENTRY_HEADER(depthBuffer);
  SO_KIT_CATALOG_ENTRY_HEADER(lightModel);
  SO_KIT_CATALOG_ENTRY_HEADER(camera);
  SO_KIT_CATALOG_ENTRY_HEADER(texture);
  SO_KIT_CATALOG_ENTRY_HEADER(shapeHints);
  SO_KIT_CATALOG_ENTRY_HEADER(pickStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(backgroundMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(backgroundShape);
  SO_KIT_CATALOG_ENTRY_HEADER(imageSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(imageTransform);
  SO_KIT_CATALOG_ENTRY_HEADER(imageMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(imageSwitch);     
  SO_KIT_CATALOG_ENTRY_HEADER(imageGroup);
  SO_KIT_CATALOG_ENTRY_HEADER(imageTranslation);
  SO_KIT_CATALOG_ENTRY_HEADER(image);
  SO_KIT_CATALOG_ENTRY_HEADER(textureGroup);
  SO_KIT_CATALOG_ENTRY_HEADER(textureQuality);
  SO_KIT_CATALOG_ENTRY_HEADER(textureImage);
  SO_KIT_CATALOG_ENTRY_HEADER(textureShape);
  SO_KIT_CATALOG_ENTRY_HEADER(tickMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(renderCallbackLines);
  SO_KIT_CATALOG_ENTRY_HEADER(textMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(renderCallbackText);
  SO_KIT_CATALOG_ENTRY_HEADER(extraNodes);

public:
  LegendKit(void);

  static void initClass(void);

public:

  SoSFBool on;
  SoSFFloat imageWidth;
  SoSFFloat space;
  SoSFFloat bigTickSize;
  SoSFFloat smallTickSize;
  SoSFString tickValueFormat;
  SoSFFloat tickValueOffset;
  SoMFString description;
  SoSFBool descriptionOnTop;
  SoSFBool delayedRender;
  SoSFFloat topSpace;
  SoSFBool discreteUseLower;
  SoSFBool threadSafe;

  void preRender(SoAction * action);
  
  void setImageTransparency(const float transparency = 0.0f);
  void useTextureNotImage(const SbBool onoff);
  void setColorCB(uint32_t (*colorCB)(double, void*), void * userdata = NULL);
  void setColorCB(uint32_t (*colorCB)(double));
  
  void clearTicks(void);
  void addSmallTick(double nval);
  void addBigTick(double nval, double tickvalue, const SbString * discretestring = NULL);
  void addBigTick(double nval, const SbString & string, const SbString * discretestring = NULL);

  void setDiscreteMode(const SbBool onoff);
  void addDiscreteColor(double uppernval, uint32_t color);
  void addDiscreteColor(double uppernval);
  
  void clearData(void);
  void enableImage(const SbBool onoff);

  float getLegendWidth(void) const;

  typedef SbString SoNumberFormatCB(const double number, void * closure);
  void setNumberFormatCallback(SoNumberFormatCB * cb, void * closure);

public:
  // convenience methods for setting part attributes
  void setTickAndLinesColor(const SbColor & color, const float transparency = 0.0f);
  void setTextColor(const SbColor & color, const float transparency = 0.0f);
  void setPosition(const SbVec2s & pos);
  void setBackgroundColor(const SbColor & color, const float transparency = 0.0f);
  void enableBackground(const SbBool onoff);


protected:
  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void handleEvent(SoHandleEventAction * action);
  virtual void search(SoSearchAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void getMatrix(SoGetMatrixAction * action);
  virtual void pick(SoPickAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void audioRender(SoAudioRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  
  virtual SbBool affectsState(void) const;

  virtual ~LegendKit();
  virtual void notify(SoNotList * list);
  
private:

  void recalcSize(SoState * state);
  void initBackground(const SbBool force = FALSE);
  void initTextureImage(void);
  void initImage(void);
  void reallyInitImage(unsigned char * data, unsigned char * rowdata = NULL);
  void fillImageAlpha(void);
  static void renderCBlines(void * userdata, SoAction * action);
  static void renderCBtext(void * userdata, SoAction * action);
  void render(SoGLRenderAction * action, const SbBool lines);
  void renderLines(SoGLRenderAction * action);
  void renderText(SoGLRenderAction * action);
  void renderString(const char * str, int xpos, int ypos);

  void setSwitchValue(const char * part, const int value);

  class LegendKitP * pimpl;
  friend class LegendKitP;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // !SMALLCHANGE_LEGENDKIT_H
