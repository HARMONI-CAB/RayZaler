#ifndef _RAY_BEAM_ELEMENT_H
#define _RAY_BEAM_ELEMENT_H

#include <Element.h>
#include <RayTracingEngine.h>
#include <GLHelpers.h>
#include <pthread.h>
#include <map>

namespace RZ {
  class RayColoring {
    public:
      virtual void id2color(uint32_t, GLfloat *rgba) const;
      virtual void id2color(uint32_t, GLfloat alpha, GLfloat *rgba) const;
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

  class RayBeamElement : public Element {
      static RayColoring   m_defaultColoring;
      const RayColoring   *m_rayColoring = nullptr;
      pthread_mutex_t      m_rayMutex = PTHREAD_MUTEX_INITIALIZER;
      std::list<Ray>       m_rays;
      std::vector<GLfloat> m_vertices;
      std::vector<GLfloat> m_colors;
      std::vector<int>     m_stages;
      GLfloat              m_lineWidth = .25;
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
