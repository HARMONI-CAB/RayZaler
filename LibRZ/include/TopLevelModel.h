#ifndef _TOP_LEVEL_MODEL_H
#define _TOP_LEVEL_MODEL_H

#include <OMModel.h>
#include <GenericCompositeModel.h>
#include <map>

namespace RZ {
  class ApertureStop;

  class TopLevelModel : public GenericCompositeModel, public OMModel {
      std::map<std::string, ReferenceFrame *> m_focalPlanes;

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
        ExprRandomState *) override;
    
      void exposePort(
          std::string const &,
          ReferenceFrame *) override;

    public:
      TopLevelModel(Recipe *recipe);
      ~TopLevelModel();

      std::list<std::string> focalPlanes() const;
      std::list<std::string> apertureStops() const;

      ApertureStop   *getApertureStop(std::string const &) const;
      ReferenceFrame *getFocalPlane(std::string const &) const;
  };
}

#endif // _TOP_LEVEL_MODEL_H
