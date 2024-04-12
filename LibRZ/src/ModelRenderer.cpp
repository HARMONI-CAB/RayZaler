#include "GLModel.h"
#include "ModelRenderer.h"
#include "OMModel.h"
#include <algorithm>
#include <png.h>
#include <Logger.h>

#include <GL/glu.h>

using namespace RZ;

ModelRenderer::ModelRenderer(
    unsigned int width,
    unsigned int height)
{
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
   m_ctx = OSMesaCreateContextExt(OSMESA_RGBA, 16, 0, 0, nullptr);
#else
   m_ctx = OSMesaCreateContext(OSMESA_RGBA, nullptr);
#endif

  if (m_ctx == nullptr)
    throw std::runtime_error("Cannot create off-screen rendering context");

  m_width  = width;
  m_height = height;

  m_pixels.resize(m_width * m_height);
}

ModelRenderer::~ModelRenderer()
{
  if (m_ctx != nullptr)
    OSMesaDestroyContext(m_ctx);
}

void
ModelRenderer::adjustViewPort()
{
  GLfloat aspect;

  // Phase 1: Set viewport
  glViewport(0, 0, m_width, m_height);

  // Phase 2: Configure projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glScalef(m_zoom, m_zoom, m_zoom);
  glTranslatef(
    +2 * m_currentCenter[0] / (m_zoom * m_width),
    -2 * m_currentCenter[1] / (m_zoom * m_height),
    0);
    
  aspect = static_cast<GLfloat>(m_width) / static_cast<GLfloat>(m_height);
  glOrtho(-2, 2, -2 / aspect, 2 / aspect, -20 * m_zoom, 20 * m_zoom);

  // Phase 3: Configure normals
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_NORMALIZE);
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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  adjustViewPort();

  if (model() != nullptr) {
    if (!m_fixedLight)
      model()->configureLighting();

      glTranslatef(0, 0, -10.);
      auto k = m_incRot.k();
      auto theta = RZ::rad2deg(m_incRot.theta());

      glRotatef(theta, k.x, k.y, k.z);

      glRotatef(-90, 1, 0, 0);
      glRotatef(-90, 0, 0, 1);

    if (m_fixedLight)
      model()->configureLighting();
    
    model()->display();
  }

  glFlush();
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
    png_rows[m_height - i - 1] = png_bytes + i * m_width * format_nchannels;

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
