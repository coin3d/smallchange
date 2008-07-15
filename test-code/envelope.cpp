#include <SmallChange/misc/SmEnvelope.h>
#include <SmallChange/misc/Init.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/SoDB.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>

int main(int argc, char ** argv)
{
  if (argc < 3) {
    (void)fprintf(stderr, "\nUsage: envelope <infile> <outfile> [octreelevel]\n");
    return 1;
  }
  int level = 0;
  if (argc > 3) {
    level = atoi(argv[3]);
  }
  SoDB::init();
  SoInteraction::init();
  smallchange_init();

  
  SmEnvelope env;
  env.importFile(argv[1]);
  env.exportGeometry(argv[2], level, TRUE);

  return 0;
}

