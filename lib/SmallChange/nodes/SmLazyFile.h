#ifndef SMALLCHANGE_LAZYFILE_H
#define SMALLCHANGE_LAZYFILE_H

#include <Inventor/nodes/SoFile.h>
#include <SmallChange/basic.h>
#include <Inventor/SbBox3f.h>

class SoInput;
class SoAction;
class SoGLRenderAction;
class SmLazyFileP;

class SMALLCHANGE_DLL_API SmLazyFile : public SoFile {
  typedef SoFile inherited;

  SO_NODE_HEADER(SmLazyFile);

public:
  static void initClass(void);
  SmLazyFile(void);

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);

protected:
  virtual SbBool readNamedFile(SoInput *);
  virtual SbBool readInstance(SoInput * in, unsigned short flags);

private:
  virtual ~SmLazyFile(void);

  SmLazyFileP * pimpl;
};

#endif // SMALLCHANGE_LAZYFILE_H
