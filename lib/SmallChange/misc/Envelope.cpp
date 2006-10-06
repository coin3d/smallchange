#include "SmEnvelope.h"

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPackedColor.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoLOD.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/SbBSPTree.h>
#include "SmHash.h"
#include <Inventor/VRMLnodes/SoVRMLIndexedFaceSet.h>
#include <Inventor/VRMLnodes/SoVRMLImageTexture.h>
#include <Inventor/VRMLnodes/SoVRMLShape.h>
#include <Inventor/VRMLnodes/SoVRMLAppearance.h>
#include <Inventor/elements/SoTextureEnabledElement.h>
#include <Inventor/SoInteraction.h>
#include <assert.h>
#include <string.h>

// helper class
class sm_meshattrib {
public:
  sm_meshattrib() : 
    ambient(0.2f, 0.2f, 0.2f),
    diffuse(0.8f, 0.8f, 0.8f),
    specular(0.0f, 0.0f, 0.0f),
    emissive(0.0f, 0.0f, 0.0f),
    shininess(0.2f),
    transparency(0.0f),
    texturename(NULL),
    vordering(SoShapeHints::UNKNOWN_ORDERING),
    shapetype(SoShapeHints::UNKNOWN_SHAPE_TYPE) { }


  int levels;
  // material
  SbColor ambient;
  SbColor diffuse;
  SbColor specular;
  SbColor emissive;
  float shininess;
  float transparency;
  const char * texturename;

  // shape hints
  SoShapeHints::VertexOrdering vordering;
  SoShapeHints::ShapeType shapetype;

  // needed for SmHash
  operator unsigned long(void) const;
  int operator==(const sm_meshattrib & v) const;
};

class sm_mesh {
public:
  sm_mesh() : colorpervertex(FALSE) { }

  sm_mesh(const sm_mesh & org) {
    colorpervertex = org.colorpervertex;
    attrib = org.attrib;
  }

  sm_meshattrib attrib;
  SbBool colorpervertex;
  
  SbBSPTree bsp;
  SbBSPTree nbsp;
  SbBSPTree tbsp;
  SbBSPTree cbsp;

  SbList <int32_t> vidx;
  SbList <int32_t> nidx;
  SbList <int32_t> tidx; 
  SbList <int32_t> cidx;

  // for lines
  SbList <int32_t> lidx;

  sm_mesh * split_mesh(const SbBox3f & bbox);
  SoSeparator * create_iv_mesh(void);

};

class SmEnvelopeP {
public:
  SmEnvelopeP() {
    this->texfilename = NULL;
    this->search = NULL;
    this->firsttriangle = TRUE;
    this->numdiffuse = 0;
    this->diffuseptr = NULL;
    this->numtransp = 0;
    this->packedptr = NULL;
    this->transpptr = NULL;
    this->vrmlifs = NULL;
  }

  SbBox3f bbox;
  
  SmHash <sm_mesh *, sm_meshattrib> hash;
  SbList <sm_mesh*> meshlist;

  SoVRMLIndexedFaceSet * vrmlifs;
  
  const char * texfilename;
  SoSearchAction * search;
  SbBool firsttriangle;
  
  int numdiffuse;
  const SbColor * diffuseptr;
  int numtransp;
  const uint32_t * packedptr;
  const float * transpptr; 
  uint32_t firstcolor;
  
  SbList <const char *> alltextures;
  
  void add_texture_filename(const char * name) 
  {
    for (int i = 0; i < alltextures.getLength(); i++) {
      if (alltextures[i] == name) return;
    }
    alltextures.append(name);
  }

