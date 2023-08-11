#include <SphericalMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
SphericalMirror::recalcModel()
{
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_processor->setRadius(m_radius);
  m_processor->setFocalLength(m_flength);

  m_reflectiveSurfaceFrame->setDistance(m_thickness * Vec3::eZ());
}

bool
SphericalMirror::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "flength") {
    m_flength = value;
    recalcModel();
  } else {
    return false;
  }

  return true;
}


SphericalMirror::SphericalMirror(
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(name, frame, parent)
{
  m_processor = new SphericalMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("flength",       1.);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);

  m_cylinder.setVisibleCaps(true, true);
  
  refreshProperties();
}

SphericalMirror::~SphericalMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
SphericalMirror::renderOpenGL()
{
  GLVectorStorage vec;
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.5, .5, .5));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));

  m_cylinder.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
SphericalMirrorFactory::name() const
{
  return "SphericalMirror";
}

Element *
SphericalMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new SphericalMirror(name, pFrame, parent);
}
