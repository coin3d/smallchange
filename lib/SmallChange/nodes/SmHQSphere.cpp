#include <stddef.h>
#include <string.h>
#include <Inventor/SbBSPTree.h>
#include <Inventor/lists/SbList.h>

class HQSphereGenerator {
public:
  
  HQSphereGenerator(void) {
    this->icosahedron = NULL;
  }
  ~HQSphereGenerator(void) {
    delete this->icosahedron;
  }
    
  class triangle {
  public:
    triangle(void) { }
    triangle(const SbVec3f & p0, const SbVec3f & p1, const SbVec3f & p2) {
      pt[0] = p0;
      pt[1] = p1;
      pt[2] = p2;
    } 
  public:
    SbVec3f pt[3];
  };
  
  class object {
  public:
    object(int npoly, const triangle * poly) {
      this->npoly = npoly;
      this->poly = new triangle[npoly];
      if (poly) {
        memcpy(this->poly, poly, npoly*sizeof(triangle));
      }
    }
    ~object() {
      delete[] this->poly;
    }
  public:
    int npoly;    /* # of triangles in object */
    triangle * poly;     /* Triangles */
  };

  void generate(const int level, SbBSPTree & bsp, SbList <int> & idx);
  
  SbVec3f normalize(const SbVec3f & p);
  SbVec3f midpoint(const SbVec3f & a, const SbVec3f & b);

private:
  void convert(object * obj, SbBSPTree & bsp, SbList <int> & idx);
  void init(void);
  object * icosahedron;
};

void
HQSphereGenerator::init(void)
{  
  /* Twelve vertices of icosahedron on unit sphere */
  const float tau = 0.8506508084f; /* t=(1+sqrt(5))/2, tau=t/sqrt(1+t^2)  */
  const float one =0.5257311121f; /* one=1/sqrt(1+t^2) , unit sphere     */

  const SbVec3f ZA(tau, one, 0.0f);
  const SbVec3f ZB(-tau, one, 0.0f);
  const SbVec3f ZC(-tau, -one, 0.0f);
  const SbVec3f ZD(tau, -one, 0.0f);
  const SbVec3f YA(one, 0.0f ,  tau);
  const SbVec3f YB(one, 0.0f , -tau);
  const SbVec3f YC(-one, 0.0f , -tau);
  const SbVec3f YD(-one, 0.0f ,  tau);
  const SbVec3f XA(0.0f, tau, one);
  const SbVec3f XB(0.0f, -tau, one);
  const SbVec3f XC(0.0f, -tau, -one);
  const SbVec3f XD(0.0f, tau, -one);

  /* Structure for unit icosahedron */
  const triangle triangles[] = {
    triangle(YA, XA, YD),
    triangle(YA, YD, XB),
    triangle(YB, YC, XD),
    triangle(YB, XC, YC),
    triangle(ZA, YA, ZD),
    triangle(ZA, ZD, YB),
    triangle(ZC, YD, ZB),
    triangle(ZC, ZB, YC),
    triangle(XA, ZA, XD),
    triangle(XA, XD, ZB),
    triangle(XB, XC, ZD),
    triangle(XB, ZC, XC),
    triangle(XA, YA, ZA),
    triangle(XD, ZA, YB),
    triangle(YA, XB, ZD),
    triangle(YB, ZD, XC),
    triangle(YD, XA, ZB),
    triangle(YC, ZB, XD),
    triangle(YD, ZC, XB),
    triangle(YC, XC, ZC)
  };
  this->icosahedron = 
    new object(20, triangles);
};

void
HQSphereGenerator::generate(const int maxlevel,
                            SbBSPTree & bsp,
                            SbList <int> & idx)
{
  int level;
  
  object * old = this->icosahedron;

  /* Subdivide each starting triangle (maxlevel - 1) times */
  for (level = 1; level < maxlevel; level++) {
    /* Allocate a new object */
    /* FIXME: Valgrind reports an 8-byte memory leak here. 20030404 mortene. */
    object * newobj = new object(old->npoly * 4, NULL);
      
    /* Subdivide each triangle in the old approximation and normalize
     *  the new points thus generated to lie on the surface of the unit
     *  sphere.
     * Each input triangle with vertices labelled [0,1,2] as shown
     *  below will be turned into four new triangles:
     *
     *                      Make new points
     *                          a = (0+2)/2
     *                          b = (0+1)/2
     *                          c = (1+2)/2
     *        1
     *       /\             Normalize a, b, c
     *      /  \
     *    b/____\ c         Construct new triangles
     *    /\    /\              [0,b,a]
     *   /  \  /  \             [b,1,c]
     *  /____\/____\            [a,b,c]
     * 0      a     2           [a,c,2]
     */
    for (int i = 0; i < old->npoly; i++) {
      triangle *oldt = &old->poly[i];
      triangle *newt = &newobj->poly[i*4];
      SbVec3f a, b, c;
      
      a = normalize(midpoint(oldt->pt[0], oldt->pt[2]));
      b = normalize(midpoint(oldt->pt[0], oldt->pt[1]));
      c = normalize(midpoint(oldt->pt[1], oldt->pt[2]));
        
      newt->pt[0] = oldt->pt[0];
      newt->pt[1] = b;
      newt->pt[2] = a;
      newt++;
        
      newt->pt[0] = b;
      newt->pt[1] = oldt->pt[1];
      newt->pt[2] = c;
      newt++;
      
      newt->pt[0] = a;
      newt->pt[1] = b;
      newt->pt[2] = c;
      newt++;
      
      newt->pt[0] = a;
      newt->pt[1] = c;
      newt->pt[2] = oldt->pt[2];
    }
    
    if (old != this->icosahedron) delete old;
    /* Continue subdividing new triangles */
    old = newobj;
  }
  this->convert(old, bsp, idx);
  if (old != this->icosahedron) delete old;
}

SbVec3f 
HQSphereGenerator::normalize(const SbVec3f & p)
{
  float mag;
  
  SbVec3f r = p;
  mag = r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
  if (mag != 0.0f) {
    mag = (float) (1.0 / sqrt(mag));
    r[0] *= mag;
    r[1] *= mag;
    r[2] *= mag;
  }
  return r;
}

/* Return the midpoint on the line between two points */
SbVec3f 
HQSphereGenerator::midpoint(const SbVec3f & a, const SbVec3f & b)
{
  SbVec3f r;
  
  r[0] = (a[0] + b[0]) * 0.5f;
  r[1] = (a[1] + b[1]) * 0.5f;
  r[2] = (a[2] + b[2]) * 0.5f;
  
  return r;
}

void 
HQSphereGenerator::convert(object * obj, SbBSPTree & bsp, SbList <int> & idx)
{
  triangle * t = obj->poly;
  int n = obj->npoly;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < 3; j++) {
      idx.append(bsp.addPoint(t->pt[j]));
    }
    idx.append(-1);
    t++;
  }
}

