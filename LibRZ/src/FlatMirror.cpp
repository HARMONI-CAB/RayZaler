#include <FlatMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
FlatMirror::recalcModel()
{
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_processor->setRadius(m_radius);
  m_reflectiveSurfaceFrame->setDistance(m_thickness * Vec3::eZ());
}

bool
FlatMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else {
    return false;
  }

  return true;
}


FlatMirror::FlatMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new FlatMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);

  m_cylinder.setVisibleCaps(true, true);
  
  recalcModel();
}

FlatMirror::~FlatMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
FlatMirror::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
FlatMirror::renderOpenGL()
{
  material("");
  m_cylinder.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
FlatMirrorFactory::name() const
{
  return "FlatMirror";
}

Element *
FlatMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new FlatMirror(this, name, pFrame, parent);
}