  static SoCallbackAction::Response 
  pre_shape_cb(void * userdata,
               SoCallbackAction * action,
               const SoNode * node)
  {
    SmEnvelopeP * thisp = (SmEnvelopeP*) userdata;

    SoState * state = action->getState();
    thisp->firsttriangle = TRUE;
    
    if (node->isOfType(SoVRMLIndexedFaceSet::getClassTypeId())) {
      thisp->vrmlifs = (SoVRMLIndexedFaceSet*) node;
    }
    else {
      thisp->vrmlifs = NULL;
    }
    
    thisp->texfilename = NULL;
    if (1 /*SoTextureEnabledElement::get(state)*/) {
      SoPath * path = action->getCurPath()->copy();
      path->ref();
      
      thisp->search->setType(SoVRMLShape::getClassTypeId());
      thisp->search->setInterest(SoSearchAction::LAST);
      thisp->search->setSearchingAll(FALSE);
      thisp->search->apply(path);
      
      SbBool vrmltex = thisp->search->getPath() != NULL;
      
      if (!vrmltex) {
        thisp->search->setType(SoTexture2::getClassTypeId());
        thisp->search->setInterest(SoSearchAction::LAST);
        thisp->search->setSearchingAll(FALSE);
        thisp->search->apply(path);
      }
      path->unref();
      
      if (thisp->search->getPath()) {
        SoFullPath * p = (SoFullPath*) thisp->search->getPath();
        SbName name("");
        if (vrmltex) {
          SoVRMLAppearance * a = (SoVRMLAppearance*) ((SoVRMLShape*)p->getTail())->appearance.getValue();
          if (a) {
            SoNode * n = a->texture.getValue();
            if (n && n->isOfType(SoVRMLImageTexture::getClassTypeId())) {
              SoVRMLImageTexture * t = (SoVRMLImageTexture*) n;
              if (t->url.getNum()) {
                name = t->url[0].getString();
              }
            }
          }
        }
        else {
          SoTexture2 * t = (SoTexture2*) p->getTail();
          name = t->filename.getValue().getString();
          // t->image.touch();
        }
        if (name.getLength()) {
          thisp->texfilename = name.getString();
          fprintf(stderr,"name: %s\n", thisp->texfilename);
        }
        else {
          thisp->texfilename = NULL;
        }
      }
      else {
        fprintf(stderr,"no texture found\n");
      }
      thisp->search->reset();
    }
    else {
      fprintf(stderr,"no tex\n");
    }
    if (thisp->texfilename) thisp->add_texture_filename(thisp->texfilename);
    
    return SoCallbackAction::CONTINUE;
  }
  
  void initfirst(SoAction * action)
  {
    this->firsttriangle = FALSE;
    SoState * state = action->getState();
    SoLazyElement * lelem = SoLazyElement::getInstance(state);
    
    this->numdiffuse = lelem->getNumDiffuse();
    this->numtransp = lelem->getNumTransparencies();
    if (lelem->isPacked()) {
      this->packedptr = lelem->getPackedPointer();
      this->diffuseptr = NULL;
      this->transpptr = NULL;
    }
    else {
      this->packedptr = NULL;
      this->diffuseptr = lelem->getDiffusePointer();
      this->transpptr = lelem->getTransparencyPointer();
    }
    // just store diffuse color with index 0
    uint32_t col;
    if (this->packedptr) {
      col = this->packedptr[0];
    }
    else {
      SbColor tmpc = diffuseptr[0];
      float tmpt = transpptr[0];
      col = tmpc.getPackedValue(tmpt);
    }
    this->firstcolor = col;   
  }
  
