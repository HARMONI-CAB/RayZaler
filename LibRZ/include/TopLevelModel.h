#ifndef _TOP_LEVEL_MODEL_H
#define _TOP_LEVEL_MODEL_H

#include <OMModel.h>
#include <GenericCompositeModel.h>

namespace RZ {
  class TopLevelModel : public GenericCompositeModel, public OMModel {
      OMModel *m_model = nullptr;

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
        GenericEvaluatorSymbolDict *dict) override;
    
    public:
      TopLevelModel(Recipe *recipe);
      ~TopLevelModel();

  };
}

#endif // _TOP_LEVEL_MODEL_H
