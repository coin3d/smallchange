#ifndef COIN_SOAUDIORENDERACTION_H
#define COIN_SOAUDIORENDERACTION_H

#include <Inventor/actions/SoSubAction.h>

class SoAudioRenderAction : public SoAction
{
  SO_ACTION_HEADER(SoAudioRenderAction);

public:
  static void initClass();
  SoAudioRenderAction();
  virtual ~SoAudioRenderAction();

protected:
  virtual void beginTraversal(SoNode *node);

private:
  static void callDoAction(SoAction *action, SoNode *node);
  static void callAudioRender(SoAction *action, SoNode *node);
};

#endif