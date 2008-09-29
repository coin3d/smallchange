/*!
  \class ShadowText2
  \brief The ShadowText2 class draws white text with a black border.
*/

/*!
  \var SoSFVec2f ShadowText2::pixelOffset

  Offset text placement, in pixels, relative to the viewport position calculated
  from the transformation matrix. Default (0,0).
*/

#include "SmShadowText2.h"
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoDrawStyleElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoFontNameElement.h>
#include <Inventor/elements/SoFontSizeElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbLine.h>

#include <climits>
#include <cstring>

static unsigned char painter_fontdata[][12] = {
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
static int painter_isolatin1_mapping[] = {
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


SO_NODE_SOURCE(SmShadowText2);

/*!
  Constructor.
*/
SmShadowText2::SmShadowText2(void)
{
  SO_NODE_CONSTRUCTOR(SmShadowText2);
  SO_NODE_ADD_FIELD(pixelOffset, (0.0f, 0.0f));
}

/*!
  Destructor.
*/
SmShadowText2::~SmShadowText2()
{
}


// doc in super
void
SmShadowText2::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(SmShadowText2, SoText2, "Text2");
  }
}

// doc in super
void
SmShadowText2::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  SoState * state = action->getState();

  // ignore when rendering wireframe overlay
  if (SoDrawStyleElement::get(state) != SoDrawStyleElement::FILLED) return;

  state->push();

  SoLightModelElement::set(state, this, SoLightModelElement::BASE_COLOR);

  SoMaterialBundle mb(action);
  mb.sendFirst();


  SbVec3f nilpoint(0.0f, 0.0f, 0.0f);
  const SbMatrix & mat = SoModelMatrixElement::get(state);
  mat.multVecMatrix(nilpoint, nilpoint);
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  // this function will also modify the z-value of nilpoint
  // according to the view matrix
  vv.projectToScreen(nilpoint, nilpoint);
  // change z-range from [0,1] to [-1,1]
  nilpoint[2] *= 2.0f;
  nilpoint[2] -= 1.0f;

  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();
  nilpoint[0] = nilpoint[0] * float(vpsize[0]);
  nilpoint[1] = nilpoint[1] * float(vpsize[1]);
  // Set new state.
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  nilpoint[0] += this->pixelOffset.getValue()[0];
  nilpoint[1] += this->pixelOffset.getValue()[1];

  float xpos, ypos;
  int strwidth;

  xpos = nilpoint[0];      // to get rid of compiler warning..
  ypos = nilpoint[1];

  glDepthFunc(GL_LEQUAL);

  for (int i = 0; i < this->string.getNum(); i++) {
    const unsigned char *s = (const unsigned char *)this->string[i].getString();
    strwidth = strlen((const char*)s);
    switch (this->justification.getValue()) {
    case SmShadowText2::LEFT:
      xpos = nilpoint[0];
      break;
    case SmShadowText2::RIGHT:
      xpos = nilpoint[0] - (strwidth * 8.0f);
      break;
    case SmShadowText2::CENTER:
      xpos = nilpoint[0] - (strwidth * 8.0f)/2.0f;
      break;
    }
    for (int i2 = 0; i2 < strwidth; i2++) {

      int idx = painter_isolatin1_mapping[s[i2]];

      if (idx) {
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos3f(xpos+1, ypos-1, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);
        glRasterPos3f(xpos-1, ypos-1, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);
        glRasterPos3f(xpos+1, ypos+1, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);
        glRasterPos3f(xpos-1, ypos+1, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);

        glRasterPos3f(xpos+1, ypos, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);
        glRasterPos3f(xpos-1, ypos, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);

        glRasterPos3f(xpos, ypos+1, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);
        glRasterPos3f(xpos, ypos-1, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);

        // use current state color, not white
        mb.forceSend(0);
        // glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos3f(xpos, ypos, -nilpoint[2]);
        glBitmap(8,12,0,0,0,0,(const GLubyte *)painter_fontdata + 12 * idx);
      }
      xpos += 8.0f;
    }
    // - instead of + because of OpenGL's "inverted" y coordinate...
    ypos -= ( 12.0f * this->spacing.getValue() );
  }

  SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::DIFFUSE_MASK);
  glDepthFunc(GL_LESS);

  state->pop();

  glPixelStorei(GL_UNPACK_ALIGNMENT,4);
  // Pop old GL state.
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}
