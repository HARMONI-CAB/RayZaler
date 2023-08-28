#ifndef _DETECTOR_H
#define _DETECTOR_H

#include <OpticalElement.h>
#include <RayProcessors.h>
#include <vector>

namespace RZ {
  class ReferenceFrame;
  class RayTransferProcessor;

  class DetectorStorage {
      std::vector<uint32_t> m_photons;
      Real m_width  = 5e-2;
      Real m_height = 5e-2;

      Real m_pxWidth;
      Real m_pxHeight;

      uint32_t     m_maxCounts = 0;
      unsigned int m_cols;
      unsigned int m_rows;
      unsigned int m_stride;

      void recalculate();

    public:
      inline bool
      hit(Real x, Real y)
      {
        int row, col;
        size_t ndx;

        col = floor((x + .5 * m_width) / m_pxWidth);
        row = floor((y + .5 * m_height) / m_pxHeight);

        if (col < 0 || col >= m_cols || row < 0 || row >= m_rows)
          return false;

        ndx = col + row * m_stride;
        ++m_photons[ndx];

        if (m_photons[ndx] > m_maxCounts)
          m_maxCounts = m_photons[ndx];
        
        return true;
      }

      DetectorStorage(unsigned int cols, unsigned int rows, Real width, Real height);

      void setDimensions(Real, Real);
      void setResolution(unsigned, unsigned);

      void clear();
      bool savePNG(std::string const &) const;

      unsigned int    cols() const;
      unsigned int    rows() const;
      unsigned int    stride() const;
      const uint32_t *data() const;
  };

  class DetectorProcessor : public PassThroughProcessor {
      DetectorStorage *m_storage; // Borrowed

    public:
      DetectorProcessor(DetectorStorage *storage);
      virtual std::string name() const;
      virtual void process(RayBeam &beam, const ReferenceFrame *) const;
  };

  class Detector : public OpticalElement {
    ReferenceFrame *m_detectorSurface = nullptr;
    DetectorStorage *m_storage = nullptr; // Owned
    const RayTransferProcessor *m_processor = nullptr;
    
    Real m_width  = 0.05;
    Real m_height = 0.05;

    unsigned int m_rows = 512;
    unsigned int m_cols = 512;

    void recalcModel();

    protected:
      virtual bool propertyChanged(std::string const &, PropertyValue const &) override;

    public:
        Detector(
          ElementFactory *,
          std::string const &,
          ReferenceFrame *,
          Element *parent = nullptr);
        ~Detector();

      virtual void renderOpenGL() override;

      void clear();
      virtual void savePNG(std::string const &) const;

      unsigned int    cols() const;
      unsigned int    rows() const;
      unsigned int    stride() const;
      const uint32_t *data() const;
  };

  class DetectorFactory : public ElementFactory {
    public:
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _DETECTOR_H
