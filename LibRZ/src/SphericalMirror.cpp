#include <SphericalMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
SphericalMirror::recalcModel()
{
  GLfloat rCurv = 2 * m_flength;
  GLfloat absR  = fabs(rCurv);
  
  // Displacement of the curvature center wrt the intercept surface
  m_displacement = sqrt(rCurv * rCurv - m_radius * m_radius);

  // If the curvature radius is negative, the center is on the other side
  if (rCurv < 0)
    m_displacement = -m_displacement;

  // Depth (or height) of the spherical surface w.r.t the edge. This is the
  // z distance of the center w.r.t the edge.
  m_depth = rCurv - m_displacement;

  m_cap.setRadius(rCurv);
  m_cap.setHeight(fabs(m_depth));
  
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_processor->setRadius(m_radius);
  m_processor->setFocalLength(m_flength);
  
  m_reflectiveSurfaceFrame->setDistance(m_depth * Vec3::eZ());
  m_reflectiveSurfaceFrame->recalculate();
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
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new SphericalMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("flength",       1.);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);
  addPort("refPort", m_reflectiveSurfaceFrame);
  
  m_cylinder.setVisibleCaps(true, false);
  m_cap.setInvertNormals(true);
  
  refreshProperties();
}

SphericalMirror::~SphericalMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
SphericalMirror::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
SphericalMirror::renderOpenGL()
{
  material("mirror");

  glTranslatef(0, 0, m_depth - m_thickness);
  m_cylinder.display();

  glRotatef(180, 1, 0, 0);

  glTranslatef(0, 0, -m_thickness - m_displacement);

  material("input.mirror");
  m_cap.display();
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
  return new SphericalMirror(this, name, pFrame, parent);
}
