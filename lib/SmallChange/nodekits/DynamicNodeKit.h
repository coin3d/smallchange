#ifndef DynamicNodeKit_H
#define DynamicNodeKit_H

#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/nodekits/SoSubKit.h>

#include <Inventor/fields/SoFieldData.h>
#include <Inventor/nodekits/SoNodekitCatalog.h>

#include <Inventor/C/XML/parser.h>
#include <Inventor/C/XML/document.h>
#include <Inventor/C/XML/element.h>
#include <Inventor/C/XML/attribute.h>


class SoNodekitCatalog;
class SoFieldData;
struct cc_xml_elt;

template <class Base>
class DynamicNodeKit : public Base {
  typedef Base inherited;

  /*
    Misc stuff from the coin init macros:
  */
public:
  static SoType getClassTypeId(void);
  virtual SoType getTypeId(void) const;
  static const SoNodekitCatalog * getClassNodekitCatalog(void);
  virtual const SoNodekitCatalog * getNodekitCatalog(void) const;
protected:
  static const SoFieldData ** getFieldDataPtr(void);
  virtual const SoFieldData * getFieldData(void) const;
  static const SoNodekitCatalog ** getClassNodekitCatalogPtr(void);
private:
  static SoType classTypeId;
  static void atexit_cleanup(void);
  static const SoFieldData ** parentFieldData;
  static SoFieldData * fieldData;
  static unsigned int classinstances;
  static void * createInstance(void);
  static SoNodekitCatalog * classcatalog;
  static const SoNodekitCatalog ** parentcatalogptr;
  static void atexit_cleanupkit(void);

  /*
    Functions that we add:
  */
public:
  DynamicNodeKit(void);
  static void initClass(void);
  virtual void copyContents(const SoFieldContainer * from, SbBool copyconn);

  bool startEditing();
  bool addField(SbString name, SbString typeString, SbString defaultValue);
  bool addPart(SbString name, SbString typeString, SbBool isDefaultNull, SbString parentName, SbString rightSiblingName, SbBool isPublic);
  bool setNodekitDescription(SbString xmlDescription);
  bool finishEditing();

  //We need public functions for [get|set]AnyPart, so that the script wrapper can call them
  virtual SoNode * getAnyPart(const SbName & partname, SbBool makeifneeded, SbBool leafcheck = 0, SbBool publiccheck = 0);
  virtual SbBool setAnyPart(const SbName & partname, SoNode * from, SbBool anypart = 1);

protected:
  virtual ~DynamicNodeKit();

private:
  SoNodekitCatalog * dynamicCatalog;
  SoFieldData * dynamicFieldData;
  bool addPartsFromXml(cc_xml_elt * element, const char * parentName, const char * rightSiblingName);

  int staticPartCount;
  bool finished;
  bool isEditing;
};

/*
End class definition, start template method implementation
*/

//only compile when instanciating with new types
#ifndef DYNAMIC_NODEKIT_NO_GENERATE_FUNCTIONS
template <class Base> SoType DynamicNodeKit<Base>::classTypeId STATIC_SOTYPE_INIT;
template <class Base> const SoFieldData ** DynamicNodeKit<Base>::parentFieldData = NULL;
template <class Base> SoFieldData * DynamicNodeKit<Base>::fieldData = NULL;
template <class Base> unsigned int DynamicNodeKit<Base>::classinstances = 0;
template <class Base> SoNodekitCatalog * DynamicNodeKit<Base>::classcatalog = NULL;
template <class Base> const SoNodekitCatalog ** DynamicNodeKit<Base>::parentcatalogptr = NULL;

template <class Base>
inline
const SoNodekitCatalog *
DynamicNodeKit<Base>::getClassNodekitCatalog(void)
{
  return DynamicNodeKit<Base>::classcatalog;
}

template <class Base>
inline
const SoNodekitCatalog **
DynamicNodeKit<Base>::getClassNodekitCatalogPtr(void)
{
  return const_cast<const class SoNodekitCatalog **>(&DynamicNodeKit<Base>::classcatalog);
}

template <class Base>
inline
void
DynamicNodeKit<Base>::atexit_cleanupkit(void)
{
  delete DynamicNodeKit<Base>::classcatalog;
  DynamicNodeKit<Base>::classcatalog = NULL;
  DynamicNodeKit<Base>::parentcatalogptr = NULL;
}

template <class Base>
inline
SoType
DynamicNodeKit<Base>::getClassTypeId(void) {
  return DynamicNodeKit<Base>::classTypeId;
}

template <class Base>
inline
SoType
DynamicNodeKit<Base>::getTypeId(void) const {
  return DynamicNodeKit<Base>::classTypeId;
}

template <class Base>
inline
const SoFieldData **
DynamicNodeKit<Base>::getFieldDataPtr(void)
{
  return const_cast<const SoFieldData **>(&DynamicNodeKit<Base>::fieldData);
}

