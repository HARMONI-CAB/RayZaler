#include <RodElement.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define ROD_DEFAULT_LENGTH   5e-2
#define ROD_DEFAULT_DIAMETER 3e-3

bool
RodElement::propertyChanged(std::string const &name, PropertyValue const &val)
{
  Real value = val;
  bool middleChanged = false;

  if (name == "length") {
    m_sides[0]->setDistance(value * Vec3::eZ());
    m_sides[0]->recalculate();
    m_cachedLength = value;
    middleChanged = true;
  } else if (name == "diameter") {
    m_cachedDiameter = value;
    middleChanged = true;
  }
  
  if (middleChanged) {
    m_sides[1]->setDistance(
        0.5 * m_cachedDiameter * Vec3::eX()
      + 0.5 * m_cachedLength   * Vec3::eZ());
    m_sides[1]->recalculate();
    m_cylinder.setHeight(m_cachedLength);
    m_cylinder.setRadius(.5 * m_cachedDiameter);
    return true;
  }

  return Element::propertyChanged(name, val);
}

void
RodElement::initSides()
{
  const char *names[] = {
    "top", "middle", "bottom"
  };

  Real rotations[][4] = {
    {  0, 0, 1, 0}, // Top
    {+90, 0, 1, 0}, // Middle
    {180, 0, 1, 0}  // Bottom
  };

  for (int i = 0; i < 3; ++i)
    m_rotatedSides[i] = new RotatedFrame(
      std::string(names[i]) + "_rotation",
      parentFrame(),
      Vec3(rotations[i][1], rotations[i][2], rotations[i][3]),
      rotations[i][0]);
  
  m_sides[0] = new TranslatedFrame(
      "top",
      m_rotatedSides[0],
      m_cachedLength * Vec3::eZ());

  m_sides[1] = new TranslatedFrame(
      "middle",
      m_rotatedSides[1],
        0.5 * m_cachedDiameter * Vec3::eX()
      + 0.5 * m_cachedLength   * Vec3::eZ());

  m_sides[2] = new TranslatedFrame(
      "bottom",
      m_rotatedSides[2], 0 * Vec3::eZ());

  m_cylinder.setHeight(m_cachedLength);
  m_cylinder.setRadius(.5 * m_cachedDiameter);
  m_cylinder.setVisibleCaps(true, true);
  m_cylinder.setSlices(24);
}

RodElement::RodElement(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(factory, name, frame, parent)
{
  registerProperty("length",   ROD_DEFAULT_LENGTH);
  registerProperty("diameter", ROD_DEFAULT_DIAMETER);

  m_cachedLength   = ROD_DEFAULT_LENGTH;
  m_cachedDiameter = ROD_DEFAULT_DIAMETER;

  initSides();

  addPort("top",    m_sides[0]);
  addPort("middle", m_sides[1]);
  addPort("bottom", m_sides[2]);

  refreshProperties();
}

void
RodElement::renderOpenGL()
{
  material("rod");

  m_cylinder.display();
}


RodElement::~RodElement()
{
  for (int i = 0; i < 3; ++i) {
    delete m_sides[i];
    delete m_rotatedSides[i];
  }
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
RodElementFactory::name() const
{
  return "RodElement";
}

Element *
RodElementFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new RodElement(this, name, pFrame, parent);
}
