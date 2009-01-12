#ifndef SMTEXTURETEXT2COLLECTOR_H
#define SMTEXTURETEXT2COLLECTOR_H

/**************************************************************************/

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/fields/SoSFBool.h>

#include <SmallChange/basic.h>

/**************************************************************************/

class SMALLCHANGE_DLL_API SmTextureText2Collector : public SoSeparator {
  typedef SoSeparator inherited;

  SO_NODE_HEADER(SmTextureText2Collector);

 public:
  SoSFBool depthMask;

  static void initClass(void);
  SmTextureText2Collector(void);

  virtual void GLRenderBelowPath(SoGLRenderAction * action);
  virtual void GLRenderInPath(SoGLRenderAction * action);

 protected:
  virtual ~SmTextureText2Collector();

 private:
};

/**************************************************************************/

#include <vector>
#include <Inventor/elements/SoSubElement.h>
#include <Inventor/elements/SoElement.h>
#include <Inventor/SbColor4f.h>

#include <SmallChange/nodes/SmTextureFont.h>
#include <SmallChange/nodes/SmTextureText2.h>

class SMALLCHANGE_DLL_API SmTextureText2CollectorElement : public SoElement {
  typedef SoElement inherited;

  SO_ELEMENT_HEADER(SmTextureText2CollectorElement);
 public:
  static void initClass(void);
 protected:
  virtual ~SmTextureText2CollectorElement();

 public:

  typedef struct {
    SbString text;
    const SmTextureFont::FontImage * font;
    SbVec3f worldpos;
    float maxdist;
    SbColor4f color;
    SmTextureText2::Justification justification;
    SmTextureText2::VerticalJustification vjustification;
  } TextItem;


  virtual void init(SoState * state);
  static void startCollecting(SoState * state, const bool storeitems = true);

  static void add(SoState * state,
                const SbString & text,
                const SmTextureFont::FontImage * font,
                const SbVec3f & worldpos,
                  const float maxdist,
                const SbColor4f & color,
                SmTextureText2::Justification j,
                SmTextureText2::VerticalJustification vj);

  static const std::vector <TextItem> &  finishCollecting(SoState * state);
  static bool isCollecting(SoState * state);

  virtual SbBool matches(const SoElement * elt) const;
  virtual SoElement * copyMatchInfo(void) const;

 private:
  bool collecting;
  bool storeitems;
  std::vector <TextItem> items;
};

/**************************************************************************/

#endif // SMTEXTURETEXT2COLLECTOR_H
