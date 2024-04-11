#include <GL/glew.h>

#include "ModelRenderer.h"
#include "OMModel.h"
#include <algorithm>
#include <png.h>
#include <Logger.h>

#include <GL/glu.h>

using namespace RZ;

ModelRenderer::ModelRenderer(
    unsigned int width,
    unsigned int height,
    OMModel *model)
{
  glGenFramebuffers(1, &m_fbo);
  glGenRenderbuffers(1, &m_renderBuf);

  glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuf);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_BGRA8_EXT, width, height);
  
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
  glFramebufferRenderbuffer(
    GL_DRAW_FRAMEBUFFER,
    GL_COLOR_ATTACHMENT0,
    GL_RENDERBUFFER,
    m_renderBuf);

  m_width  = width;
  m_height = height;

  m_pixels.resize(width * height);
}

ModelRenderer::~ModelRenderer()
{
  glDeleteFramebuffers(1,&m_fbo);
  glDeleteRenderbuffers(1, &m_renderBuf);
}

void
ModelRenderer::pushElement(Element *element)
{
  m_renderList.push_back(element);
}

void
ModelRenderer::pushModel(OMModel *model)
{
  auto &orig = model->elementList();
  m_renderList.insert(m_renderList.end(), orig.begin(), orig.end());
  m_beams.push_back(model->beam());
}

void
ModelRenderer::pushElementMatrix(RZ::Element *element)
{
  auto frame = element->parentFrame();
  auto R     = frame->getOrientation();
  auto O     = frame->getCenter();

  GLdouble viewMatrix[16] = {
    R.rows[0].coords[0], R.rows[1].coords[0], R.rows[2].coords[0], O.coords[0],
    R.rows[0].coords[1], R.rows[1].coords[1], R.rows[2].coords[1], O.coords[1],
    R.rows[0].coords[2], R.rows[1].coords[2], R.rows[2].coords[2], O.coords[2],
                      0,                   0,                   0,           1};

  glPushMatrix();
  glLoadMatrixf(m_refMatrix);
  glMultTransposeMatrixd(viewMatrix);
}

#define IN_LIST(list, element) \
  (std::find(list.begin(), list.end(), element) != list.end())
  
void
ModelRenderer::popElementMatrix()
{
  glPopMatrix();
}

void
ModelRenderer::renderElements(std::list<Element *> const &list)
{
  for (auto &p : list) {
    if (IN_LIST(m_beams, p))
      continue;
    
    pushElementMatrix(p);
    p->renderOpenGL();
    popElementMatrix();

    if (p->nestedModel() != nullptr)
      renderElements(p->nestedModel()->elementList());
  }

  for (auto &beam : m_beams) {
    pushElementMatrix(beam);
    beam->renderOpenGL();
    popElementMatrix();
  }

  for (auto &p : list) {
    if (IN_LIST(m_beams, p))
      continue;
    
    pushElementMatrix(p);
    p->renderOpenGL();
    popElementMatrix();

    if (p->nestedModel() != nullptr)
      renderElements(p->nestedModel()->elementList());
  }
}

void
ModelRenderer::render()
{
  // Enter in render to framebuffer mode
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

  renderElements(m_renderList);

  // Retrieve pixels
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glReadPixels(
    0,
    0,
    m_width,
    m_height,
    GL_BGRA,
    GL_UNSIGNED_BYTE,
    m_pixels.data());
  
  // Needed to get back to on-screen rendering
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
