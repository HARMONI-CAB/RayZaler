#include <CircularMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
CircularMirror::recalcModel()
{
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_processor->setRadius(m_radius);
  m_reflectiveSurfaceFrame->setDistance(m_thickness * Vec3::eZ());
}

bool
CircularMirror::propertyChanged(
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


CircularMirror::CircularMirror(
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(name, frame, parent)
{
  m_processor = new CircularMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);

  m_cylinder.setVisibleCaps(true, true);
  
  recalcModel();
}

CircularMirror::~CircularMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
CircularMirror::renderOpenGL()
{
  GLVectorStorage vec;
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.5, .5, .5));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));

  m_cylinder.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
CircularMirrorFactory::name() const
{
  return "CircularMirror";
}

Element *
CircularMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new CircularMirror(name, pFrame, parent);
}
