#ifndef COIN_SOLISTENER_H
#define COIN_SOLISTENER_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSfVec3f.h>
#include <Inventor/fields/SoSfRotation.h>
#include <Inventor/fields/SoSfFloat.h>

#include "SoAudioRenderAction.h"

class SoListener : public SoNode
{
  SO_NODE_HEADER(SoListener);

  friend class SoAudioRenderAction;

public:

  static void initClass();
  SoListener();

  SoSFVec3f position;
  SoSFRotation orientation;
  SoSFVec3f velocity;
  SoSFFloat gain;

protected:

  virtual void audioRender(SoAudioRenderAction *action);
//  virtual void GLRender(SoGLRenderAction *action);

private:

  virtual ~SoListener();

};

#endif // COIN_SOLISTENER_H
