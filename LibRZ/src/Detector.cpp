#include <Detector.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <Singleton.h>
#include <GLHelpers.h>
#include <GL/glut.h>
#include <RayTracingEngine.h>
#include <png++/png.hpp>

using namespace RZ;

DetectorStorage::DetectorStorage(
  unsigned int cols,
  unsigned int rows,
  Real width,
  Real height)
{
  m_width = width;
  m_height = height;

  m_cols = cols;
  m_rows = rows;

  recalculate();
}

void
DetectorStorage::recalculate()
{
  size_t newSize;

  m_pxWidth  = m_width  / m_cols;
  m_pxHeight = m_height / m_rows;

  m_stride = 4 * ((m_cols + 3) / 4);
  newSize = m_rows * m_stride;

  if (m_photons.size() != newSize) {
    m_photons.resize(newSize);
    clear();
  }
}

void
DetectorStorage::setDimensions(Real width, Real height)
{
  m_width  = width;
  m_height = height;

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

void
DetectorStorage::clear()
{
  std::fill(m_photons.begin(), m_photons.end(), 0);
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

      if (!m_storage->hit(coordX, coordY))
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
  m_storage->setDimensions(m_width, m_height);
}

bool
Detector::propertyChanged(
  std::string const &name,
  PropertyValue const &value)
{
  if (name == "width") {
    m_width = value;
    recalcModel();
  } else if (name == "height") {
    m_height = value;
    recalcModel();
  } else if (name == "cols") {
    unsigned int new_cols = std::get<int64_t>(value);
    if (m_cols != new_cols) {
      m_cols = new_cols;
      m_storage->setResolution(m_cols, m_rows);
    }
  } else if (name == "rows") {
    unsigned int new_rows = std::get<int64_t>(value);
    if (m_rows != new_rows) {
      m_rows = new_rows;
      m_storage->setResolution(m_cols, m_rows);
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
  registerProperty("width",  m_width);
  registerProperty("height", m_height);
  registerProperty("cols",  m_cols);
  registerProperty("rows", m_rows);

  m_storage = new DetectorStorage(m_rows, m_cols, m_width, m_height);
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

const uint32_t *
Detector::data() const
{
  return m_storage->data();
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
Detector::renderOpenGL()
{
  GLVectorStorage vec;
  glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.0, 0.0, 0.0));
  glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(.5, .5, .25));
  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(0, 0, 0));

  glTranslatef(0, 0, 1e-3 / 2);
  glScalef(m_width, m_height, 1e-3);
  glutSolidCube(1);
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
