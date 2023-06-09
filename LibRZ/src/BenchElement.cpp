#include <BenchElement.h>
#include <TranslatedFrame.h>
#include <GL/glut.h>

using namespace RZ;

#define BENCH_DEFAULT_WIDTH        2
#define BENCH_DEFAULT_DEPTH        1.5
#define BENCH_DEFAULT_TABLE_HEIGHT 0.03
#define BENCH_DEFAULT_LEG_RADIUS   0.15
#define BENCH_DEFAULT_LEG_SEP      (2 * BENCH_DEFAULT_LEG_RADIUS)

bool
BenchElement::propertyChanged(std::string const &name, PropertyValue const &val)
{
  if (name == "height") {
    Real height = val;
    m_cachedHeight = height;
    
    m_cylinder.setRadius(BENCH_DEFAULT_LEG_RADIUS);
    m_cylinder.setHeight(height - BENCH_DEFAULT_TABLE_HEIGHT);

    m_surfaceFrame->setDistance(height * Vec3::eZ());
    m_surfaceFrame->recalculate();
    return true;
  }
  
  return false;
}

BenchElement::BenchElement(
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent)
  : Element(name, frame, parent)
{
  registerProperty("height", 0.);

  m_cylinder.setVisibleCaps(true, false);

  m_surfaceFrame = registerPort(
    "surface",
    new TranslatedFrame("surface", frame, Vec3::zero()));

  refreshProperties();
}


void
BenchElement::renderOpenGL()
{
  GLVectorStorage vec;
  GLfloat legLoc[4][2] = {
    {
      -BENCH_DEFAULT_WIDTH / 2 + BENCH_DEFAULT_LEG_SEP, 
      -BENCH_DEFAULT_DEPTH / 2 + BENCH_DEFAULT_LEG_SEP
    },
    {
      +BENCH_DEFAULT_WIDTH / 2 - BENCH_DEFAULT_LEG_SEP, 
      -BENCH_DEFAULT_DEPTH / 2 + BENCH_DEFAULT_LEG_SEP
    },
    {
      -BENCH_DEFAULT_WIDTH / 2 + BENCH_DEFAULT_LEG_SEP, 
      +BENCH_DEFAULT_DEPTH / 2 - BENCH_DEFAULT_LEG_SEP
    },
    {
      +BENCH_DEFAULT_WIDTH / 2 - BENCH_DEFAULT_LEG_SEP, 
      +BENCH_DEFAULT_DEPTH / 2 - BENCH_DEFAULT_LEG_SEP
    }
  };
  // Draw the table itself

  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.25, .25, .25));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(.1, .1, .1));

  glPushMatrix();
  glTranslatef(0, 0, m_cachedHeight - BENCH_DEFAULT_TABLE_HEIGHT / 2);
  glScalef(BENCH_DEFAULT_WIDTH, BENCH_DEFAULT_DEPTH, BENCH_DEFAULT_TABLE_HEIGHT);
  glutSolidCube(1);
  glPopMatrix();

  // Draw the legs
  glMaterialfv(GL_FRONT, GL_AMBIENT, vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE, vec.get(.1, .1, .1));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(.1, .1, .1));

  for (auto i = 0; i < 4; ++i) {
    glPushMatrix();
    glTranslatef(legLoc[i][0], legLoc[i][1], 0);

    m_cylinder.display();

    glPopMatrix();
  }
}


BenchElement::~BenchElement()
{
}
///////////////////////////////// Factory //////////////////////////////////////
std::string
BenchElementFactory::name() const
{
  return "BenchElement";
}

Element *
BenchElementFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new BenchElement(name, pFrame, parent);
}
