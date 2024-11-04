#include <ScatterPainter.h>
#include <QPen>

ScatterPainter::ScatterPainter(
  QImage &image,
  double zoom,
  double x0,
  double y0)
{
  m_image   = &image;
  m_zoom    = zoom;
  m_x0      = x0;
  m_y0      = y0;
  m_width   = image.width();
  m_height  = image.height();
  m_dx      = 1. / (m_zoom * m_width);

  m_stride  = image.bytesPerLine() / sizeof(uint32_t);
  m_imgData = reinterpret_cast<uint32_t *>(m_image->bits());
}

int
ScatterPainter::x2px(double x) const
{
  int px = (x - m_x0) / m_dx + m_width / 2;

  return qBound(0, px, m_width - 1);
}

int
ScatterPainter::y2py(double y) const
{
  int px = (y - m_y0) / m_dx + m_height / 2;

  return qBound(0, px, m_height - 1);
}

double
ScatterPainter::px2x(int i) const
{
  return (i - m_width / 2) * m_dx + m_x0;
}

double
ScatterPainter::px2y(int j) const
{
  return (j - m_height / 2) * m_dx + m_y0;
}

ScatterVec
ScatterPainter::resolution() const
{
  return ScatterVec(m_dx);
}

ScatterVec
ScatterPainter::topLeft() const
{
  return ScatterVec(px2x(0), px2y(0));
}

ScatterVec
ScatterPainter::bottomRight() const
{
  return ScatterVec(px2x(m_width - 1), px2y(m_height - 1));
}

void
ScatterPainter::render(int x, int y, unsigned int count)
{
  y = m_height - y - 1;

  if (x >= 0 && y >= 0 && x < m_width && y < m_height) {
    uint32_t color = 0xff0000ff;
    int i;
    pset(x, y, color);

    for (i = 1; i < 3; ++i) {
      if (x + i < m_width)
        pset(x + i, y, color);
      if (y + i < m_height)
        pset(x, y + i, color);
      if (x >= i)
        pset(x - i, y, color);
      if (y >= i)
        pset(x, y - i, color);
    }
  }
}