  static void
  line_segment_cb(void * userdata, SoCallbackAction * action,
                  const SoPrimitiveVertex * v1,
                  const SoPrimitiveVertex * v2)
  {
    SmEnvelopeP * thisp = (SmEnvelopeP*) userdata;

    if (thisp->firsttriangle) {
      thisp->initfirst(action);
    }
    
    // FIXME: add line-specific rendering settings
    int i;
    sm_meshattrib attrib;
    action->getMaterial(attrib.ambient,
                        attrib.diffuse,
                        attrib.specular,
                        attrib.emissive,
                        attrib.shininess,
                        attrib.transparency);
    attrib.texturename = thisp->texfilename;
    
    sm_mesh * m;
    if (!thisp->hash.get(attrib, m)) {
      m = new sm_mesh;
      m->attrib = attrib;
      thisp->meshlist.append(m);
      thisp->hash.put(attrib, m);
    }
    
    // FIXNE: support more features for lines
    SbVec3f vtx[2];
    int mi[3];
    
    const SoPrimitiveVertex * vp[] = { v1, v2 };
    for (i = 0; i < 2; i++) {
      vtx[i] = vp[i]->getPoint();
      mi[i] = vp[i]->getMaterialIndex();
    }
    
    const SbMatrix mm = action->getModelMatrix();
    const SbMatrix tm = action->getTextureMatrix();
    
    SbVec3f vx[2];
    for (i = 0; i < 2; i++) { 
      mm.multVecMatrix(vtx[i], vx[i]); 
    }
    
    for (i = 0; i < 2; i++) {
      m->lidx.append(m->bsp.addPoint(vx[i]));
      // FIXME: support color for lines
      
# if 0
      int midx = mi[i];
      uint32_t col;
      if (packedptr) {
        col = packedptr[SbClamp(midx, 0, numdiffuse)];
      }
      else {
        SbColor tmpc = diffuseptr[SbClamp(midx,0,numdiffuse)];
        float tmpt = transpptr[SbClamp(midx,0,numtransp)];
        col = tmpc.getPackedValue(tmpt);
      }
      if (col != firstcolor) m->colorpervertex = TRUE;
      SbColor tcol;
      float t;
      tcol.setPackedValue(col, t);
      m->cidx.append(m->cbsp.addPoint(SbVec3f(tcol[0], tcol[1], tcol[2])));
#endif
    }
  }
  

  static void
  triangle_cb(void * userdata, SoCallbackAction * action,
              const SoPrimitiveVertex * v1,
              const SoPrimitiveVertex * v2,
              const SoPrimitiveVertex * v3)
  {
    SmEnvelopeP * thisp = (SmEnvelopeP*) userdata;
    if (thisp->firsttriangle) {
      thisp->initfirst(action);
    }
    
    int i;
    sm_meshattrib attrib;
    action->getMaterial(attrib.ambient,
                        attrib.diffuse,
                        attrib.specular,
                        attrib.emissive,
                        attrib.shininess,
                        attrib.transparency);
    attrib.texturename = thisp->texfilename;
    
    attrib.vordering = action->getVertexOrdering();
    attrib.shapetype = action->getShapeType();
    
    if (thisp->vrmlifs) {
      attrib.vordering = thisp->vrmlifs->ccw.getValue() ? SoShapeHints::COUNTERCLOCKWISE : SoShapeHints::CLOCKWISE;
      attrib.shapetype = thisp->vrmlifs->solid.getValue() ? SoShapeHints::SOLID : SoShapeHints::UNKNOWN_SHAPE_TYPE;
    }
    sm_mesh * m;
    if (!thisp->hash.get(attrib, m)) {
      m = new sm_mesh;
      m->attrib = attrib;
      thisp->meshlist.append(m);
      thisp->hash.put(attrib, m);
    }
    
    SbVec3f vtx[3];
    SbVec3f n[3];
    SbVec4f t[3];
    int mi[3];
    
    const SoPrimitiveVertex * vp[] = { v1, v2, v3 };
    for (i = 0; i < 3; i++) {
      vtx[i] = vp[i]->getPoint();
      n[i] = vp[i]->getNormal();
      t[i] = vp[i]->getTextureCoords(); 
      mi[i] = vp[i]->getMaterialIndex();
    }
    
    const SbMatrix mm = action->getModelMatrix();
    const SbMatrix tm = action->getTextureMatrix();
  
    SbVec3f vx[3];
    SbVec3f nx[3];
    SbVec4f tx[3];
    for (i = 0; i < 3; i++) { 
      mm.multVecMatrix(vtx[i], vx[i]); 
      mm.multDirMatrix(n[i], nx[i]);
      tm.multVecMatrix(t[i], tx[i]);
      nx[i].normalize();
    }
    
    for (i = 0; i < 3; i++) {
      m->vidx.append(m->bsp.addPoint(vx[i]));
      m->nidx.append(m->nbsp.addPoint(nx[i]));
      SbVec3f tmp;
      tx[i].getReal(tmp);
      m->tidx.append(m->tbsp.addPoint(tmp));
      
      int midx = mi[i];
      uint32_t col;
      if (thisp->packedptr) {
        col = thisp->packedptr[SbClamp(midx, 0, thisp->numdiffuse)];
      }
      else {
        SbColor tmpc = thisp->diffuseptr[SbClamp(midx,0,thisp->numdiffuse)];
        float tmpt = thisp->transpptr[SbClamp(midx,0,thisp->numtransp)];
        col = tmpc.getPackedValue(tmpt);
      }
      if (col != thisp->firstcolor) m->colorpervertex = TRUE;
      SbColor tcol;
      float t;
      tcol.setPackedValue(col, t);
      m->cidx.append(m->cbsp.addPoint(SbVec3f(tcol[0], tcol[1], tcol[2])));
    }
  }
  
