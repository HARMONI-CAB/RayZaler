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
        GenericEvaluatorSymbolDict *dict,
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
      virtual OpticalPath opticalPath() const override;
      ~CompositeElement();

      virtual void renderOpenGL() override;
      virtual OMModel *nestedModel() const override;
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
