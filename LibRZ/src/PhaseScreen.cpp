#include <PhaseScreen.h>

#include <PhaseScreen.h>
#include <TranslatedFrame.h>
#include <Helpers.h>

using namespace RZ;

void
PhaseScreen::recalcModel()
{
  if (m_height < 2 * m_radius)
    m_height = 2 * m_radius;
  
  if (m_width < 2 * m_radius)
    m_width = 2 * m_radius;

  m_processor->setRadius(m_radius);
  m_processor->setRefractiveIndex(m_muIn, m_muOut);

  m_pinHole.setRadius(m_radius);
  m_pinHole.setHeight(m_height);
  m_pinHole.setWidth(m_width);
}

bool
PhaseScreen::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  unsigned int zCoef;

  if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "ni") {
    m_muIn = value;
    recalcModel();
  } else if (name == "no") {
    m_muOut = value;
    recalcModel();
  } else if (name == "width") {
    m_width = value;
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    recalcModel();
  } else if (sscanf(name.c_str(), "Z%u", &zCoef) == 1) {
    m_processor->setCoef(zCoef, value);
  } else {
    return false;
  }

  return true;
}


PhaseScreen::PhaseScreen(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_processor = new PhaseScreenProcessor;

  unsigned int i;

  for (i = 0; i < 60; ++i)
    registerProperty(string_printf("Z%u", i), 0);
  
  registerProperty("radius",    2.5e-2);
  registerProperty("width",     7.5e-2);
  registerProperty("height",    7.5e-2);
  registerProperty("ni",        1);
  registerProperty("no",        1.5);
  m_stopSurface = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("stopSurf", m_stopSurface, m_processor);

  recalcModel();
}

PhaseScreen::~PhaseScreen()
{
  if (m_processor != nullptr)
    delete m_processor;
}

void
PhaseScreen::nativeMaterialOpenGL(std::string const &)
{
  GLVectorStorage vec;
  GLfloat shiny = 0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
PhaseScreen::renderOpenGL()
{
  material("input.surface");
  m_pinHole.display();
  
  glRotatef(180, 1, 0, 0);
  glTranslatef(0, 0, 1e-3 * m_radius);
  
  material("output.surface");
  m_pinHole.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
PhaseScreenFactory::name() const
{
  return "PhaseScreen";
}

Element *
PhaseScreenFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new PhaseScreen(this, name, pFrame, parent);
}
