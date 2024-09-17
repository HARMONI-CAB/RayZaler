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
      std::vector<Complex>  m_amplitude;
      Real m_width;
      Real m_height;

      Real m_pxWidth  = 15e-6;
      Real m_pxHeight = 15e-6;

      uint32_t     m_maxCounts = 0;
      Real         m_maxEnergy = 0;

      unsigned int m_cols;
      unsigned int m_rows;
      unsigned int m_stride;

      void recalculate();

    public:
      inline bool
      hit(Real x, Real y, Complex amplitude)
      {
        int row, col;
        size_t ndx;
        Real E;

        col = floor((x + .5 * m_width) / m_pxWidth);
        row = floor((y + .5 * m_height) / m_pxHeight);

        if (col < 0 || col >= m_cols || row < 0 || row >= m_rows)
          return false;

        ndx = col + row * m_stride;
        ++m_photons[ndx];
        m_amplitude[ndx] += amplitude;

        E = (m_amplitude[ndx] * std::conj(m_amplitude[ndx])).real();

        if (m_photons[ndx] > m_maxCounts)
          m_maxCounts = m_photons[ndx];
        
        if (E > m_maxEnergy)
          m_maxEnergy = E;
        
        return true;
      }

      inline uint32_t
      maxCounts() const
      {
        return m_maxCounts;
      }

      inline Real
      maxEnergy() const
      {
        return m_maxEnergy;
      }

      DetectorStorage(unsigned int cols, unsigned int rows, Real width, Real height);

      void setPixelDimensions(Real, Real);
      void setResolution(unsigned, unsigned);

      void clear();
      bool savePNG(std::string const &) const;
      bool saveRawData(std::string const &) const;
      bool saveAmplitude(std::string const &) const;

      unsigned int    cols() const;
      unsigned int    rows() const;
      unsigned int    stride() const;
      const uint32_t *data() const;
      const Complex  *amplitude() const;
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
    RayTransferProcessor *m_processor = nullptr;
    
    Real m_pxWidth  = 15e-6;
    Real m_pxHeight = 15e-6;

    Real m_width;
    Real m_height;

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

      virtual void nativeMaterialOpenGL(std::string const &) override;
      virtual void renderOpenGL() override;

      void clear();
      virtual bool savePNG(std::string const &) const;
      virtual bool saveRawData(std::string const &) const;
      virtual bool saveAmplitude(std::string const &) const;

      unsigned int    cols() const;
      unsigned int    rows() const;
      Real            pxWidth() const;
      Real            pxHeight() const;
      Real            width() const;
      Real            height() const;
      unsigned int    stride() const;
      const uint32_t *data() const;
      const Complex  *amplitude() const;
      
      uint32_t        maxCounts() const;
      Real            maxEnergy() const;
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
