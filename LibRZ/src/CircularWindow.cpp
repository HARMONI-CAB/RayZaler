#include <CircularWindow.h>
#include <RayProcessors/CircularWindow.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
CircularWindow::recalcModel()
{
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_inputProcessor->setRadius(m_radius);
  m_inputProcessor->setRefractiveIndex(1, m_mu);

  m_outputProcessor->setRadius(m_radius);
  m_outputProcessor->setRefractiveIndex(m_mu, 1);

  // Intercept surfaces
  m_inputFrame->setDistance(-.5 * m_thickness * Vec3::eZ());
  m_outputFrame->setDistance(+.5 * m_thickness * Vec3::eZ());
}

bool
CircularWindow::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "n") {
    m_mu = value;
    recalcModel();
  } else {
    return Element::propertyChanged(name, value);
  }

  return true;
}

CircularWindow::CircularWindow(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputProcessor  = new CircularWindowProcessor;
  m_outputProcessor = new CircularWindowProcessor;

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("n",            1.5);
  
  m_inputFrame  = new TranslatedFrame("inputSurf",  frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("outputSurf", frame, Vec3::zero());

  pushOpticalSurface("inputFace",  m_inputFrame,  m_inputProcessor);
  pushOpticalSurface("outputFace", m_outputFrame, m_outputProcessor);

  addPort("inputPort", m_inputFrame);
  addPort("outputPort", m_outputFrame);
  
  m_cylinder.setVisibleCaps(true, true);
  
  refreshProperties();
}

CircularWindow::~CircularWindow()
{
  if (m_inputProcessor != nullptr)
    delete m_inputProcessor;

  if (m_outputProcessor != nullptr)
    delete m_outputProcessor;
}

void
CircularWindow::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
CircularWindow::renderOpenGL()
{
  glTranslatef(0, 0, -.5 * m_thickness);

  material("lens");
  m_cylinder.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
CircularWindowFactory::name() const
{
  return "CircularWindow";
}

Element *
CircularWindowFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new CircularWindow(this, name, pFrame, parent);
}