  void importScene(SoNode * root) {
    alltextures.append(NULL); // important!

    if (search == NULL) {
      search = new SoSearchAction;
    }
    SbViewportRegion vp(640, 480);
    SoGetBoundingBoxAction bba(vp);
    bba.apply(root);

    this->bbox.extendBy(bba.getBoundingBox());
    
    // just use the first child from each SoLOD node. We'll use RR to
    // create LODs if we need them
#if 1
    SoSearchAction sa;
    sa.setSearchingAll(TRUE);
    sa.setInterest(SoSearchAction::ALL);
    sa.setType(SoLOD::getClassTypeId());
    sa.apply(root);
    
    SoPathList & pl = sa.getPaths();
    fprintf(stderr,"found %d SoLOD nodes\n",
            pl.getLength());
    for (int i = 0; i < pl.getLength(); i++) {
      SoFullPath * p = (SoFullPath*) pl[i];
      if (p->getTail()->isOfType(SoLOD::getClassTypeId())) {
        SoLOD * l = (SoLOD*) p->getTail();
        l->range.setNum(0);
      }
    }
#endif
    
    fprintf(stderr,"About to extract all triangles...");
    SoCallbackAction ca;
    ca.addPreCallback(SoShape::getClassTypeId(),
                      pre_shape_cb, this);
    ca.addTriangleCallback(SoShape::getClassTypeId(), triangle_cb, this);
    ca.addLineSegmentCallback(SoShape::getClassTypeId(), line_segment_cb, this);
    ca.apply(root);
    fprintf(stderr,"done\n");

  }
  
  SbBool importFile(const char * filename) {    
    
    SoInput input;
    if (!input.openFile(filename)) { return FALSE; } 
    
    SoSeparator * root = SoDB::readAll(&input); 
    if (root == NULL) { return FALSE; } // err msg from SoDB::readAll()

    root->ref();
    this->importScene(root);
    root->unref();

    return TRUE;
  }

