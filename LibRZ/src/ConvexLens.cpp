#include <ConvexLens.h>
#include <TranslatedFrame.h>

using namespace RZ;

void
ConvexLens::recalcModel()
{
  m_cylinder.setHeight(m_thickness);
  m_cylinder.setRadius(m_radius);

  m_depth = m_rCurv - sqrt(m_rCurv * m_rCurv - m_radius * m_radius);
  m_cap.setRadius(m_rCurv);
  m_cap.setHeight(m_depth);

  m_inputProcessor->setRadius(m_radius);
  m_inputProcessor->setCurvatureRadius(m_rCurv);
  m_inputProcessor->setRefractiveIndex(1, m_mu);

  m_outputProcessor->setRadius(m_radius);
  m_outputProcessor->setCurvatureRadius(m_rCurv);
  m_outputProcessor->setRefractiveIndex(m_mu, 1);
  
  m_inputFrame->setDistance(-.5 * m_thickness * Vec3::eZ());
  m_outputFrame->setDistance(+.5 * m_thickness * Vec3::eZ());
}

bool
ConvexLens::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "thickness") {
    m_thickness = value;
    recalcModel();
  } else if (name == "radius") {
    m_radius = value;
    recalcModel();
  } else if (name == "curvature") {
    m_rCurv = value;
    recalcModel();
  } else if (name == "n") {
    m_mu = value;
    recalcModel();
  } else {
    return false;
  }

  return true;
}

ConvexLens::ConvexLens(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  m_inputProcessor = new SphericalLensProcessor;
  m_outputProcessor = new SphericalLensProcessor;

  m_inputProcessor->setConvex(true);
  m_outputProcessor->setConvex(false);

  registerProperty("thickness",   1e-2);
  registerProperty("radius",    2.5e-2);
  registerProperty("curvature",     1.);
  registerProperty("n",            1.5);

  m_inputFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());
  m_outputFrame = new TranslatedFrame("refSurf", frame, Vec3::zero());

  pushOpticalSurface("inputFace",  m_inputFrame,  m_inputProcessor);
  pushOpticalSurface("outputFace", m_outputFrame, m_outputProcessor);

  m_cylinder.setVisibleCaps(true, true);
  
  refreshProperties();
}

ConvexLens::~ConvexLens()
{
  if (m_inputProcessor != nullptr)
    delete m_inputProcessor;

    if (m_outputProcessor != nullptr)
    delete m_outputProcessor;
}

void
ConvexLens::nativeMaterialOpenGL(std::string const &role)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.75, .75, .75));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
ConvexLens::renderOpenGL()
{
  glTranslatef(0, 0, -.5 * m_thickness);

  material("lens");
  m_cylinder.display();

  glTranslatef(0, 0, m_thickness - m_rCurv + m_depth);
  
  material("output.lens");
  m_cap.display();
  
  glRotatef(180, 1, 0, 0);
  glTranslatef(0, 0, m_thickness - 2 * (m_rCurv - m_depth));
  
  material("input.lens");
  m_cap.display();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
ConvexLensFactory::name() const
{
  return "ConvexLens";
}

Element *
ConvexLensFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new ConvexLens(this, name, pFrame, parent);
}
