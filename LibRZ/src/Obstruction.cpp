#include <Obstruction.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
Obstruction::recalcModel()
{
  m_processor->setRadius(m_radius);
  m_disc.setRadius(m_radius);
}

bool
Obstruction::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else {
    return false;
  }

  return true;
}


Obstruction::Obstruction(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new ObstructionProcessor;

  registerProperty("radius",    2.5e-2);

  m_stopSurface = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("stopSurf", m_stopSurface, m_processor);

  recalcModel();
}

Obstruction::~Obstruction()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
Obstruction::renderOpenGL()
{
  GLVectorStorage vec;
  GLfloat shiny = 0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);

  m_disc.display();
  glRotatef(180, 1, 0, 0);
  glTranslatef(0, 0, 1e-3 * m_radius);
  m_disc.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ObstructionFactory::name() const
{
  return "Obstruction";
}

Element *
ObstructionFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new Obstruction(this, name, pFrame, parent);
}
