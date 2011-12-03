#ifndef SM_TEXTURE_FONT_H
#define SM_TEXTURE_FONT_H

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

#include <Inventor/SbImage.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/elements/SoReplacedElement.h>
#include <Inventor/elements/SoSubElement.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbDict.h>
#include <SmallChange/basic.h>

class SoGLImage;


class SMALLCHANGE_DLL_API SmTextureFont : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(SmTextureFont);

 public:
  class SMALLCHANGE_DLL_API FontImage : public SbImage {
  public:
    FontImage(const SbVec2s glyphsize,
              const int leading,
              const int ascent,
              const int descent,
              const int numcomponents);
    ~FontImage();

    void addGlyph(unsigned char c,
                const SbImage & image,
                const int width,
                const int gfxwidth = -1,
                const int xoffset = 0);
    int getGlyphWidth(unsigned char c) const;

    void setKerning(unsigned char glyph, unsigned char next, int kerning);
    int getKerning(unsigned char glyph, unsigned char next) const;
    int getXOffset(unsigned char glyph) const;

    SoGLImage * getGLImage(void) const;

    SbVec2s getGlyphPositionPixels(unsigned char c) const;
    const SbVec2s & getGlyphSizePixels() const;

    SbVec2f getGlyphPosition(unsigned char c) const;
    SbVec2f getGlyphSize(unsigned char c) const;

    int getLeading() const;
    int getAscent() const;
    int getDescent() const;
    int height() const;

    SbImage getGlyphImage(const unsigned char c) const;

    int stringWidth(const SbString & s) const;
    void renderString(const SbString & s,
                      const SbVec3f & pos,
                      const bool needglbeginend = true) const;

  private:
    FontImage();

    void copyGlyph(unsigned char c, const SbImage & glyph);
    SbVec2s glyphsize;
    int numcomp;
    int leading;
    int ascent;
    int descent;
    SoGLImage * glimage;
    short glyphwidth[256];
    short gfxglyphwidth[256];
    short xoffset[256];
    SbDict kerningdict;

  };

  static void initClass(void);
  static void destroyClass(void);

  SmTextureFont(void);

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void pick(SoPickAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);

  virtual void setFont(FontImage * image);
  virtual const FontImage * getFont(void) const;

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

class SMALLCHANGE_DLL_API SmTextureFontBundle {
 public:
  SmTextureFontBundle(SoAction * action, SoNode * node = NULL);
  ~SmTextureFontBundle();

  void begin() const;
  void renderString(const SbString & string,
                  const SbVec3f & pos) const;
  void end() const;
  int stringWidth(const SbString & s) const {
    return this->font->stringWidth(s);
  }
  int getLeading() const {
    return this->font->getLeading();
  }
  int getAscent() const {
    return this->font->getAscent();
  }
  int getDescent() const {
    return this->font->getDescent();
  }
  int height() const {
    return this->font->height();
  }

 private:

  SoState * state;
  SoNode * node;
  bool didupdatecoin;
  const SmTextureFont::FontImage * font;
};


#endif // SM_TEXTURE_FONT_H
