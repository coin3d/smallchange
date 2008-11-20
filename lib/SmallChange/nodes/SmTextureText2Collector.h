#ifndef SMTEXTURETEXT2COLLECTOR_H
#define SMTEXTURETEXT2COLLECTOR_H

/**************************************************************************/

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoGroup.h>

#include <SmallChange/basic.h>

/**************************************************************************/

class SMALLCHANGE_DLL_API SmTextureText2Collector : public SoGroup {
  typedef SoGroup inherited;

  SO_NODE_HEADER(SmTextureText2Collector);
  
 public:
  static void initClass(void);
  SmTextureText2Collector(void);
  
  virtual void GLRender(SoGLRenderAction * action);

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
  virtual void init(SoState * state);
  static void startCollecting(SoState * state); 
  
  static void add(SoState * state,
		  const char * text,
		  const SmTextureFont::FontImage * font,
		  const SbVec3f & worldpos,
		  const SbColor4f & color,
		  SmTextureText2::Justification j,
		  SmTextureText2::VerticalJustification vj);
  
  static void finishCollecting(SoState * state);
  static bool isCollecting(SoState * state);

  virtual SbBool matches(const SoElement * elt) const;
  virtual SoElement * copyMatchInfo(void) const;
  
 private:
  typedef struct {
    const char * text;
    const SmTextureFont::FontImage * font;
    SbVec3f worldpos;
    SbColor4f color;
    SmTextureText2::Justification justification;
    SmTextureText2::VerticalJustification vjustification;
  } TextItem;
  
  bool collecting;
  std::vector <TextItem> items;
};

/**************************************************************************/

#endif // SMTEXTURETEXT2COLLECTOR_H