template <class Base>
inline
void
DynamicNodeKit<Base>::atexit_cleanup(void)
{
  delete DynamicNodeKit<Base>::fieldData;
  DynamicNodeKit<Base>::fieldData = NULL;
  DynamicNodeKit<Base>::parentFieldData = NULL;
  SoType::removeType(DynamicNodeKit<Base>::classTypeId.getName());
  DynamicNodeKit<Base>::classTypeId STATIC_SOTYPE_INIT;
  DynamicNodeKit<Base>::classinstances = 0;
}

template <class Base>
inline
void *
DynamicNodeKit<Base>::createInstance(void)
{
  return new DynamicNodeKit<Base>;
}

//End default implementations

//overloaded:
template <class Base>
inline
const SoFieldData *
DynamicNodeKit<Base>::getFieldData(void) const
{
  return dynamicFieldData;
}

// overloaded:
template <class Base>
inline
const SoNodekitCatalog *
DynamicNodeKit<Base>::getNodekitCatalog(void) const
{
  return this->dynamicCatalog;
}

template <class Base>
inline
void
DynamicNodeKit<Base>::initClass(void)
{
  if (DynamicNodeKit<Base>::getClassTypeId() == SoType::badType()){
    SoType parentType = Base::getClassTypeId();
    SbName parentName = parentType.getName();

    //FIXME: find a good naming scheme, will be used as script constructor name
    SbString classNameString = "Dynamic";
    classNameString += parentName;

    const char * classname = classNameString.getString();

    /* Set up entry in the type system. */
    DynamicNodeKit<Base>::classTypeId =
      SoType::createType(parentType,
                         classname,
                         &DynamicNodeKit<Base>::createInstance,
                         SoNode::getNextActionMethodIndex());
    SoNode::incNextActionMethodIndex();

    /* Store parent's fielddata pointer for later use in the constructor. */
    DynamicNodeKit<Base>::parentFieldData = Base::getFieldDataPtr();
    /* Make sure also external nodes are cleaned up */
    cc_coin_atexit_static_internal((coin_atexit_f*)DynamicNodeKit<Base>::atexit_cleanup);

    DynamicNodeKit<Base>::parentcatalogptr = Base::getClassNodekitCatalogPtr();
  }
  /*
  static bool never = false;
  if (never){
    never = true;
    SbString dummy("");
    DynamicNodeKit<Base>::initClass();
    DynamicNodeKit<Base> * kit = new DynamicNodeKit<Base>();
    //virtual void copyContents(const SoFieldContainer * from, SbBool copyconn);
    kit->startEditing();
    kit->addField(dummy, dummy, dummy);
    kit->addPart(dummy, dummy, false, dummy, dummy, false);
    kit->setNodekitDescription(dummy);
    kit->finishEditing();
    kit->getAnyPart(dummy, false);
    kit->setAnyPart(dummy, NULL);
  }
  */
}

template <class Base>
inline
DynamicNodeKit<Base>::DynamicNodeKit(void) : inherited()
{
  SO_KIT_CONSTRUCTOR(DynamicNodeKit);//FIXME: expand

  this->dynamicFieldData = new SoFieldData;
  this->dynamicFieldData->copy(inherited::getFieldData());//clone from parent?

  this->dynamicCatalog = inherited::getNodekitCatalog()->clone(DynamicNodeKit<Base>::getClassTypeId());
  this->staticPartCount = dynamicCatalog->getNumEntries();

  this->finished = false;
  this->isEditing = false;
}

template <class Base>
inline
DynamicNodeKit<Base>::~DynamicNodeKit()
{
  //remove dynamic fields and parts and delete the fieldData and catalog
  const int n = this->dynamicFieldData->getNumFields();
  for (int i = 0; i < n; i++) {
    SoField * f = this->dynamicFieldData->getField(this, i);
    if ((*this->getFieldDataPtr())->getIndex(this, f) == -1) { //don't delete fields from parent
      delete f;
    }
  }
  delete this->dynamicFieldData;
  delete this->dynamicCatalog;
}

template <class Base>
inline
void
DynamicNodeKit<Base>::copyContents(const SoFieldContainer * from, SbBool copyconn)
{
  //FIXME: verify that this makes sense
  assert(from->isOfType(DynamicNodeKit<Base>::getClassTypeId()) && "wrong type in copyContents");

  const SoFieldData * src = from->getFieldData();
  const int n = src->getNumFields();
  for (int i = 0; i < n; i++) {
    const SoField * f = src->getField(from, i);
    if (this->dynamicFieldData->getIndex(this, f) == -1) { //only add new fields
      SoField * cp = (SoField*) f->getTypeId().createInstance();
      cp->setFieldType(f->getFieldType());
      cp->setContainer(this);
      this->dynamicFieldData->addField(this, src->getFieldName(i), cp);
    }
  }
  //FIXME: copy the catalog as well? seems like this is handled by SoBase::copyContents...
  inherited::copyContents(from, copyconn);
}

template <class Base>
inline
bool
DynamicNodeKit<Base>::addField(SbString name, SbString typeString, SbString defaultValue)
{
  if (!this->isEditing){
    return false;
  }
  //FIXME: check typestring
  SoType type = SoType::fromName(typeString);
  SoField * f = static_cast<SoField *>(type.createInstance());
  f->setContainer(this);
  if (defaultValue != "") f->set(defaultValue.getString());
  this->dynamicFieldData->addField(this, name.getString(), f);
  return true;
}

