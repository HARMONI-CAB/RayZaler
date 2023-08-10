#include <BlockElement.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define BLOCK_DEFAULT_LENGTH 1
#define BLOCK_DEFAULT_WIDTH  1
#define BLOCK_DEFAULT_HEIGHT 1

bool
BlockElement::propertyChanged(std::string const &name, PropertyValue const &val)
{
  Real value = val;

  if (name == "length") {
    m_sides[0]->setDistance(+.5 * value * Vec3::eX());
    m_sides[1]->setDistance(-.5 * value * Vec3::eX());
    m_sides[0]->recalculate();
    m_sides[1]->recalculate();
    m_cachedLength = value;
    return true;
  } else if (name == "width") {
    m_sides[2]->setDistance(+.5 * value * Vec3::eY());
    m_sides[3]->setDistance(-.5 * value * Vec3::eY());
    m_sides[2]->recalculate();
    m_sides[3]->recalculate();
    m_cachedWidth = value;
    return true;
  } else if (name == "height") {
    m_sides[4]->setDistance(+.5 * value * Vec3::eZ());
    m_sides[5]->setDistance(-.5 * value * Vec3::eZ());
    m_sides[4]->recalculate();
    m_sides[5]->recalculate();
    m_cachedHeight = value;
    return true;
  }
  
  return false;
}

void
BlockElement::initSides()
{
  Real rotations[][4] = {
    {+90, 0, 1, 0},
    {-90, 0, 1, 0},
    {-90, 1, 0, 0},
    {+90, 1, 0, 0},
    {  0, 0, 1, 0},
    {180, 0, 1, 0}
  };

  for (int i = 0; i < 6; ++i)
    m_rotatedSides[i] = new RotatedFrame(
      "side-rot-" + std::to_string(i),
      parentFrame(),
      Vec3(rotations[i][1], rotations[i][2], rotations[i][3]),
      rotations[i][0]);
  
  m_sides[0] = new TranslatedFrame(
      "front",
      m_rotatedSides[0],
      +5 * BLOCK_DEFAULT_LENGTH * Vec3::eX());

  m_sides[1] = new TranslatedFrame(
      "back",
      m_rotatedSides[1],
      -5 * BLOCK_DEFAULT_LENGTH * Vec3::eX());

  m_sides[2] = new TranslatedFrame(
      "right",
      m_rotatedSides[2],
      +5 * BLOCK_DEFAULT_WIDTH * Vec3::eY());

  m_sides[3] = new TranslatedFrame(
      "left",
      m_rotatedSides[3],
      -5 * BLOCK_DEFAULT_WIDTH * Vec3::eY());

  m_sides[4] = new TranslatedFrame(
      "top",
      m_rotatedSides[4],
      +5 * BLOCK_DEFAULT_HEIGHT * Vec3::eZ());

  m_sides[5] = new TranslatedFrame(
      "bottom",
      m_rotatedSides[5],
      -5 * BLOCK_DEFAULT_HEIGHT * Vec3::eZ());
}

BlockElement::BlockElement(
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(name, frame, parent)
{
  registerProperty("length",  BLOCK_DEFAULT_LENGTH);
  registerProperty("width",   BLOCK_DEFAULT_WIDTH);
  registerProperty("height",  BLOCK_DEFAULT_HEIGHT);

  m_cachedLength = BLOCK_DEFAULT_LENGTH;
  m_cachedWidth  = BLOCK_DEFAULT_WIDTH;
  m_cachedHeight = BLOCK_DEFAULT_HEIGHT;

  initSides();

  addPort("front_side", m_sides[0]);
  addPort("back_side", m_sides[1]);

  addPort("right_side", m_sides[2]);
  addPort("left_side", m_sides[3]);

  addPort("top_side", m_sides[4]);
  addPort("bottom_side", m_sides[5]);

  refreshProperties();
}


void
BlockElement::renderOpenGL()
{
  GLVectorStorage vec;

  // Draw the legs
  glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(.25, .25, .25));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(.25, .25, .25));

  glPushMatrix();
  glScalef(m_cachedLength, m_cachedWidth, m_cachedHeight);
  glutSolidCube(1);
  glPopMatrix();
}


BlockElement::~BlockElement()
{
  for (int i = 0; i < 6; ++i) {
    delete m_sides[i];
    delete m_rotatedSides[i];
  }
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
BlockElementFactory::name() const
{
  return "BlockElement";
}

Element *
BlockElementFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new BlockElement(name, pFrame, parent);
}