  SbBool exportGeometry(const char * outfile, const int level, const SbBool vrml2) {
    fprintf(stderr,"About to write envelope(s)\n");
        
    SbVec3f bmin = this->bbox.getMin();
    SbVec3f bd = this->bbox.getMax() - bmin;
    bd *= 1.0001f; // to make sure all points are _inside_ the box
    
    int numsplit = 1 << level;
    int numboxes = numsplit*numsplit*numsplit;
    SbBox3f *bboxes = new SbBox3f[numboxes];
    
    bd *= float(1) / float(numsplit);
    
    int cnt = 0;
    for (int z = 0; z < numsplit; z++) {
      float z0 = bmin[2] + bd[2] * float(z);
      float z1 = z0 + bd[2];
      
      for (int y = 0; y < numsplit; y++) {
        float y0 = bmin[1] + bd[1] * float(y);
        float y1 = y0 + bd[1];
        for (int x = 0; x < numsplit; x++) {
          float x0 = bmin[0] + bd[0] * float(x);
          float x1 = x0 + bd[0];
          bboxes[cnt++].setBounds(x0,y0,z0,x1,y1,z1);
        }
      }
    }

    for (int l = 0; l < numboxes; l++) {
      SbBox3f b = bboxes[l];
      fprintf(stderr,"bbox: %g %g %g, %g %g %g\n",
              b.getMin()[0],
              b.getMin()[1],
              b.getMin()[2],
              b.getMax()[0],
              b.getMax()[1],
              b.getMax()[2]);
      
      SoSeparator * triroot = new SoSeparator;
      triroot->ref();
      
      for (int j = 0; j < alltextures.getLength(); j++) {
        const char * name = alltextures[j];
        if (name) {
#if 1
          SoVRMLImageTexture * t2 = new SoVRMLImageTexture;
          t2->url = name;
          
          triroot->addChild(t2);
#else
          SoTexture2 * t = new SoTexture2;
          t->filename = name;
          t->image.setDefault(TRUE);
          t->filename.setDefault(FALSE);
          triroot->addChild(t);
#endif
        }
        for (int i = 0; i < meshlist.getLength(); i++) {
          if (meshlist[i]->attrib.texturename == name) {
            if (numboxes > 1) {
              sm_mesh * mesh = meshlist[i]->split_mesh(bboxes[l]);
              if (mesh) {
                triroot->addChild(mesh->create_iv_mesh());
                delete mesh;
              }
            }
            else {
              triroot->addChild(meshlist[i]->create_iv_mesh());
            }
          }
        }
      }

      SoOutput out;      
      SbString filename(outfile);
      if (numboxes > 1) {
        filename += "_";
        filename.addIntString(l);
        filename += ".wrl";
      }
      
      if (!out.openFile(filename.getString())) {
        fprintf(stderr,"Unable to open output file: %s\n", outfile);
        return -1;
      }
      out.setHeaderString("#VRML V1.0 ascii");
      //  out.setBinary(TRUE);

      SoWriteAction wa(&out);
      wa.apply(triroot);
      triroot->unref();
    }
    return TRUE;
  }
};

sm_meshattrib::operator unsigned long(void) const
{
  int size = sizeof(*this);
  unsigned long key = 0;
  const unsigned char * ptr = (const unsigned char *) this;
  for (int i = 0; i < size; i++) {
    int shift = (i%4) * 8;
    key ^= (ptr[i]<<shift);
  }
  return key;
}

int
sm_meshattrib::operator==(const sm_meshattrib & v) const
{
  return memcmp(this, &v, sizeof(sm_meshattrib)) == 0;
}

