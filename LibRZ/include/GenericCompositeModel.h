#ifndef _GENERIC_COMPOSITE_MODEL_H
#define _GENERIC_COMPOSITE_MODEL_H

#include <Vector.h>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <Random.h>

namespace RZ {
  struct ParamAssignExpression;
  struct RecipeContext;
  struct RecipeParameter;
  struct RecipeElementStep;
  
  class Recipe;
  class ReferenceFrame;
  class RotatedFrame;
  class TranslatedFrame;
  class Element;
  class OMModel;
  class ExprRandomState;
  class Detector;
  
#ifdef PYTHON_SCRIPT_SUPPORT
  class Script;
#endif // PYTHON_SCRIPT_SUPPORT

  enum GenericModelParamType {
    GENERIC_MODEL_PARAM_TYPE_ELEMENT,
    GENERIC_MODEL_PARAM_TYPE_ROTATED_FRAME,
    GENERIC_MODEL_PARAM_TYPE_TRANSLATED_FRAME,
  };

  class GenericModelParam;
  typedef std::map<std::string, GenericModelParam *> GenericEvaluatorSymbolDict;

  struct GenericCustomFunction {
    std::string name;
    unsigned argc;
    virtual Real evaluate (Real const *args, unsigned argc) = 0;
  };

  class GenericEvaluator {
      GenericEvaluatorSymbolDict *m_dict = nullptr;
      ExprRandomState            *m_randState = nullptr;
      ExprRandomState            *m_ownState  = nullptr;
      std::list<GenericCustomFunction *> m_funcList; // Borrowed

    protected:
      std::list<std::string> symbols() const;
      std::list<GenericCustomFunction *> functions() const;
      Real *resolve(std::string const &);
      ExprRandomState *randState() const;

    public:
      GenericEvaluator(GenericEvaluatorSymbolDict *, ExprRandomState *);
      virtual ~GenericEvaluator();
      virtual bool registerCustomFunction(GenericCustomFunction *);

      virtual std::list<std::string> dependencies() const = 0;
      virtual bool compile(std::string const &) = 0;
      virtual Real evaluate() = 0;
  };

  // This describes how a parameter of an element or a frame is calculated
  struct GenericComponentParamEvaluator {
    GenericModelParamType type; // What type of object we need to update
    ParamAssignExpression *description = nullptr; // Already contains an index
    GenericEvaluator      *evaluator = nullptr; // Owned
    std::string            assignString;
    int                    position = -1;

    union {
      Element         *element = nullptr;
      RotatedFrame    *rotation;
      TranslatedFrame *translation;
    };

    void assign();
    ~GenericComponentParamEvaluator();
  };

  // This describes what depends on what. This is what is exposed
  struct GenericModelParam {
    const RecipeParameter *description = nullptr;
    Real value = 0.; // Current value

    // What needs to be evaluated
    std::list<GenericComponentParamEvaluator *> dependencies;

    bool test(Real val);
  };

  // Serves as storage
  class CompositeElementFactory;
  class ElementFactory;

  class GenericCompositeModel {
      OMModel                      *m_model = nullptr;           // Borrowed
      Recipe                       *m_recipe = nullptr;          // Borrowed
      Element                      *m_parent = nullptr;          // Borrowed
      GenericCompositeModel        *m_parentModel = nullptr;     // Borrowed
      
      GenericEvaluatorSymbolDict    m_global;
      std::vector<ReferenceFrame *> m_frames;                    // Borrowed (m_model)
      std::vector<Element *>        m_elements;                  // Borrowed (m_model)
      
      std::list<CompositeElementFactory *> m_customFactoryList;  // Owned
      std::map<std::string, CompositeElementFactory *> m_customFactories;
      std::list<GenericComponentParamEvaluator *> m_expressions; // Owned
      ExprRandomState               m_ownState;
      ExprRandomState              *m_randState;

#ifdef PYTHON_SCRIPT_SUPPORT
      std::list<Script *>           m_scripts;
#endif // PYTHON_SCRIPT_SUPPORT

      unsigned int m_completedFrames = 0;
      unsigned int m_completedElements = 0;

      // Generic parameter storage (dofs and params)
      std::list<GenericModelParam *> m_genParamStorage;          // Owned

      // What is dof and what is param
      std::map<std::string, GenericModelParam *> m_params;       // Borrowed
      std::map<std::string, GenericModelParam *> m_dofs;         // Borrowed

      std::string m_prefix;

      bool m_constructed = false;

      bool registerCustomFactory(CompositeElementFactory *);
      ElementFactory *lookupElementFactory(const std::string &, bool &) const;

      void initGlobalScope();
      void registerCustomElements();
      void createFrames(ReferenceFrame *);
      void loadScripts();
      void resolvePorts();
      void createDelayedElements();
      void createElementInside(RecipeElementStep *step, ReferenceFrame *pFrame);
      void createElements(ReferenceFrame *);
      void createParams();
      void createLocalExpressions(
        GenericEvaluatorSymbolDict &,
        RecipeContext *);

      void delayedCreationLoop();
      void createExpressions();
      void exposeOpticalPaths();
      void exposePorts();
      
      GenericComponentParamEvaluator *makeExpression(
        std::string const &expr,
        GenericEvaluatorSymbolDict *dict);
        
      static bool getLastDottedElement(
        std::string const &,
        std::string &prefix,
        std::string &suffix);

    protected:
      GenericModelParam *allocateParam();
      GenericEvaluatorSymbolDict const &symbolDict() const;

      // Interface methods
      virtual void registerDof(
        std::string const &name, 
        GenericModelParam *) = 0;

      virtual void registerParam(
        std::string const &name, 
        GenericModelParam *) = 0;

      virtual void registerOpticalPath(
        std::string const &name,
        std::list<std::string> &params) = 0;
      
      virtual GenericEvaluator *allocateEvaluator(
        std::string const &expr,
        GenericEvaluatorSymbolDict *dict,
        std::list<GenericCustomFunction *> const &functions,
        ExprRandomState *state) = 0;

      virtual void exposePort(
        std::string const &name,
        ReferenceFrame *frame);

      ReferenceFrame *getFrameOfContext(const RecipeContext *) const;

    public:
      GenericCompositeModel(
        Recipe *,
        OMModel *,
        GenericCompositeModel *parentModel = nullptr,
        Element *parent = nullptr);
      virtual ~GenericCompositeModel();
    
      std::list<std::string> params() const;
      std::list<std::string> dofs() const;

      GenericModelParam *lookupParam(std::string const &);
      GenericModelParam *lookupDof(std::string const &);
      GenericCompositeModel *parentCompositeModel() const;

      bool loadScript(std::string const &path);
      bool setParam(std::string const &, Real);
      bool setDof(std::string const &, Real);

      std::string resolveFilePath(std::string const &) const;

      void assignEverything();
      void updateRandState();
      void setRandomState(ExprRandomState * state);
      ExprRandomState *randState() const;
      
      // Takes the recipe and constructs elements
      void build(ReferenceFrame *, std::string const &prefix = "");

      virtual void notifyDetector(
        std::string const &preferredName,
        Detector *det) = 0;
  };
}

#endif // _GENERIC_COMPOSITE_MODEL_H
