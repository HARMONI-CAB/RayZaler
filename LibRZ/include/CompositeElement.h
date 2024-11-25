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

#ifndef _COMPOSITE_ELEMENT
#define _COMPOSITE_ELEMENT

#include "GenericCompositeModel.h"
#include "OpticalElement.h"
#include "ReferenceFrame.h"

namespace RZ {
  class OMModel;

  // This is all Bjarne Stroustrup's fault, he made me do this.
  class IHateCPlusPlus {
      OMModel *m_model = nullptr;

    public:
      IHateCPlusPlus(OMModel *model);
      inline OMModel *
      model() const
      {
        return m_model;
      }
  };

  class CompositeElement : public IHateCPlusPlus, public GenericCompositeModel, public OpticalElement {
      OMModel *m_model = nullptr; // Owned
      OpticalPath m_defaultPath;

    protected:
      // Interface methods
      virtual void registerDof(
        std::string const &name, 
        GenericModelParam *) override;

      virtual void registerParam(
        std::string const &name, 
        GenericModelParam *)  override;

      virtual void registerOpticalPath(
        std::string const &name,
        std::list<std::string> &params) override;
      
      virtual GenericEvaluator *allocateEvaluator(
        std::string const &expr,
        const GenericEvaluatorSymbolDict *dict,
        std::list<GenericCustomFunction *> const &functions,
        ExprRandomState *state) override;

      virtual void exposePort(
        std::string const &name,
        ReferenceFrame *frame) override;

      virtual bool propertyChanged(
        std::string const &name,
        PropertyValue const &val) override;

    public:
      CompositeElement(
        ElementFactory *factory,
        std::string const &name, 
        ReferenceFrame *pFrame,
        Recipe *recipe,
        GenericCompositeModel *parentCompositeModel,
        Element *parent = nullptr);
      virtual OpticalPath opticalPath(std::string const &name = "") const override;
      ~CompositeElement();

      virtual void renderOpenGL() override;
      virtual OMModel *nestedModel() const override;
      virtual GenericCompositeModel *nestedCompositeModel() const;
      
      virtual void notifyDetector(
        std::string const &preferredName,
        Detector *det) override;

      virtual void setRecordHits(bool) override;
      virtual void clearHits() override;
  };

  class CompositeElementFactory : public ElementFactory {
      Recipe *m_recipe = nullptr;
      GenericCompositeModel *m_owner = nullptr;
      std::string m_name;

    public:
      CompositeElementFactory(
        std::string const &,
        Recipe *,
        GenericCompositeModel *owner);
      virtual std::string name() const override;
      virtual Element *make(
        std::string const &name,
        ReferenceFrame *pFrame,
        Element *parent = nullptr) override;
  };
}

#endif // _COMPOSITE_ELEMENT
