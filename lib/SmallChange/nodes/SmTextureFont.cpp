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

/*!
  \class SmTextureFont
  \brief The SmTextureFont is used for setting up a texture font used by SmTextureText2

  Using this node, it's possible to use a third party library to generate the fonts
  to get potentially higher quality font rendering in Coin.
*/

#include "SmTextureFont.h"
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <assert.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/system/gl.h>
#include <string.h>

/**************************************************************************/

static const unsigned char texture_fontdata[][12] = {
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, //
  {  0,  0, 12, 12,  0,  8, 12, 12, 12, 12, 12,  0 }, // !
  {  0,  0,  0,  0,  0,  0,  0,  0,  0, 20, 20, 20 }, // "
  {  0,  0, 18, 18, 18, 63, 18, 18, 63, 18, 18,  0 }, // #
  {  0,  8, 28, 42, 10, 10, 12, 24, 40, 42, 28,  8 }, // $
  {  0,  0,  6, 73, 41, 22,  8, 52, 74, 73, 48,  0 }, // %
  {  0, 12, 18, 18, 12, 25, 37, 34, 34, 29,  0,  0 }, // &
  {  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 12, 12 }, // '
  {  0,  6,  8,  8, 16, 16, 16, 16, 16,  8,  8,  6 }, // (
  {  0, 48,  8,  8,  4,  4,  4,  4,  4,  8,  8, 48 }, // )
  {  0,  0,  0,  0,  0,  0,  8, 42, 20, 42,  8,  0 }, // *
  {  0,  0,  0,  8,  8,  8,127,  8,  8,  8,  0,  0 }, // +
  {  0, 24, 12, 12,  0,  0,  0,  0,  0,  0,  0,  0 }, // ,
  {  0,  0,  0,  0,  0,  0,127,  0,  0,  0,  0,  0 }, // -
  {  0,  0, 24, 24,  0,  0,  0,  0,  0,  0,  0,  0 }, // .
  {  0, 32, 32, 16, 16,  8,  8,  8,  4,  4,  2,  2 }, // /
  {  0,  0, 28, 34, 34, 34, 34, 34, 34, 34, 28,  0 }, // 0
  {  0,  0,  8,  8,  8,  8,  8,  8, 40, 24,  8,  0 }, // 1
  {  0,  0, 62, 32, 16,  8,  4,  2,  2, 34, 28,  0 }, // 2
  {  0,  0, 28, 34,  2,  2, 12,  2,  2, 34, 28,  0 }, // 3
  {  0,  0,  4,  4,  4,126, 68, 36, 20, 12,  4,  0 }, // 4
  {  0,  0, 28, 34,  2,  2,  2, 60, 32, 32, 62,  0 }, // 5
  {  0,  0, 28, 34, 34, 34, 60, 32, 32, 34, 28,  0 }, // 6
  {  0,  0, 16, 16, 16,  8,  8,  4,  2,  2, 62,  0 }, // 7
  {  0,  0, 28, 34, 34, 34, 28, 34, 34, 34, 28,  0 }, // 8
  {  0,  0, 28, 34,  2,  2, 30, 34, 34, 34, 28,  0 }, // 9
  {  0,  0, 24, 24,  0,  0,  0, 24, 24,  0,  0,  0 }, // :
  {  0, 48, 24, 24,  0,  0,  0, 24, 24,  0,  0,  0 }, // ;
  {  0,  0,  0,  2,  4,  8, 16,  8,  4,  2,  0,  0 }, // <
  {  0,  0,  0,  0,  0,127,  0,127,  0,  0,  0,  0 }, // =
  {  0,  0,  0, 16,  8,  4,  2,  4,  8, 16,  0,  0 }, // >
  {  0,  0, 16, 16,  0, 16, 28,  2,  2,  2, 60,  0 }, // ?
  {  0,  0, 28, 32, 73, 86, 82, 82, 78, 34, 28,  0 }, // @
  {  0,  0, 33, 33, 33, 63, 18, 18, 18, 12, 12,  0 }, // A
  {  0,  0, 60, 34, 34, 34, 60, 34, 34, 34, 60,  0 }, // B
  {  0,  0, 14, 16, 32, 32, 32, 32, 32, 18, 14,  0 }, // C
  {  0,  0, 56, 36, 34, 34, 34, 34, 34, 36, 56,  0 }, // D
  {  0,  0, 62, 32, 32, 32, 60, 32, 32, 32, 62,  0 }, // E
  {  0,  0, 16, 16, 16, 16, 30, 16, 16, 16, 30,  0 }, // F
  {  0,  0, 14, 18, 34, 34, 32, 32, 32, 18, 14,  0 }, // G
  {  0,  0, 34, 34, 34, 34, 62, 34, 34, 34, 34,  0 }, // H
  {  0,  0, 62,  8,  8,  8,  8,  8,  8,  8, 62,  0 }, // I
  {  0,  0,112,  8,  8,  8,  8,  8,  8,  8, 62,  0 }, // J
  {  0,  0, 33, 33, 34, 36, 56, 40, 36, 34, 33,  0 }, // K
  {  0,  0, 30, 16, 16, 16, 16, 16, 16, 16, 16,  0 }, // L
  {  0,  0, 33, 33, 33, 45, 45, 45, 51, 51, 33,  0 }, // M
  {  0,  0, 34, 34, 38, 38, 42, 42, 50, 50, 34,  0 }, // N
  {  0,  0, 12, 18, 33, 33, 33, 33, 33, 18, 12,  0 }, // O
  {  0,  0, 32, 32, 32, 60, 34, 34, 34, 34, 60,  0 }, // P
  {  3,  6, 12, 18, 33, 33, 33, 33, 33, 18, 12,  0 }, // Q
  {  0,  0, 34, 34, 34, 36, 60, 34, 34, 34, 60,  0 }, // R
  {  0,  0, 60,  2,  2,  6, 28, 48, 32, 32, 30,  0 }, // S
  {  0,  0,  8,  8,  8,  8,  8,  8,  8,  8,127,  0 }, // T
  {  0,  0, 28, 34, 34, 34, 34, 34, 34, 34, 34,  0 }, // U
  {  0,  0, 12, 12, 18, 18, 18, 33, 33, 33, 33,  0 }, // V
  {  0,  0, 34, 34, 34, 54, 85, 73, 73, 73, 65,  0 }, // W
  {  0,  0, 34, 34, 20, 20,  8, 20, 20, 34, 34,  0 }, // X
  {  0,  0,  8,  8,  8,  8, 20, 20, 34, 34, 34,  0 }, // Y
  {  0,  0, 62, 32, 16, 16,  8,  4,  4,  2, 62,  0 }, // Z
  {  0, 14,  8,  8,  8,  8,  8,  8,  8,  8,  8, 14 }, // [
  {  0,  2,  2,  4,  4,  8,  8,  8, 16, 16, 32, 32 }, // [backslash]
  {  0, 56,  8,  8,  8,  8,  8,  8,  8,  8,  8, 56 }, // ]
  //  {  0,  0,  0,  0,  0, 34, 34, 20, 20,  8,  8,  0 }, // ^ (rotate)
  {  0,  0,  0,127,  1,  1, 65,225, 65, 65,127,  0 },
  {  0,127,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, // _
  {  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 24, 12 }, // `
  {  0,  0, 29, 34, 34, 30,  2, 34, 28,  0,  0,  0 }, // a
  {  0,  0, 60, 34, 34, 34, 34, 50, 44, 32, 32, 32 }, // b
  {  0,  0, 14, 16, 32, 32, 32, 16, 14,  0,  0,  0 }, // c
  {  0,  0, 26, 38, 34, 34, 34, 34, 30,  2,  2,  2 }, // d
  {  0,  0, 28, 34, 32, 62, 34, 34, 28,  0,  0,  0 }, // e
  {  0,  0, 16, 16, 16, 16, 16, 16, 62, 16, 16, 14 }, // f
  { 28,  2,  2, 26, 38, 34, 34, 34, 30,  0,  0,  0 }, // g
  {  0,  0, 34, 34, 34, 34, 34, 50, 44, 32, 32, 32 }, // h
  {  0,  0,  8,  8,  8,  8,  8,  8, 56,  0,  8,  8 }, // i
  { 56,  4,  4,  4,  4,  4,  4,  4, 60,  0,  4,  4 }, // j
  {  0,  0, 33, 34, 36, 56, 40, 36, 34, 32, 32, 32 }, // k
  {  0,  0,  8,  8,  8,  8,  8,  8,  8,  8,  8, 56 }, // l
  {  0,  0, 73, 73, 73, 73, 73,109, 82,  0,  0,  0 }, // m
  {  0,  0, 34, 34, 34, 34, 34, 50, 44,  0,  0,  0 }, // n
  {  0,  0, 28, 34, 34, 34, 34, 34, 28,  0,  0,  0 }, // o
  { 32, 32, 60, 34, 34, 34, 34, 50, 44,  0,  0,  0 }, // p
  {  2,  2, 26, 38, 34, 34, 34, 34, 30,  0,  0,  0 }, // q
  {  0,  0, 16, 16, 16, 16, 16, 24, 22,  0,  0,  0 }, // r
  {  0,  0, 60,  2,  2, 28, 32, 32, 30,  0,  0,  0 }, // s
  {  0,  0, 14, 16, 16, 16, 16, 16, 62, 16, 16,  0 }, // t
  {  0,  0, 26, 38, 34, 34, 34, 34, 34,  0,  0,  0 }, // u
  {  0,  0,  8,  8, 20, 20, 34, 34, 34,  0,  0,  0 }, // v
  {  0,  0, 34, 34, 34, 85, 73, 73, 65,  0,  0,  0 }, // w
  {  0,  0, 34, 34, 20,  8, 20, 34, 34,  0,  0,  0 }, // x
  { 48, 16,  8,  8, 20, 20, 34, 34, 34,  0,  0,  0 }, // y
  {  0,  0, 62, 32, 16,  8,  4,  2, 62,  0,  0,  0 }, // z
  {  0,  6,  8,  8,  8,  4, 24,  4,  8,  8,  8,  6 }, // {
  {  0,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 }, // |
  {  0, 48,  8,  8,  8, 16, 12, 16,  8,  8,  8, 48 }, // }
  {  0,  0, 16, 40, 72,  4,  4,  2,  1,  0,  0,  0 },  // ~ v
  // iso-latin-1 norwegian letters
  {  0,  0, 59, 76, 72, 62,  9, 73, 54,  0,  0,  0 }, // ae
  {  0,  0, 92, 34, 50, 42, 42, 38, 29,  0,  0,  0 }, // oe
  {  0,  0, 29, 34, 34, 30,  2, 34, 28,  8, 20,  8 }, // aa
  {  0,  0, 79, 72, 72, 72,127, 72, 72, 72, 63,  0 }, // AE
  {  0,  0, 44, 18, 41, 41, 41, 37, 37, 18, 13,  0 }, // OE
  {  0,  0, 33, 33, 33, 63, 18, 18, 12, 12, 18, 12 }  // AA
};

// map from iso-latin1 to font data array index
static const int texturetext_isolatin1_mapping[] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   1,   2,   3,   4,   5,   6,   7,
    8,   9,  10,  11,  12,  13,  14,  15,
   16,  17,  18,  19,  20,  21,  22,  23,
   24,  25,  26,  27,  28,  29,  30,  31,
   32,  33,  34,  35,  36,  37,  38,  39,
   40,  41,  42,  43,  44,  45,  46,  47,
   48,  49,  50,  51,  52,  53,  54,  55,
   56,  57,  58,  59,  60,  61,  62,  63,
   64,  65,  66,  67,  68,  69,  70,  71,
   72,  73,  74,  75,  76,  77,  78,  79,
   80,  81,  82,  83,  84,  85,  86,  87,
   88,  89,  90,  91,  92,  93,  94,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0, 100,  98,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
   99,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,  97,  95,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
   96,   0,   0,   0,   0,   0,   0,   0
};

