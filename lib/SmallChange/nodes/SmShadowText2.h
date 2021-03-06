#ifndef SMALLCHANGE_SOSHADOWTEXT2_H
#define SMALLCHANGE_SOSHADOWTEXT2_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFVec2f.h>

#include <SmallChange/basic.h>

class SmShadowText2P;

class SMALLCHANGE_DLL_API SmShadowText2 : public SoText2 {
  typedef SoShape inherited;

  SO_NODE_HEADER(ShadowText2);

public:
  static void initClass(void);
  SmShadowText2(void);

  enum Justification {
    LEFT = 1,
    RIGHT,
    CENTER
  };

  SoSFVec2f pixelOffset;

  virtual void GLRender(SoGLRenderAction * action);

protected:
  virtual ~SmShadowText2();

private:
  class SmShadowText2P * pimpl;
  friend class SmShadowText2P;
};

#endif // !SMALLCHANGE_SOSHADOWTEXT2_H
