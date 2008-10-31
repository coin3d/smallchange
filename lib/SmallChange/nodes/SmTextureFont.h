#ifndef SM_TEXTURE_FONT_H
#define SM_TEXTURE_FONT_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2008 by Systems in Motion.  All rights reserved.
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

#include <Inventor/SbImage.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/elements/SoReplacedElement.h>
#include <Inventor/elements/SoSubElement.h>
#include <Inventor/SbVec2f.h>
#include <SmallChange/basic.h>

class SoGLImage;

class SMALLCHANGE_DLL_API SmTextureFont : public SoNode {
  typedef SoNode inherited;
  
  SO_NODE_HEADER(SmTextureFont);

 public:
  class FontImage : public SbImage {
  public:
    FontImage(const SbVec2s glyphsize, const int numcomponents);
    ~FontImage();

    void addGlyph(unsigned char c, const SbImage & image);
    void setGlyphWidth(unsigned char c, short w);
    short getGlyphWidth(unsigned char c) const;
    
    const SbVec2s & getGlyphSize() const;
    SoGLImage * getGLImage(void) const;
    SbVec2s getGlyphPositionPixels(unsigned char c);

  private:
    FontImage();
    
    short findGlyphWidth(const SbImage & glyph);
    void copyGlyph(unsigned char c, const SbImage & glyph);
    SbVec2s glyphsize;
    SoGLImage * glimage;
    short glyphwidth[256];

  };

  static void initClass(void);
  SmTextureFont(void);
  
  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void pick(SoPickAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);

  void setFont(FontImage * image);

 protected:
  virtual ~SmTextureFont();
  
 private:
  FontImage * image;
};

class SMALLCHANGE_DLL_API SmTextureFontElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;

  SO_ELEMENT_HEADER(SmTextureFontElement);
 public:
  static void initClass(void);
 protected:
  virtual ~SmTextureFontElement();
  
 public:
  virtual void init(SoState * state);
  static void set(SoState * state, SoNode * node,
                  SmTextureFont::FontImage * image);

  static const SmTextureFont::FontImage * get(SoState * const state);
  
 private:
  SmTextureFont::FontImage * image;
};


#endif // SM_TEXTURE_FONT_H
