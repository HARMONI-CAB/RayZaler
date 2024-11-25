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

#ifndef _RAY_BEAM_ELEMENT_H
#define _RAY_BEAM_ELEMENT_H

#include <Element.h>
#include <RayTracingEngine.h>
#include <GLHelpers.h>
#include <Random.h>
#include <pthread.h>
#include <map>

namespace RZ {
  class RayColoring {
    public:
      virtual void id2color(uint32_t, GLfloat *rgba) const;
      virtual void id2color(uint32_t, GLfloat alpha, GLfloat *rgba) const;
      virtual ~RayColoring();
  };

  struct CrappyCplusplusColorWrapper {
    GLfloat rgb[3];
  };

  class PaletteBasedColoring : public RayColoring {
    std::map<uint32_t, CrappyCplusplusColorWrapper> m_colors;
    GLfloat m_defaultColor[3] = {1., 1., 0};

    public:
      virtual void id2color(uint32_t, GLfloat *rgb) const override;
      
      void setColor(uint32_t id, Real r, Real g, Real b);
      void setDefaultColor(Real r, Real g, Real b);
  };

  struct LineVertexSet {
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;
    GLfloat              lineWidth = .25;
    GLushort             stipple = 0xffff;

    void renderOpenGL();
    void clear();
    void push(Vec3 const &origin, Vec3 const &dest, const GLfloat *color);
  };

  class RayBeamElement : public Element {
      static RayColoring   m_defaultColoring;
      const RayColoring   *m_rayColoring = nullptr;
      ExprRandomState      m_randState;
      unsigned int         m_maxRays = 5000;
      pthread_mutex_t      m_rayMutex = PTHREAD_MUTEX_INITIALIZER;
      std::list<Ray>       m_rays;
      LineVertexSet        m_commonRayVert;
      LineVertexSet        m_chiefRayVert;
      bool                 m_dynamicAlpha = false;
      void raysToVertices();

    public:
      RayBeamElement(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);
      virtual ~RayBeamElement();

      void clear();
      void setList(std::list<Ray> const &);
      void setRayColoring(RayColoring const *);
      void setRayColoring(RayColoring const &);
      void setRayWidth(Real width);
      void setDynamicAlpha(bool);
      virtual void renderOpenGL() override;
  };

  class RayBeamElementFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _RAY_BEAM_ELEMENT_H
