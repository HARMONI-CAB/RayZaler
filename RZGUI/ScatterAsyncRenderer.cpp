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

#include "ScatterAsyncRenderer.h"
#include "ScatterPainter.h"

ScatterAsyncRenderer::ScatterAsyncRenderer(
    RZ::ScatterDataProduct *product,
    QObject *parent) : AsyncDataProductRenderer(product, parent)
{
  m_asScatterDP = product;
}

ScatterAsyncRenderer::~ScatterAsyncRenderer()
{

}

void
ScatterAsyncRenderer::renderToImage(QImage &img, qreal zoom, qreal x0, qreal y0)
{
  ScatterPainter painter(img, zoom, x0, y0);
  m_asScatterDP->render(&painter);
}

bool
ScatterAsyncRenderer::isBig() const
{
  return m_asScatterDP->size() > SCATTER_ASYNC_RENDERER_THRESHOLD;
}

void
ScatterAsyncRenderer::addSet(RZ::ScatterSet *set)
{
  // Already protected by the ScatterDP mutex
  m_asScatterDP->addSet(set);
}