static SmTextureFont::FontImage * default_font = NULL;

/**************************************************************************/

SmTextureFont::FontImage::FontImage(const SbVec2s glyphsize_in,
                                    const int leading_in,
                                    const int ascent_in,
                                    const int descent_in,
                                    const int numcomp_in)
  : glyphsize(glyphsize_in),
    numcomp(numcomp_in),
    leading(leading_in),
    ascent(ascent_in),
    descent(descent_in)
{
  for (int i = 0; i < 256; i++) {
    this->glyphwidth[i] =  0;
    this->gfxglyphwidth[i] =  0;
  }
  // max glyphs = 256, sqrt(256) = 16
  int w = coin_geq_power_of_two(16 * glyphsize[0]);
  int h = coin_geq_power_of_two(16 * glyphsize[1]);

  this->setValue(SbVec2s(short(w), short(h)), numcomp, NULL);

  // initialize image to a blank image
  SbVec2s dummy;
  int dummy2;
  memset(this->getValue(dummy, dummy2), 0, w*h*numcomp);
  this->glimage = NULL;
}

SmTextureFont::FontImage::~FontImage()
{
  if (this->glimage) this->glimage->unref();
}

/*!

  Adds the image for a glyph to the texture image. The size of the
  image must be smaller than the \a glyphsize provided in the
  constructor. \a glyphwidth is the number of pixels to advance before
  rendering the next character.  \a gfxglyphwidth is the width of the
  glyph in the bitmap. If you supply a negative number in
  \a gfxglyphwidth, the width of the glyphs supplied in the constructor
  will be used instead.

  \a xoffset is the offset used when rendering the glyph. This is useful
  for specifying glyphs that extend to the left of the glyph position.
*/
void
SmTextureFont::FontImage::addGlyph(unsigned char c,
                               const SbImage & image,
                               const int glyphwidth,
                               const int gfxglyphwidth,
                               const int xoffset)
{
  if (this->glimage) {
    this->glimage->unref();
    this->glimage = NULL;
  }
  this->glyphwidth[c] = static_cast<short>(glyphwidth);
  this->xoffset[c] = static_cast<short>(xoffset);
  this->gfxglyphwidth[c] = gfxglyphwidth >= 0 ? short(gfxglyphwidth) : this->glyphsize[0];
  this->copyGlyph(c, image);
}