SoSeparator *
sm_mesh::create_iv_mesh()
{
  SoSeparator * sep = new SoSeparator;
  sep->ref();
  const sm_meshattrib & a = this->attrib;
  
  SoMaterial * mat = new SoMaterial;
  mat->diffuseColor = a.diffuse;
  mat->ambientColor = a.ambient;
  mat->emissiveColor = a.emissive;
  mat->specularColor = a.specular;
  mat->shininess = a.shininess;
  mat->transparency = a.transparency;
  
  sep->addChild(mat);

  SoCoordinate3 * c = new SoCoordinate3;
  c->point.setValues(0, this->bsp.numPoints(),
                     this->bsp.getPointsArrayPtr());
  sep->addChild(c);
  
  if (this->vidx.getLength()) {
    SoShapeHints * sh = new SoShapeHints;
    sh->creaseAngle = 0.5f;
    sh->vertexOrdering = a.vordering;
    sh->shapeType = a.shapetype;
    sh->faceType = SoShapeHints::CONVEX;
    
    sep->addChild(sh);
      
    if (this->colorpervertex) {
      SoMaterialBinding * mb = new SoMaterialBinding;
      mb->value = SoMaterialBinding::PER_VERTEX_INDEXED;
      sep->addChild(mb);
      mat->diffuseColor.setNum(this->cbsp.numPoints());
      mat->diffuseColor.setValues(0, this->cbsp.numPoints(), 
                                  (SbColor*) this->cbsp.getPointsArrayPtr());
    }
    
    
    SoNormal * n = new SoNormal;
    n->vector.setValues(0, this->nbsp.numPoints(), 
                        this->nbsp.getPointsArrayPtr());
    sep->addChild(n);
    
    if (this->attrib.texturename != NULL) {
      SoTextureCoordinate2 * tc = new SoTextureCoordinate2;
      tc->point.setNum(this->tbsp.numPoints());
      SbVec2f * dst = tc->point.startEditing();
      const int num = this->tbsp.numPoints();
      const SbVec3f * src = this->tbsp.getPointsArrayPtr(); 
      for (int i = 0; i < num; i++) {
        dst[i] = SbVec2f(src[i][0], src[i][1]);
      }
      tc->point.finishEditing();
      sep->addChild(tc);
      
#if 0 // moved to main()    
#if 0
      SoVRMLImageTexture * t2 = new SoVRMLImageTexture;
      t2->url = this->attrib.texturename;

      sep->addChild(t2);
#else

      SoTexture2 * t = new SoTexture2;
      t->filename = this->attrib.texturename;
      t->image.setDefault(TRUE);
      t->filename.setDefault(FALSE);
      fprintf(stderr,"setting texture name: %s\n", this->attrib.texturename);
      sep->addChild(t);
#endif
#endif // moved to main

    }
    
    SoIndexedFaceSet * ifs = new SoIndexedFaceSet;
    
    int numtri = this->vidx.getLength() / 3;
    
    fprintf(stderr,"mesh triangles: %d\n", numtri);
    
    ifs->coordIndex.setNum(numtri*4);
    ifs->normalIndex.setNum(numtri*4);
    if (this->colorpervertex) {
      ifs->materialIndex.setNum(numtri*4);
    }
    if (this->attrib.texturename) {
      ifs->textureCoordIndex.setNum(numtri*4);
    }
    
    int32_t * cptr = ifs->coordIndex.startEditing();
    int32_t * nptr = ifs->normalIndex.startEditing();
    int32_t * mptr = NULL;
    int32_t * tptr = NULL;
    
    if (this->colorpervertex) {
      mptr = ifs->materialIndex.startEditing();
    }
    if (this->attrib.texturename) {
      tptr = ifs->textureCoordIndex.startEditing();
    }
    
    for (int i = 0; i < numtri; i++) {
      *cptr++ = this->vidx[i*3];
      *cptr++ = this->vidx[i*3+1];
      *cptr++ = this->vidx[i*3+2];
      *cptr++ = -1;
      
      *nptr++ = this->nidx[i*3];
      *nptr++ = this->nidx[i*3+1];
      *nptr++ = this->nidx[i*3+2];
      *nptr++ = -1;
      
      if (mptr) {
        *mptr++ = this->cidx[i*3];
        *mptr++ = this->cidx[i*3+1];
        *mptr++ = this->cidx[i*3+2];
        *mptr++ = -1;
      }
      if (tptr) {
        *tptr++ = this->tidx[i*3];
        *tptr++ = this->tidx[i*3+1];
        *tptr++ = this->tidx[i*3+2];
        *tptr++ = -1; 
      }
    }
    
    ifs->coordIndex.finishEditing();
    ifs->normalIndex.finishEditing();
    if (mptr) {
      ifs->materialIndex.finishEditing();
    }
    if (tptr) {
      ifs->textureCoordIndex.finishEditing();
    }
    
    sep->addChild(ifs);
  }
  if (this->lidx.getLength()) {
    SoIndexedLineSet * ils = new SoIndexedLineSet;
    
    int numlines = this->lidx.getLength() / 2;
    
    fprintf(stderr,"mesh lines: %d\n", numlines);
    
    ils->coordIndex.setNum(numlines*3);
    int32_t * cptr = ils->coordIndex.startEditing();
       
    for (int i = 0; i < numlines; i++) {
      *cptr++ = this->lidx[i*2];
      *cptr++ = this->lidx[i*2+1];
      *cptr++ = -1;
    }
    ils->coordIndex.finishEditing();

    sep->addChild(ils);
  }

  sep->unrefNoDelete();
  return sep;
}

