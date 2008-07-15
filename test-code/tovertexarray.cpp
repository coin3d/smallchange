#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCoordinate4.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/SoInteraction.h>
#include <SmallChange/misc/Init.h>
#include <cassert>
#include <cstdio>
#include <SmallChange/actions/SmToVertexArrayShapeAction.h>
#include <Inventor/nodekits/SoBaseKit.h>

static void strip_node(SoType type, SoNode * root)
{
  SoSearchAction sa;
  sa.setType(type);
  sa.setSearchingAll(TRUE);
  sa.setInterest(SoSearchAction::ALL);
  sa.apply(root);

  SoPathList & pl = sa.getPaths();
  for (int i = 0; i < pl.getLength(); i++) {
    SoFullPath * p = (SoFullPath*) pl[i];
    if (p->getTail()->isOfType(type)) {
      SoGroup * g = (SoGroup*) p->getNodeFromTail(1);
      g->removeChild(p->getIndexFromTail(0));
    }
  }
  sa.reset();  
}

int
main(int argc, char ** argv )
{
  if (argc < 3) {
    fprintf(stderr,"Usage: tovertexarray <infile> <outfile> [nostrip]\n");
    return -1;
  }

  SbBool strip = TRUE;
  if (argc > 3) {
    if (strcmp(argv[3], "nostrip") == 0) strip = FALSE;
    else {
      fprintf(stderr,"Usage: tovertexarray <infile> <outfile> [nostrip]\n");
      return -1;
    }
  }

  SoDB::init();
  SoInteraction::init();
  SoBaseKit::setSearchingChildren(TRUE);
  smallchange_init();
    
  SoInput input;
  SbBool ok = input.openFile(argv[1]);
  if (!ok) {
    fprintf(stderr,"Unable to open file.\n");
    return -1;
  }
  SoSeparator * root = SoDB::readAll(&input); 
  
  SbBool vrml1 = input.isFileVRML1();
  SbBool vrml2 = input.isFileVRML2();

  if (vrml2) {
    fprintf(stderr,"VRML2 not supported yet\n");
    return -1;
  }
  
  if (!root) {
    fprintf(stderr,"Unable to read file.\n");
    return -1;
  }
  root->ref();

  fprintf(stderr,"Applying SmToVertexArrayShapeAction...");
  SmToVertexArrayShapeAction tova;
  tova.apply(root);
  fprintf(stderr,"done\n");

  SbBool binary = FALSE;

  SoOutput out;
  if (binary) out.setBinary(TRUE);
  if (out.openFile(argv[2])) {
    if (binary) {
      out.setHeaderString("#Inventor V2.1 binary");
    }
    if (strip) { // strip coord3, texcoord and normal nodes
      fprintf(stderr,"stripping scene graph\n");
      strip_node(SoCoordinate3::getClassTypeId(), root);
      strip_node(SoCoordinate4::getClassTypeId(), root);
      strip_node(SoNormal::getClassTypeId(), root);
      strip_node(SoTextureCoordinate2::getClassTypeId(), root);
    }
    fprintf(stderr,"writing target\n");
    SoWriteAction wa(&out);
    wa.apply(root);
  }
 
  root->unref();
  return 0;
} // main()