template <class Base>
inline
bool
DynamicNodeKit<Base>::addPart(SbString name, SbString typeString, SbBool isDefaultNull, SbString parentName, SbString rightSiblingName, SbBool isPublic)
{
  if (!this->isEditing){
    return false;
  }
  //FIXME: check typestring
  SoType type = SoType::fromName(typeString);
  this->dynamicCatalog->addEntry(name, type, type, isDefaultNull, parentName, rightSiblingName, FALSE, SoType::badType(), SoType::badType(), isPublic);

  SoSFNode * f = new SoSFNode;//FIXME: support list parts, abstract parts
  f->setContainer(this);
  f->setValue(NULL);
  this->dynamicFieldData->addField(this, name.getString(), f);
  return true;
}

template <class Base>
inline
bool
DynamicNodeKit<Base>::startEditing()
{
  if (this->finished){
    //FIXME: support more than one edit
    return false;
  }
  this->isEditing = true;
  return true;
}

template <class Base>
inline
bool
DynamicNodeKit<Base>::finishEditing()
{
  if (!this->isEditing || this->finished){
    return false;
  }

  this->createFieldList();//adds all entries from catalog to SoBaseKit::pimpl->instancelist
  this->createDefaultParts();//instanciates all the parts that are not null by default

  this->isEditing = false;
  this->finished = true;
  return true;
}

template <class Base>
inline
bool
DynamicNodeKit<Base>::setNodekitDescription(SbString xmlDescription)
{
  if (this->finished || this->isEditing){
    return false;
  }
  //FIXME: check xml against a dtd or similar
  cc_xml_document * xmldoc = cc_xml_read_buffer(xmlDescription.getString());

  cc_xml_element * root = cc_xml_doc_get_root(xmldoc);
  if (strcmp(cc_xml_elt_get_type(root), "DynamicNodeKitDescription")){
    return false;
  }

  this->startEditing();

  int numFields = cc_xml_elt_get_num_children_of_type(root, "Field");
  for (int i = 0; i < numFields; i++){
    cc_xml_element * fieldElement = cc_xml_elt_get_child_of_type(root, "Field", i);
    const char * name = cc_xml_attr_get_value(cc_xml_elt_get_attribute(fieldElement, "name"));
    const char * type = cc_xml_attr_get_value(cc_xml_elt_get_attribute(fieldElement, "type"));
    const char * value = cc_xml_elt_get_cdata(fieldElement);
    this->addField(name, type, value);
  }

  this->addPartsFromXml(root, "this", "");
  this->finishEditing();

  cc_xml_doc_delete_x(xmldoc);//should free up everything cc_xml has allocated
  return true;
}

template <class Base>
inline
bool
DynamicNodeKit<Base>::addPartsFromXml(cc_xml_element * element, const char * parentName, const char * rightSiblingName)
{
  const char * name = parentName;
  if (!strcmp(cc_xml_elt_get_type(element), "Part")){//true except for first call
    name = cc_xml_attr_get_value(cc_xml_elt_get_attribute(element, "name"));
    const char * type = cc_xml_attr_get_value(cc_xml_elt_get_attribute(element, "type"));
    cc_xml_attribute * isDefaultNullAttr = cc_xml_elt_get_attribute(element, "isNullByDefault");
    const char * isDefaultNull = isDefaultNullAttr ? cc_xml_attr_get_value(isDefaultNullAttr) : "false";
    cc_xml_attribute * isPublicAttr = cc_xml_elt_get_attribute(element, "isPublic");
    const char * isPublic = isPublicAttr ? cc_xml_attr_get_value(isPublicAttr) : "false";
    this->addPart(name, type, strcmp(isDefaultNull, "false"), parentName, rightSiblingName, strcmp(isPublic, "false"));
  }

  int numChildren = cc_xml_elt_get_num_children_of_type(element, "Part");
  for (int i = 0; i < numChildren; i++){
    cc_xml_element * partElement = cc_xml_elt_get_child_of_type(element, "Part", i);
    const char * nextSiblingName = "";
    if (i < numChildren - 1){
      cc_xml_element * siblingElement = cc_xml_elt_get_child_of_type(element, "Part", i + 1);
      nextSiblingName = cc_xml_attr_get_value(cc_xml_elt_get_attribute(siblingElement, "name"));
    }
    this->addPartsFromXml(partElement, name, nextSiblingName);//recurse
  }
  return true;
}


template <class Base>
inline
SoNode *
DynamicNodeKit<Base>::getAnyPart(const SbName & partname, SbBool makeifneeded, SbBool leafcheck, SbBool publiccheck)
{
  return inherited::getAnyPart(partname, makeifneeded, leafcheck, publiccheck);
}

template <class Base>
inline
SbBool
DynamicNodeKit<Base>::setAnyPart(const SbName & partname, SoNode * from, SbBool anypart)
{
  return inherited::setAnyPart(partname, from, anypart);
}

#endif //SMALLCHANGE_INTERNAL

#endif // DynamicNodeKit_H
