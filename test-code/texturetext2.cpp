
// Generate lots of texts for testing SoTextureText2 performance.
// Usage: texturetext2 [numtexts] > out.iv
//

#include <cstdio>
#include <cstdlib>
#include <Inventor/SbVec3f.h>

int main(int argc, char ** argv)
{
  int num = 500;
  if (argc > 1) num = atoi(argv[1]);

  srand(0);

  printf("#Inventor V2.1 ascii\n"
         "DrawStyle { style LINES }\n" 
         "Cube { }\n"
         "DrawStyle { style FILLED }\n"
         "MaterialBinding { value PER_PART }\n"
         "Material { diffuseColor [\n");


  for (int i = 0; i < num; i++) {
    SbVec3f pos(float(rand()) / RAND_MAX,
                float(rand()) / RAND_MAX,
                float(rand()) / RAND_MAX);
    pos *= 0.5f;
    pos += SbVec3f(0.5f, 0.5f, 0.5f);


    if (i == num-1) 
      printf("    %g %g %g ]\n", 
             pos[0], pos[1], pos[2]);
    else
      printf("    %g %g %g,\n",
             pos[0], pos[1], pos[2]);
    
  }
  printf("}\n");

  printf("Complexity { textureQuality 1.0 }\n"
         "SmTextureText2 {\n"
         "  position [\n");
  
  for (int i = 0; i < num; i++) {
    SbVec3f pos(float(rand()) / RAND_MAX,
                float(rand()) / RAND_MAX,
                float(rand()) / RAND_MAX);
    pos -= SbVec3f(0.5f, 0.5f, 0.5f);
    pos *= 0.8f;
    
    if (i == num-1) 
      printf("    %g %g %g ]\n", 
             pos[0], pos[1], pos[2]);
    else
      printf("    %g %g %g,\n",
             pos[0], pos[1], pos[2]);
  }


  printf("  string [\n");
  for (int i = 0; i < num; i++) {
    if (i == num-1) 
      printf("    \"SIM %d\" ]\n", i+1);
    else 
      printf("    \"SIM %d\",\n", i+1);
  }
  printf("}\n");
}

