#ifndef _RAY_BEAM_ELEMENT_H
#define _RAY_BEAM_ELEMENT_H

#include <Element.h>
#include <RayTracingEngine.h>
#include <GLHelpers.h>

namespace RZ {
  class RayBeamElement : public Element {
      std::list<Ray> m_rays;
      std::vector<GLfloat> m_vertices;

      void raysToVertices();

    public:
      RayBeamElement(std::string const &, ReferenceFrame *, Element *parent = nullptr);
      virtual ~RayBeamElement();

      void setList(std::list<Ray> const &);

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