SbVec2s
SmTextureFont::FontImage::getGlyphPositionPixels(unsigned char c) const
{
  // FIXME: I'm wasting some space in the texture since loads of the
  // characters aren't printable, but doing it like this to keep it
  // simple
  short column = short(c) % 16;
  short row = short(c) / 16;
  return SbVec2s(column * this->glyphsize[0], row * this->glyphsize[1]);
}

/*!
  Returns the lower left corner position of the glyph in texture coordinates.
*/
SbVec2f
SmTextureFont::FontImage::getGlyphPosition(unsigned char c) const
{
  SbVec2s size;
  int dummy;
  (void) this->getValue(size, dummy);

  SbVec2s pos = this->getGlyphPositionPixels(c);
  return SbVec2f(float(pos[0])/float(size[0]),
                 float(pos[1])/float(size[1]));
}

/*!
  Returns the glyph image for \a c.
 */
SbImage
SmTextureFont::FontImage::getGlyphImage(const unsigned char c) const
{
  SbVec2s pos = this->getGlyphPositionPixels(c);

  SbImage image(NULL, this->glyphsize, this->numcomp);
  SbVec2s size, fullsize;
  int nc;

  unsigned char * dst = image.getValue(size, nc);
  const unsigned char * src = this->getValue(fullsize, nc);

  for (short y = 0; y < size[1]; y++) {
    for (short x = 0; x < size[0]; x++) {
      for (int c = 0; c < nc; c++) {
        *dst++ = src[(pos[1]+y)*fullsize[0]*nc + (pos[0]+x)*nc + c];
      }
    }
  }
  return image;
}

