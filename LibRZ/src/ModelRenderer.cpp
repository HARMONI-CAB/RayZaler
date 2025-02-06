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

#include "GLModel.h"
#include "ModelRenderer.h"
#include "OMModel.h"
#include <algorithm>
#include <png.h>
#include <Logger.h>
#include <GL/glu.h>
#include "RZGLModel.h"

using namespace RZ;

ModelRenderer::ModelRenderer(
    unsigned int width,
    unsigned int height,
    RZGLModel *own)
{
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
   m_ctx = OSMesaCreateContextExt(OSMESA_RGBA, 16, 0, 0, nullptr);
#else
   m_ctx = OSMesaCreateContext(OSMESA_RGBA, nullptr);
#endif

  if (m_ctx == nullptr)
    throw std::runtime_error("Cannot create off-screen rendering context");

  m_width    = width;
  m_height   = height;
  m_ownModel = own;

  view()->setScreenGeom(width, height);
  
  RZ::IncrementalRotation screenRotation;

  screenRotation.rotateRelative(RZ::Vec3::eX(), -M_PI / 2);
  screenRotation.rotateRelative(RZ::Vec3::eZ(), -M_PI / 2);

  view()->setScreenRotation(screenRotation);

  m_pixels.resize(m_width * m_height);

  if (m_ownModel != nullptr)
    setModel(m_ownModel);
}

ModelRenderer::~ModelRenderer()
{
  if (m_ctx != nullptr)
    OSMesaDestroyContext(m_ctx);

  if (m_ownModel != nullptr)
    delete m_ownModel;
}

void
ModelRenderer::adjustViewPort()
{
  view()->configureViewPort(m_width, m_height);
}

void
ModelRenderer::zoomToContents()
{
  Vec3 p1 = Vec3::zero();
  Vec3 p2 = Vec3::zero();

  m_ownModel->omModel()->boundingBox(p1, p2);
  zoomToBox(*m_ownModel->omModel()->world(), p1, p2);
}

void
ModelRenderer::zoomToElement(Element const *element)
{
  Vec3 p1 = Vec3::zero();
  Vec3 p2 = Vec3::zero();

  element->boundingBox(p1, p2);
  zoomToBox(*m_ownModel->omModel()->world(), p1, p2);
}

void
ModelRenderer::setHighlightedBoundingBox(Element *el)
{
  m_ownModel->setHighlightedBoundingBox(el);
}

void
ModelRenderer::setShowElements(bool show)
{
  m_ownModel->setShowElements(show);
}

void
ModelRenderer::setShowApertures(bool show)
{
  m_ownModel->setShowApertures(show);
}

void
ModelRenderer::render()
{
  if (!OSMesaMakeCurrent(
    m_ctx,
    m_pixels.data(),
    GL_UNSIGNED_BYTE,
    m_width,
    m_height))
    throw std::runtime_error("Cannot create off-screen rendering context");
  
  OSMesaPixelStore(OSMESA_Y_UP, false);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  adjustViewPort();

  if (model() != nullptr) {
    if (!m_fixedLight)
      model()->configureLighting();

    view()->configureOrientation();

    if (m_fixedLight)
      model()->configureLighting();
    
    model()->display();
  }

  drawGrids();
  
  glFlush();
}

const uint32_t *
ModelRenderer::pixels() const
{
  return m_pixels.data();
}

bool
ModelRenderer::savePNG(const char *path)
{
  png_byte *png_bytes = NULL;
  png_byte **png_rows = NULL;
  png_structp png = nullptr;
  png_infop info = nullptr;
  bool ok = false;
  size_t i, nvals;
  const size_t format_nchannels = 4;

  FILE *fp = fopen(path, "wb");

  if (fp == nullptr) {
    RZError("Cannot open `%s' for writing: %s\n", path, strerror(errno));
    goto done;
  }

  nvals = m_pixels.size();

  png_bytes = reinterpret_cast<png_byte  *>(m_pixels.data());
  png_rows  = reinterpret_cast<png_byte **>(malloc(m_height * sizeof(png_byte *)));

  for (auto i = 0; i < m_height; i++)
    png_rows[i] = png_bytes + i * m_width * format_nchannels;

  png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == nullptr) {
    RZError("Failed to create PNG write struct.\n");
    goto done;
  }

  info = png_create_info_struct(png);
  if (info == nullptr) {
    RZError("Failed to create PNG info struct.\n");
    goto done;
  }
  
  if (setjmp(png_jmpbuf(png))) {
    RZError("PNG setjmp trap detected, assuming failed.\n");
    goto done;
  }

  png_init_io(png, fp);
  png_set_IHDR(
      png,
      info,
      m_width,
      m_height,
      8,
      PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
  );

  png_write_info(png, info);
  png_write_image(png, png_rows);
  png_write_end(png, nullptr);

  ok = true;

done:
  if (png != nullptr)
    png_destroy_write_struct(&png, &info);

  if (png_rows != nullptr)
    free(png_rows);
  
  if (fp != nullptr)
    fclose(fp);
  
  return ok;
}

ModelRenderer *
ModelRenderer::fromOMModel(
  OMModel *model,
  unsigned width,
  unsigned height,
  bool showElements,
  bool showApertures)
{
  RZGLModel *glModel = new RZGLModel;

  glModel->pushOptoMechanicalModel(model);
  glModel->setShowElements(showElements);
  glModel->setShowApertures(showApertures);
  

  return new ModelRenderer(width, height, glModel);
}
