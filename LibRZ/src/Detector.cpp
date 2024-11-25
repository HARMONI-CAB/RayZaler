//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#include <Detector.h>
#include <TranslatedFrame.h>
#include <RotatedFrame.h>
#include <Singleton.h>
#include <GLHelpers.h>
#include <GL/glut.h>
#include <Logger.h>
#include <RayTracingEngine.h>
#include <Surfaces/Rectangular.h>
#include <png++/png.hpp>
#include <cmath>
#include <complex>

using namespace RZ;

#define RZ_DETECTOR_THICKNESS 1e-4

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

  m_maxCounts = 0;
  m_maxEnergy = 0;
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

bool
DetectorStorage::saveRawData(std::string const &path) const
{
  FILE *fp = nullptr;
  size_t chunkSize = sizeof(uint32_t) * m_cols;
  bool ok = false;

  if ((fp = fopen(path.c_str(), "wb")) == nullptr) {
    RZError("Cannot save raw data to `%s': %s\n", path.c_str(), strerror(errno));
    goto done;
  }

  for (auto j = 0; j < m_rows; ++j) {
    const uint32_t *data = m_photons.data() + j * m_stride;
    if (fwrite(data, chunkSize, 1, fp) < 1) {
      RZError("Failed to write raw data to `%s': %s\n", path.c_str(), strerror(errno));
      goto done;
    }
  }

  ok = true;
done:
  if (fp != nullptr)
    fclose(fp);

  return ok;
}

bool
DetectorStorage::saveAmplitude(std::string const &path) const
{
  FILE *fp = nullptr;
  size_t chunkSize = sizeof(RZ::Complex) * m_cols;
  bool ok = false;

  if ((fp = fopen(path.c_str(), "wb")) == nullptr) {
    RZError("Cannot save complex amplitude to `%s': %s\n", path.c_str(), strerror(errno));
    goto done;
  }

  for (auto j = 0; j < m_rows; ++j) {
    const RZ::Complex *data = m_amplitude.data() + j * m_stride;
    if (fwrite(data, chunkSize, 1, fp) < 1) {
      RZError("Failed to write complex amplitude to `%s': %s\n", path.c_str(), strerror(errno));
      goto done;
    }
  }

  ok = true;

done:
  if (fp != nullptr)
    fclose(fp);

  return ok;
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
  

  for (i = 0; i < count; ++i) {
    // Check intercept
    if (beam.hasRay(i)) {
      Vec3 coord  = plane->toRelative(Vec3(beam.destinations + 3 * i));
      if (!beam.isChief(i) && !m_storage->hit(coord.x, coord.y, beam.amplitude[i]))
        beam.prune(i);
      else
        beam.interceptDone(i);
    }
  }
}

DetectorProcessor::DetectorProcessor(DetectorStorage *storage)
{
  m_storage = storage;
  setSurfaceShape(new RectangularFlatSurface());
}

/////////////////////////////// Detector element ///////////////////////////////
void
Detector::recalcModel()
{
  m_width  = m_pxWidth  * m_cols;
  m_height = m_pxHeight * m_rows;

  m_storage->setPixelDimensions(m_pxWidth, m_pxHeight);
  m_processor->surfaceShape<RectangularFlatSurface>()->setWidth(m_width);
  m_processor->surfaceShape<RectangularFlatSurface>()->setHeight(m_height);

  setBoundingBox(
    Vec3(-m_width / 2, -m_height/2, 0),
    Vec3(+m_width / 2, +m_height/2, 2 * RZ_DETECTOR_THICKNESS));
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

bool
Detector::savePNG(std::string const &path) const
{
  return m_storage->savePNG(path);
}

bool
Detector::saveRawData(std::string const &path) const
{
  return m_storage->saveRawData(path);
}

bool
Detector::saveAmplitude(std::string const &path) const
{
  return m_storage->saveAmplitude(path);
}


uint32_t
Detector::maxCounts() const
{
  return m_storage->maxCounts();
}

Real
Detector::maxEnergy() const
{
  return m_storage->maxEnergy();
}

void
Detector::nativeMaterialOpenGL(std::string const &name)
{
  GLVectorStorage vec;
  GLfloat shiny = 128;

  glMaterialfv(GL_FRONT, GL_AMBIENT,  vec.get(0.0, 0.0, 0.0));
  if (name == "input.detector")
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(0, 0, .5));
  else
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  vec.get(.5, .5, .5));

  glMaterialfv(GL_FRONT, GL_SPECULAR, vec.get(1, 1, 1));
  glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

void
Detector::renderOpenGL()
{
  const GLfloat thickness = RZ_DETECTOR_THICKNESS;

  glTranslatef(0, 0, -thickness / 2);
  
  glPushMatrix();
  material("input.detector");
  glScalef(m_width, m_height, thickness);
  GLCube(1);
  glPopMatrix();

  glTranslatef(0, 0, -thickness);
  glPushMatrix();
  material("substrate");
  glScalef(m_width + 1e-3, m_height + 1e-3, thickness);
  GLCube(1);
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