/*!
  Convenience method that calculates the width (in pixels) for \a s.
*/
int
SmTextureFont::FontImage::stringWidth(const SbString & s) const
{
  int acc = 0;
  const int len = s.getLength();
  const unsigned char * sptr =  reinterpret_cast<const unsigned char *>(s.getString());
  for (int i = 0; i < len; i++) {
    acc += this->getKerning(sptr[i], sptr[i+1]);
  }
  return acc;
}

/*!

  Convenience method to render \a s at position \a pos. This function
  assumed a coordinate system with 1 pixel == 1 unit is set up.

*/
void
SmTextureFont::FontImage::renderString(const SbString & s,
                                       const SbVec3f & pos,
                                       const bool needglbeginend) const
{
  const unsigned char * sptr =  reinterpret_cast<const unsigned char *>(s.getString());
  const int len = s.getLength();

  const SbVec2f n0(pos[0], pos[1]);
  const SbVec2f n1(n0[0], n0[1] + float(this->glyphsize[1]));

  if (needglbeginend) glBegin(GL_QUADS);

  int acc  = 0;
  for (int j = 0; j < len; j++) {
    int gw = this->getGlyphWidth(sptr[j]);
    int xoffset = this->getXOffset(sptr[j]);
    SbVec2f t0 = this->getGlyphPosition(sptr[j]);
    SbVec2f t1 = t0 + this->getGlyphSize(sptr[j]);

    float n00 = pos[0];
    SbVec3f c0(float(n00 + acc + xoffset),      float(n1[1]), -pos[2]);
    SbVec3f c1(float(n00 + acc + gw + xoffset), float(n1[1]), -pos[2]);
    SbVec3f c2(float(n00 + acc + gw + xoffset), float(n0[1]), -pos[2]);
    SbVec3f c3(float(n00 + acc + xoffset),      float(n0[1]), -pos[2]);

    acc += this->getKerning(sptr[j], sptr[j+1]);
    glTexCoord2f(t0[0], t0[1]);
    glVertex3fv(c0.getValue());
    glTexCoord2f(t1[0], t0[1]);
    glVertex3fv(c1.getValue());
    glTexCoord2f(t1[0], t1[1]);
    glVertex3fv(c2.getValue());
    glTexCoord2f(t0[0], t1[1]);
    glVertex3fv(c3.getValue());
  }
  if (needglbeginend) glEnd();
}

