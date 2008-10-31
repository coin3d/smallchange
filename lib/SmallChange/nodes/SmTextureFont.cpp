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

/*!
  \class SmTextureFont 
  \brief The SmTextureFont is used for setting up a texture font used by SmTextureText2

  Using this node, it's possible to use a third party library to generate the fonts
  to get potentially higher quality font rendering in Coin.
*/

#include "SmTextureFont.h"
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/actions/SoAction.h>
#include <assert.h>

/**************************************************************************/

SmTextureFont::FontImage::FontImage(const SbVec2s glyphsize_in, const int numcomp)
{
  this->glyphsize = glyphsize_in;
  for (int i = 0; i < 256; i++) {
    this->glyphwidth[i] =  0;
  }
  // FIXME: it should proably be possible to configure the width of the space character
  this->glyphwidth[' '] = glyphsize_in[0];

  // max glyphs = 256, sqrt(256) = 16
  int w = coin_geq_power_of_two(16 * glyphsize[0]);
  int h = coin_geq_power_of_two(16 * glyphsize[1]);
  
  this->setValue(SbVec2s(short(w), short(h)), numcomp, NULL);
  this->glimage = NULL;
}

SmTextureFont::FontImage::~FontImage()
{
  if (this->glimage) this->glimage->unref();
}

void 
SmTextureFont::FontImage::addGlyph(unsigned char c, const SbImage & image)
{
  if (this->glimage) {
    this->glimage->unref();
    this->glimage = NULL;
  }
  SbVec2s size;
  int nc;
  unsigned char * bytes = image.getValue(size, nc);
  assert(size == this->glyphsize);
  this->glyphwidth[c] = this->findGlyphWidth(image);
  this->copyGlyph(c, image);
}

SbVec2s 
SmTextureFont::FontImage::getGlyphPositionPixels(unsigned char c)
{
  // FIXME: I'm wasting some space in the texture since loads of the
  // characters aren't printable, but doing it like this to keep it
  // simple
  short column = c % 16;
  short row = c / 16;
  return SbVec2s(column * this->glyphsize[0], row * this->glyphsize[1]);
}

void 
SmTextureFont::FontImage::copyGlyph(unsigned char c, const SbImage & glyph)
{
  SbVec2s origo = this->getGlyphPositionPixels(c);
  int nc;
  SbVec2s size, fullsize;
  unsigned char * src = glyph.getValue(size, nc);
  unsigned char * dst = this->getValue(fullsize, nc);
  for (short y = 0; y < this->glyphsize[1]; y++) {
    for (short x = 0; x < this->glyphsize[0]; x++) {
      for (int i = 0; i < nc; i++) {
        src[(origo[1]+y)*fullsize[0]*nc + (origo[0]+x)*nc + i] = 
          dst[y*size[0]*nc + x*nc + i];
      }
    }
  }
}

/*!
  Can be used to adjust the width of a glyph, if you want the text
  node to offset more/less after rendering that specific glyph.
*/
void 
SmTextureFont::FontImage::setGlyphWidth(unsigned char c, short w)
{
  this->glyphwidth[c] = w;
}

short 
SmTextureFont::FontImage::getGlyphWidth(unsigned char c) const
{
  return this->glyphwidth[c];
}

SoGLImage * 
SmTextureFont::FontImage::getGLImage(void) const
{
  if (this->glimage == NULL) {
    SmTextureFont::FontImage * thisp = 
      const_cast<SmTextureFont::FontImage*> (this);
    thisp->glimage = new SoGLImage();
    SbVec2s size;
    int nc;
    unsigned char * bytes = this->getValue(size, nc);
    thisp->glimage->setData(bytes, size, nc);
  }
  return this->glimage;
}

short 
SmTextureFont::FontImage::findGlyphWidth(const SbImage & glyph)
{
  SbVec2s size;
  int nc;
  unsigned char * bytes = glyph.getValue(size, nc);
  
  short x;
  for (x = size[0]-1; x >= 0; x--) {
    short y;
    for (y = 0; y < size[1]; y++) {
      int c;
      for (c = 0; c < nc; c++) {
        if (bytes[y*size[0]*nc+x*nc+c] != 0) break;
      }
      if (c < nc) break;
    }
    if (y < size[1]) break;
  }
  return size[0] - x;
}

const SbVec2s & 
SmTextureFont::FontImage::getGlyphSize() const
{
  return this->glyphsize;
}

SmTextureFont::FontImage::FontImage()
{
  assert(0 && "shouldn't be used");
}

/**************************************************************************/

SO_NODE_SOURCE(SmTextureFont);

void 
SmTextureFont::initClass(void)
{
  SO_NODE_INIT_CLASS(SmTextureFont, inherited, "Node");
}

SmTextureFont::SmTextureFont(void)
{
  SO_NODE_CONSTRUCTOR(SmTextureFont);
  this->image = NULL;
}

SmTextureFont::~SmTextureFont()
{
  delete this->image;
}

void 
SmTextureFont::doAction(SoAction * action)
{
  SmTextureFontElement::set(action->getState(), 
                            this,
                            this->image);
}

void 
SmTextureFont::GLRender(SoGLRenderAction * action)
{
  SmTextureFont::doAction((SoAction*) action);
}

void 
SmTextureFont::pick(SoPickAction * action)
{
  SmTextureFont::doAction((SoAction*) action);
}

void 
SmTextureFont::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SmTextureFont::doAction((SoAction*) action);
}

void 
SmTextureFont::setFont(FontImage * image)
{
  this->touch();
  delete this->image;
  this->image = image;
}


/**************************************************************************/

SO_ELEMENT_SOURCE(SmTextureFontElement);

void 
SmTextureFontElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(SmTextureFontElement, inherited);
}

SmTextureFontElement::~SmTextureFontElement()
{
}
  
void 
SmTextureFontElement::init(SoState * state)
{
  this->image = NULL;
}

void 
SmTextureFontElement::set(SoState * state, 
                          SoNode * node,
                          SmTextureFont::FontImage * image)
{
  SmTextureFontElement * elem = static_cast<SmTextureFontElement*> 
    (inherited::getElement(state,
                           classStackIndex,
                           node));
  
  elem->image = image;
}

const SmTextureFont::FontImage * 
SmTextureFontElement::get(SoState * const state)
{
  const SmTextureFontElement * elem = static_cast<const SmTextureFontElement*> 
    (SoElement::getConstElement(state,
                                classStackIndex));
  return elem->image;
}

  
/**************************************************************************/
