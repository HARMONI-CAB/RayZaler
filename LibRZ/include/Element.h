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

#ifndef _ELEMENT_H
#define _ELEMENT_H

#include "ReferenceFrame.h"
#include "Helpers.h"
#include "ElementMacros.h"
#include <set>
#include <list>
#include <map>
#include <variant>
#include <GLHelpers.h>

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
  struct ElementFactoryMeta;
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

      std::string m_description;
      std::string m_context;
      bool        m_hidden = false;

    public:
      inline void
      setHidden(bool hidden)
      {
        m_hidden = hidden;
      }

      bool
      isHidden() const
      {
        return m_hidden;
      }

      inline void
      setDescription(std::string const &desc)
      {
        m_description = desc;
      }

      inline void
      setContext(std::string const &ctx)
      {
        m_context = ctx;
      }

      inline std::string
      description() const
      {
        return m_description;
      }

      inline std::string
      context() const
      {
        return m_context;
      }

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

      inline int64_t asInteger()    const { return std::get<int64_t>(*this); }
      inline Real    asReal()       const { return std::get<Real>(*this);    }
      inline bool    asBool()       const { return std::get<bool>(*this);    }
      inline std::string asString() const { return std::get<std::string>(*this); }
  };

  template<>
  inline PropertyValue::operator bool() const
  {
    std::string val;

    switch (type()) {
      case IntegerValue:
        return std::get<int64_t>(*this) != 0;
      
      case RealValue:
        return std::get<Real>(*this) < -0.5 
        || std::get<Real>(*this) > 0.5;

      case BooleanValue:
        return std::get<bool>(*this);
      
      case StringValue:
        return val == "1" || iequals(val, "yes") || iequals(val, "true");
    }

    return false;
  }
  
  class ElementFactory;

  class Element {
      std::string            m_className;
      std::string            m_name;
      Element               *m_parent      = nullptr;
      ReferenceFrame        *m_parentFrame = nullptr;
      GenericCompositeModel *m_parentModel = nullptr;
      ElementFactory        *m_factory     = nullptr;
      Vec3                   m_bb1;
      Vec3                   m_bb2;
      std::vector<GLfloat>   m_bbLines;
      
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
      void registerProperties(const ElementFactoryMeta *meta);

    protected:
      void material(std::string const &role);
      void refreshProperties();
      void registerProperty(
        std::string const &,
        PropertyValue const &,
        std::string const &);
        
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

      void setBoundingBox(Vec3 const &p1, Vec3 const &p2);
      void updatePropertyValue(std::string const &, PropertyValue const &);
      
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

      inline PropertyValueType
      queryPropertyType(std::string const &prop) const
      {
        auto it = m_properties.find(prop);

        if (it == m_properties.end())
          return UndefinedValue;

        return it->second.type();
      }

      inline bool
      propertyIsHidden(std::string const &prop) const
      {
        auto it = m_properties.find(prop);

        if (it == m_properties.end())
          return false;

        return it->second.isHidden();
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

      // Refresh all reference frames depending on parent
      void refreshFrames();
      
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
      void setDefaults();

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
      void boundingBox(Vec3 &p1, Vec3 &p2) const;
      
      // Representation interface
      virtual void enterOpenGL();
      virtual void nativeMaterialOpenGL(std::string const &role);
      virtual void renderOpenGL();
      
      void calcBoundingBoxOpenGL();
      void renderBoundingBoxOpenGL();

      virtual OMModel *nestedModel() const;
      virtual GenericCompositeModel *nestedCompositeModel() const;

      // Other helper methods
      Element *lookupElement(std::string const &name) const;
      OpticalElement *lookupOpticalElement(std::string const &name) const;
      Detector *lookupDetector(std::string const &name) const;
      const OpticalPath *lookupOpticalPath(std::string const &name) const;
  };

  struct ElementFactoryMeta {
    ElementFactoryMeta                  *parent;           // Parent factory meta
    std::string                          name;             // Name of the element factory
    std::string                          description;      // Description
    std::map<std::string, PropertyValue> properties;       // Default properties
    std::vector<std::string>             sortedProperties; // Properties in addition order

    PropertyValue *queryProperty(std::string const &);
  };

  class ElementFactory {
      std::list<ElementFactoryMeta> m_meta;
      ElementFactoryMeta *m_metaData = nullptr;

    protected:
      void enterDecls(std::string const &, std::string const &);
      PropertyValue &hiddenProperty(
        std::string const &,
        PropertyValue const &,
        std::string const &);
      
      void property(
        std::string const &,
        PropertyValue const &,
        std::string const &);

    public:
      ElementFactory();

      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) = 0;

      std::string name() const;
      const ElementFactoryMeta *metaData() const;
      PropertyValue *queryProperty(std::string const &);
  };
}

#endif // _ELEMENT_H