/*!
  Returns the size of the glyph, in texture coordinates.
*/
SbVec2f
SmTextureFont::FontImage::getGlyphSize(unsigned char c) const
{
  SbVec2s size;
  int dummy;
  (void) this->getValue(size, dummy);

  return SbVec2f(float(this->gfxglyphwidth[c]) / float(size[0]),
                 float(this->glyphsize[1]) / float(size[1]));
}

void
SmTextureFont::FontImage::copyGlyph(unsigned char c, const SbImage & glyph)
{
  SbVec2s origo = this->getGlyphPositionPixels(c);
  int nc;
  SbVec2s size, fullsize;
  const unsigned char * src = glyph.getValue(size, nc);
  unsigned char * dst = this->getValue(fullsize, nc);
  for (short y = 0; y < this->glyphsize[1]; y++) {
    for (short x = 0; x < this->glyphsize[0]; x++) {
      for (int i = 0; i < nc; i++) {
        dst[(origo[1]+y)*fullsize[0]*nc + (origo[0]+x)*nc + i] =
          src[y*size[0]*nc + x*nc + i];
      }
    }
  }
}

/*!
  Returns the width of the glyph in the texture.
*/
int
SmTextureFont::FontImage::getGlyphWidth(unsigned char c) const
{
  return this->gfxglyphwidth[c];
}

/*!
  Sets kerning for the glyph/next pair.
 */
void
SmTextureFont::FontImage::setKerning(unsigned char glyph, unsigned char next, int kerning)
{
  uintptr_t key = uintptr_t(glyph)<<8 | uintptr_t(next);
  void * val = reinterpret_cast<void*> (kerning);
  this->kerningdict.enter(key, val);
}

/*!
  Returns the kerning for glyph/next. If no kerning info exists for the pair,
  the width of the glyph is returned.
*/
int
SmTextureFont::FontImage::getKerning(unsigned char glyph, unsigned char next) const
{
  uintptr_t key = uintptr_t(glyph)<<8 | uintptr_t(next);
  void * val;
  if (this->kerningdict.find(key, val)) {
    return reinterpret_cast<uintptr_t> (val);
  }
  return this->glyphwidth[glyph];
}

/*!
  Returns the x offset for \a glyph.
*/
int
SmTextureFont::FontImage::getXOffset(unsigned char glyph) const
{
  return this->xoffset[glyph];
}

/*!
  Returns the SoGLImage instance for this font.
*/
SoGLImage *
SmTextureFont::FontImage::getGLImage(void) const
{
  if (this->glimage == NULL) {
    SmTextureFont::FontImage * thisp =
      const_cast<SmTextureFont::FontImage*> (this);
    thisp->glimage = new SoGLImage();
    SbVec2s size;
    int nc;
    const unsigned char * bytes = this->getValue(size, nc);
    thisp->glimage->setData(bytes, size, nc, SoGLImage::CLAMP, SoGLImage::CLAMP, 0.01f);
  }
  return this->glimage;
}

int
SmTextureFont::FontImage::getLeading() const
{
  return this->leading;
}

int
SmTextureFont::FontImage::getAscent() const
{
  return this->ascent;
}

int
SmTextureFont::FontImage::getDescent() const
{
  return this->descent;
}

/*!
  Returns the height of the font (ascent + descent + 1).
*/
int
SmTextureFont::FontImage::height() const
{
  return this->ascent + this->descent + 1;
}

const SbVec2s &
SmTextureFont::FontImage::getGlyphSizePixels() const
{
  return this->glyphsize;
}

SmTextureFont::FontImage::FontImage()
{
  assert(0 && "shouldn't be used");
}

/**************************************************************************/

SO_NODE_SOURCE(SmTextureFont);

