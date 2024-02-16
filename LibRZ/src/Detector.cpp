#include <Detector.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <Singleton.h>
#include <GLHelpers.h>
#include <GL/glut.h>
#include <RayTracingEngine.h>
#include <png++/png.hpp>
#include <cmath>
#include <complex>

using namespace RZ;

DetectorStorage::DetectorStorage(
  unsigned int cols,
  unsigned int rows,
  Real pxWidth,
  Real pxHeight)
{
  m_pxWidth  = pxWidth;
  m_pxHeight = pxHeight;

  m_cols = cols;
  m_rows = rows;

  recalculate();
}

void
DetectorStorage::recalculate()
{
  size_t newSize;

  m_width  = m_pxWidth  * m_cols;
  m_height = m_pxHeight * m_rows;

  m_stride = 4 * ((m_cols + 3) / 4);
  newSize = m_rows * m_stride;

  if (m_photons.size() != newSize) {
    m_photons.resize(newSize);
    m_amplitude.resize(newSize);
    clear();
  }
}

void
DetectorStorage::setPixelDimensions(Real width, Real height)
{
  m_pxWidth  = width;
  m_pxHeight = height;

  recalculate();
}

void
DetectorStorage::setResolution(unsigned int cols, unsigned int rows)
{
  m_cols = cols;
  m_rows = rows;

  recalculate();
}

unsigned int
DetectorStorage::cols() const
{
  return m_cols;
}

unsigned int
DetectorStorage::rows() const
{
  return m_rows;
}

unsigned int
DetectorStorage::stride() const
{
  return m_stride;
}

const uint32_t *
DetectorStorage::data() const
{
  return m_photons.data();
}

const Complex *
DetectorStorage::amplitude() const
{
  return m_amplitude.data();
}

void
DetectorStorage::clear()
{
  std::fill(m_photons.begin(), m_photons.end(), 0);
  std::fill(m_amplitude.begin(), m_amplitude.end(), 0.);
}

bool
DetectorStorage::savePNG(std::string const &path) const
{
  png::image<png::rgb_pixel> image(m_cols, m_rows);

  for (size_t j = 0; j < m_rows; ++j) {
    for (size_t i = 0; i < m_cols; ++i) {
      auto ndx = i + j * m_stride;
      uint8_t value = m_maxCounts > 0 ? (m_photons[ndx] * 255) / m_maxCounts : 0;

      image[j][i] = png::rgb_pixel(value, value, value);
    }
  }

  image.write(path);

  return true; // Cross your fingers
}

/////////////////////////////// Detector storage ///////////////////////////////
std::string
DetectorProcessor::name() const
{
  return "DetectorProcessor";
}

#define WAVENUMBER (2 * M_PI * 4e9 / 3e8)

void
DetectorProcessor::process(RayBeam &beam, const ReferenceFrame *plane) const
{
  uint64_t count = beam.count;
  uint64_t i;
  Vec3 center = plane->getCenter();
  Vec3 tX     = plane->eX();
  Vec3 tY     = plane->eY();

  for (i = 0; i < count; ++i) {
    // Check intercept
    if (beam.hasRay(i)) {
      Vec3 coord  = Vec3(beam.destinations + 3 * i) - plane->getCenter();
      Real coordX = coord * tX;
      Real coordY = coord * tY;
      auto phasor = std::exp(Complex(0, 1) * WAVENUMBER * beam.cumOptLengths[i]);

      if (!m_storage->hit(coordX, coordY, phasor))
        beam.prune(i); 
    }
  }

  PassThroughProcessor::process(beam, plane);
}

DetectorProcessor::DetectorProcessor(DetectorStorage *storage)
{
  m_storage = storage;
}

/////////////////////////////// Detector element ///////////////////////////////
void
Detector::recalcModel()
{
  m_width  = m_pxWidth  * m_cols;
  m_height = m_pxHeight * m_rows;

  m_storage->setPixelDimensions(m_pxWidth, m_pxHeight);
}

bool
Detector::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "pixelWidth") {
    m_pxWidth = value;
    recalcModel();
  } else if (name == "pixelHeight") {
    m_pxHeight = value;
    recalcModel();
  } else if (name == "cols") {
    unsigned int new_cols = (unsigned int) value;
    if (m_cols != new_cols) {
      m_cols = new_cols;
      m_storage->setResolution(m_cols, m_rows);
      recalcModel();
    }
  } else if (name == "rows") {
    unsigned int new_rows = (unsigned int) value;
    if (m_rows != new_rows) {
      m_rows = new_rows;
      m_storage->setResolution(m_cols, m_rows);
      recalcModel();
    }
  } else {
    return false;
  }

  return true;
}


Detector::Detector(
  ElementFactory *factory,
  std::string const &name,
  ReferenceFrame *frame,
  Element *parent) : OpticalElement(factory, name, frame, parent)
{
  registerProperty("pixelWidth",  m_pxWidth);
  registerProperty("pixelHeight", m_pxHeight);
  registerProperty("cols",  m_cols);
  registerProperty("rows",  m_rows);

  m_storage = new DetectorStorage(m_rows, m_cols, m_pxWidth, m_pxHeight);
  m_processor = new DetectorProcessor(m_storage);
  m_detectorSurface = new TranslatedFrame("detSurf", frame, Vec3::zero());

  pushOpticalSurface("detSurf", m_detectorSurface, m_processor);

  refreshProperties();
  recalcModel();
}

Detector::~Detector()
{
  if (m_storage != nullptr)
    delete m_storage;

  if (m_processor != nullptr)
    delete m_processor;

}

unsigned int
Detector::cols() const
{
  return m_storage->cols();
}

unsigned int
Detector::rows() const
{
  return m_storage->rows();
}

unsigned int
Detector::stride() const
{
  return m_storage->stride();
}

Real
Detector::pxWidth() const
{
  return m_pxWidth;
}

Real
Detector::pxHeight() const
{
  return m_pxHeight;
}
Real
Detector::width() const
{
  return m_width;
}

Real
Detector::height() const
{
  return m_height;
}

const uint32_t *
Detector::data() const
{
  return m_storage->data();
}

const Complex *
Detector::amplitude() const
{
  return m_storage->amplitude();
}

void
Detector::clear()
{
  m_storage->clear();
}

void
Detector::savePNG(std::string const &path) const
{
  m_storage->savePNG(path);
}

void
Detector::nativeMaterialOpenGL(std::string const &name)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.0, 0.0, 0.0));
  if (name == "detector")
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(0, 0, .5));
  else
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(.5, .5, .5));

  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
Detector::renderOpenGL()
{
  const GLfloat thickness = 1e-4;

  glTranslatef(0, 0, -thickness / 2);
  
  glPushMatrix();
  material("detector");
  glScalef(m_width, m_height, thickness);
  glutSolidCube(1);
  glPopMatrix();

  glTranslatef(0, 0, -thickness);
  glPushMatrix();
  material("substrate");
  glScalef(m_width + 1e-3, m_height + 1e-3, thickness);
  glutSolidCube(1);
  glPopMatrix();
}

///////////////////////////////// Factory //////////////////////////////////////
std::string
DetectorFactory::name() const
{
  return "Detector";
}

Element *
DetectorFactory::make(
  std::string const &name,
  ReferenceFrame *pFrame,
  Element *parent)
{
  return new Detector(this, name, pFrame, parent);
}
