#ifndef _ELEMENT_H
#define _ELEMENT_H

#include "ReferenceFrame.h"
#include <set>
#include <list>
#include <map>
#include <variant>

//
// An optical element is something that:
//
//  1. Is defined on top of a reference frame
//  2. May define additional axes and points on that reference frame
//  3. May provide additional reference frames
//  4. May include a method to draw an OpenGL representation
//  5. Define several properties

namespace RZ {
  class OMModel;
  class GenericCompositeModel;
  class OpticalElement;
  class Detector;
  class OpticalPath;
  struct UndefinedProperty {};

  enum PropertyValueType {
    UndefinedValue,
    IntegerValue,
    RealValue,
    BooleanValue,
    StringValue
  };

  typedef std::variant<UndefinedProperty, int64_t, Real, bool, std::string> BasePropertyVariant;
  class PropertyValue : public BasePropertyVariant {
      // Cobolization of C++ will never end. I need this specific construct to
      // inherit constructors from BasePropertyVariant
      using BasePropertyVariant::BasePropertyVariant;

    public:
      static inline PropertyValue
      undefined()
      {
        return PropertyValue();
      }

      inline bool
      isUndefined() const
      {
        return index() == 0;
      }

      inline PropertyValueType
      type() const
      {
        return static_cast<PropertyValueType>(index());
      }

      template<typename T>
      inline operator T() const
      {
        switch (index()) {
          case 1:
            return std::get<int64_t>(*this);
            break;

          case 2:
            return std::get<Real>(*this);
            break;

          case 3:
            return std::get<bool>(*this);
            break;
        }
        
        return T();
      }
  };

  class ElementFactory;

  class Element {
      std::string            m_name;
      Element               *m_parent      = nullptr;
      ReferenceFrame        *m_parentFrame = nullptr;
      GenericCompositeModel *m_parentModel = nullptr;
      ElementFactory        *m_factory     = nullptr;
      
      std::list<Element *>                    m_children;
      std::list<ReferenceFrame *>             m_portList;
      std::map<std::string, ReferenceFrame *> m_nameToPort;
      std::vector<std::string>                m_sortedProperties;
      std::map<std::string, PropertyValue>    m_properties;

      // Representation state
      bool m_selected  = false;
      bool m_visible   = true;

      // Default appearence
      Real m_shiny     = 64;
      Real m_red       = .25;
      Real m_green     = .25;
      Real m_blue      = .25;

      Real m_specRed   = .25;
      Real m_specGreen = .25;
      Real m_specBlue  = .25;
      
      void pushChild(Element *);
      
    protected:
      // Non-positional hidden arguments
      unsigned m_hidden = 0;

      void material(std::string const &role);
      void registerProperty(std::string const &, PropertyValue const &);
      void refreshProperties();
      
      // Takes ownership
      ReferenceFrame *registerPort(std::string const &, ReferenceFrame *);

      // Does not take ownership
      bool addPort(std::string const &, ReferenceFrame *);
      
      template <class T>
      inline T *
      registerPort(std::string const &name, T *frame)
      {
        auto ret = registerPort(name, static_cast<ReferenceFrame *>(frame));
        return static_cast<T *>(ret);
      }

      virtual bool propertyChanged(std::string const &, PropertyValue const &);

      Element(
        ElementFactory *,
        std::string const &,
        ReferenceFrame *,
        Element *parent = nullptr);

    public:
      virtual ~Element();
      
      inline Real red() const { return m_red; }
      inline Real green() const { return m_green; }
      inline Real blue() const { return m_blue; }

      ElementFactory *
      factory() const
      {
        return m_factory;
      }

      inline GenericCompositeModel *
      parentModel()
      {
        return m_parentModel;
      }

      void
      setParentModel(GenericCompositeModel *model)
      {
        m_parentModel = model;
      }

      // Get name
      std::string const &
      name() const
      {
        return m_name;
      }

      // Determine whether it has a property
      inline bool
      hasProperty(std::string const &prop) const
      {
        return m_properties.find(prop) != m_properties.end();
      }

      // Get hidden parameter number
      inline unsigned
      hidden() const
      {
        return m_hidden;
      }

      // Get parent frame
      inline ReferenceFrame *
      parentFrame() const
      {
        return m_parentFrame;
      }

      inline bool
      visible() const
      {
        return m_visible;
      }

      // Enumerate ports
      std::set<std::string> ports() const;

      // Enumerate properties
      std::set<std::string> properties() const;
      
      // Sorted properties
      std::vector<std::string> sortedProperties() const;

      // Get the reference frame of one of these ports
      ReferenceFrame *getPortFrame(std::string const &) const;

      // Plug a new element to this port
      Element *plug(
        std::string const &port,
        std::string const &type,
        std::string const &name = "");


      template <class T>
      inline T *
      plug(
        std::string const &port,
        std::string const &type,
        std::string const &name = "")
      {
        auto ret = plug(port, type, name);
        return static_cast<T *>(ret);
      }

      // Set properties
      bool set(std::string const &, PropertyValue const &);
      
      // Get properties
      PropertyValue get(std::string const &) const;
      template <class T>
      T
      get(std::string const &name) const
      {
        return static_cast<T>(get(name));
      }

      // Representation methods
      void setSelected(bool);
      void setVisible(bool);

      // Representation interface
      virtual void enterOpenGL();
      virtual void nativeMaterialOpenGL(std::string const &role);
      virtual void renderOpenGL();
      virtual OMModel *nestedModel() const;
      virtual GenericCompositeModel *nestedCompositeModel() const;

      // Other helper methods
      Element *lookupElement(std::string const &name) const;
      OpticalElement *lookupOpticalElement(std::string const &name) const;
      Detector *lookupDetector(std::string const &name) const;
      const OpticalPath *lookupOpticalPath(std::string const &name) const;
  };

  class ElementFactory {
    public:
      virtual std::string name() const = 0;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) = 0;
  };
}

#endif // _ELEMENT_H
