
#include <stdio.h>

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/SoInteraction.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/actions/SoWriteAction.h>

#include <SmallChange/actions/SoGenerateSceneGraphAction.h>

int
main(int argc, char ** argv)
{
  if ( argc != 3 ) {
    fprintf(stderr, "Usage: %s <infile.iv> <outfile.iv>\n", argv[0]);
    return -1;
  }

  SoDB::init();
  SoNodeKit::init();
  SoInteraction::init();

  SoGenerateSceneGraphAction::initClass();

  SoInput in;
  SoNode * scene, * graph;
  if ( !in.openFile(argv[1]) ) {
    fprintf(stderr, "%s: error opening \"%s\" for reading.\n", argv[0], argv[1]);
    return -1;
  }
  scene = SoDB::readAll(&in);
  if ( scene == NULL ) {
    fprintf(stderr, "%s: error parsing \"%s\"\n", argv[0], argv[1]);
    return -1;
  }
  scene->ref();

  SoGenerateSceneGraphAction action;
  action.apply(scene);
  graph = action.getGraph();
  if ( graph == NULL ) {
    fprintf(stderr, "%s: error generating scene graph\n", argv[0]);
    return -1;
  }
  graph->ref();
  scene->unref();

  SoOutput out;
  if ( !out.openFile(argv[2]) ) {
    fprintf(stderr, "%s: error opening \"%s\" for writing.\n", argv[0], argv[2]);
    return -1;
  }
  SoWriteAction writer(&out);
  writer.apply(graph);
  graph->unref();
  return 0;
}