static void
render_text(unsigned char * dst,
           const int idx,
           const unsigned char value,
           const unsigned char alpha)
{
  const unsigned char * src = texture_fontdata[idx];

  const int sx = 1;
  for (int y = 0; y < 12; y++) {
    for (int x = sx; x < 8; x++) {
      int dstidx = (11-y)*8*2 + (x-sx)*2;
      if (src[y] & (0x80 >> x)) {
        dst[dstidx] = value;
        dst[dstidx+1] = alpha;
      }
      else {
        dst[dstidx] = 0;
        dst[dstidx+1] = 0;
      }
    }
  }
}

void
SmTextureFont::initClass(void)
{
  SO_NODE_INIT_CLASS(SmTextureFont, SoNode, "Node");

  SO_ENABLE(SoGLRenderAction, SmTextureFontElement);
  SO_ENABLE(SoPickAction, SmTextureFontElement);
  SO_ENABLE(SoGetBoundingBoxAction, SmTextureFontElement);

  default_font = new SmTextureFont::FontImage(SbVec2s(8,12),
                                              0,
                                              8,
                                              3,
                                              2);
  SbImage img(NULL, SbVec2s(8,12), 2);
  for (size_t c = 0; c < sizeof(texturetext_isolatin1_mapping) / sizeof(int); c++) {
    SbVec2s dummy;
    int dummy2;
    render_text(img.getValue(dummy, dummy2),
              texturetext_isolatin1_mapping[c], 255, 255);
    default_font->addGlyph(c, img, 7, 8);
  }
  cc_coin_atexit(destroyClass);
}

void
SmTextureFont::destroyClass()
{
  delete default_font;
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
                            const_cast<FontImage*>(this->getFont()));
}

void
SmTextureFont::GLRender(SoGLRenderAction * action)
{
  SmTextureFont::doAction(static_cast<SoAction *>(action));
}

void
SmTextureFont::pick(SoPickAction * action)
{
  SmTextureFont::doAction(static_cast<SoAction *>(action));
}

void
SmTextureFont::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SmTextureFont::doAction(static_cast<SoAction *>(action));
}

void
SmTextureFont::setFont(FontImage * image)
{
  this->touch();
  delete this->image;
  this->image = image;
}

const SmTextureFont::FontImage *
SmTextureFont::getFont(void) const
{
  return this->image ? this->image : default_font;
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
  this->image = default_font;
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

  if (image) {
    elem->image = image;
  }
  else {
    elem->image = default_font;
  }
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

#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/elements/SoGLTextureImageElement.h>
#include <Inventor/elements/SoGLTextureCoordinateElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoLightModelElement.h>

SmTextureFontBundle::SmTextureFontBundle(SoAction * action, SoNode * node_in)
  : state(action->getState()),
    node(node_in),
    didupdatecoin(false),
    font(SmTextureFontElement::get(action->getState()))
{
  if (this->node == NULL) this->node = action->getCurPath()->getTail();
  this->state->push();
  // update these elements here to make blending work
  SoTextureQualityElement::set(this->state, 0.3f);
  SoGLTextureImageElement::set(this->state, this->node,
                            this->font->getGLImage(),
                            SoTextureImageElement::MODULATE,
                            SbColor(1.0f, 1.0f, 1.0f));
  SoGLTextureEnabledElement::set(this->state, this->node, TRUE);
}

SmTextureFontBundle::~SmTextureFontBundle()
{
  this->state->pop();
}

void
SmTextureFontBundle::begin() const
{
  if (!this->didupdatecoin) {
    // turn off any texture coordinate functions
    SoLazyElement::setVertexOrdering(this->state, SoLazyElement::CCW);
    SoGLTextureCoordinateElement::setTexGen(this->state, this->node, NULL);
    SoLightModelElement::set(this->state, SoLightModelElement::BASE_COLOR);
    SoGLLazyElement::getInstance(this->state)->send(this->state,
                                              SoLazyElement::VERTEXORDERING_MASK|
                                              SoLazyElement::LIGHT_MODEL_MASK);
    const_cast<SmTextureFontBundle*> (this)->didupdatecoin = true;
  }
  glBegin(GL_QUADS);
}

void
SmTextureFontBundle::end() const
{
  glEnd();
}

void
SmTextureFontBundle::renderString(const SbString & string,
                              const SbVec3f & pos) const
{
  this->font->renderString(string, pos, false);
}


/**************************************************************************/