sm_mesh * 
sm_mesh::split_mesh(const SbBox3f & bbox)
{
  if (this->vidx.getLength() == 0) return NULL;
  
  sm_mesh * newmesh = new sm_mesh(*this);
  int numtri = this->vidx.getLength() / 3;
  
  const SbVec3f * csrc = this->bsp.getPointsArrayPtr();
  const SbVec3f * nsrc = this->nbsp.getPointsArrayPtr();
  const SbVec3f * tsrc = this->tbsp.getPointsArrayPtr();
  const SbVec3f * colsrc = this->cbsp.getPointsArrayPtr();

  if (numtri) {
    for (int i = 0; i < numtri; i++) {

      SbVec3f tst = csrc[this->vidx[i*3]];
      if (tst[0] < bbox.getMin()[0] ||
          tst[0] >= bbox.getMax()[0] ||
          tst[1] < bbox.getMin()[1] ||
          tst[1] >= bbox.getMax()[1] ||
          tst[2] < bbox.getMin()[2] ||
          tst[2] >= bbox.getMax()[2]) continue;

      newmesh->vidx.append(newmesh->bsp.addPoint(csrc[this->vidx[i*3]]));
      newmesh->vidx.append(newmesh->bsp.addPoint(csrc[this->vidx[i*3+1]]));
      newmesh->vidx.append(newmesh->bsp.addPoint(csrc[this->vidx[i*3+2]]));
      
      newmesh->nidx.append(newmesh->nbsp.addPoint(nsrc[this->nidx[i*3]]));
      newmesh->nidx.append(newmesh->nbsp.addPoint(nsrc[this->nidx[i*3+1]]));
      newmesh->nidx.append(newmesh->nbsp.addPoint(nsrc[this->nidx[i*3+2]]));
      
      if (this->colorpervertex) {
        newmesh->cidx.append(newmesh->cbsp.addPoint(colsrc[this->cidx[i*3]]));
        newmesh->cidx.append(newmesh->cbsp.addPoint(colsrc[this->cidx[i*3+1]]));
        newmesh->cidx.append(newmesh->cbsp.addPoint(colsrc[this->cidx[i*3+2]]));
      }
      if (this->attrib.texturename) {
        newmesh->tidx.append(newmesh->tbsp.addPoint(tsrc[this->tidx[i*3]]));
        newmesh->tidx.append(newmesh->tbsp.addPoint(tsrc[this->tidx[i*3+1]]));
        newmesh->tidx.append(newmesh->tbsp.addPoint(tsrc[this->tidx[i*3+2]]));
      }
    }
  }

  int numlines = this->lidx.getLength() / 2;
  if (numlines) {
    int numlines = this->lidx.getLength() / 2;
    // FIXME: support lines
  }  
  return newmesh;
}



SmEnvelope::SmEnvelope(void)
{
  this->pimpl = new SmEnvelopeP;
}

SmEnvelope::~SmEnvelope()
{
  delete this->pimpl;
}

SbBool 
SmEnvelope::importFile(const char * infile)
{
  return this->pimpl->importFile(infile);
}

void 
SmEnvelope::importScene(SoNode * node)
{
  this->pimpl->importScene(node);
}

SbBool 
SmEnvelope::exportGeometry(const char * outfile, 
                           const int octtreelevels,
                           const SbBool vrml2)
{
  return this->pimpl->exportGeometry(outfile, octtreelevels, vrml2);
}
