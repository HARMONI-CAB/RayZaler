#include <ParabolicMirror.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ParabolicMirror::recalcModel()
{
  m_displacement = .25 * m_radius * m_radius / m_flength;

  m_cap.setRadius(m_radius);
  m_cap.setFocalLength(-m_flength);
  m_cap.setInvertNormals(true);

  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_processor->setRadius(m_radius);
  m_processor->setFocalLength(-m_flength);

  m_reflectiveSurfaceFrame->setDistance((m_thickness + m_displacement) * Vec3::eZ());
  m_reflectiveSurfacePort->setDistance((m_thickness + m_displacement) * Vec3::eZ());

  m_reflectiveSurfaceFrame->recalculate();
  m_reflectiveSurfacePort->recalculate();
}

bool
ParabolicMirror::propertyChanged(
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


ParabolicMirror::ParabolicMirror(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new ParabolicMirrorProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("flength",     5e-2);

  m_reflectiveSurfaceFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());
  m_reflectiveSurfacePort  = new TranslatedFrame("refPort", frame, Vec3::zero());

  pushOpticalSurface("refSurf", m_reflectiveSurfaceFrame, m_processor);
  addPort("refPort", m_reflectiveSurfacePort);

  m_cylinder.setVisibleCaps(true, false);
  m_cap.setInvertNormals(true);
  
  refreshProperties();
}

ParabolicMirror::~ParabolicMirror()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
ParabolicMirror::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ParabolicMirror::renderOpenGL()
{
  material("mirror");

  glTranslatef(0, 0,  m_displacement);
  m_cylinder.display();

  glRotatef(180, 1, 0, 0);
  glTranslatef(0, 0, -m_thickness);

  material("input.mirror");
  m_cap.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ParabolicMirrorFactory::name() const
{
  return "ParabolicMirror";
}

Element *
ParabolicMirrorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ParabolicMirror(this, name, pFrame, parent);
}
