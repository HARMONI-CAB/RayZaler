#ifndef _RECIPE_H
#define _RECIPE_H

#include <string>
#include <list>
#include <map>
#include <OpticalElement.h>
#include <cmath>

namespace RZ {
  class OMModel;
  class RecipeContext;
  class Recipe;

  struct ParamAssignExpression {
    int s_target = -1;
    int s_index = -1;

    std::string parameter;
    std::string expression;

    RecipeContext *parent = nullptr;
  };

  struct RecipeParamListProto {
    std::vector<std::string> params;
    std::map<std::string, std::string> values;
    std::list<std::string> defined;

    unsigned int positionalNdx = 0;
    unsigned int nonPositionalNdx = 0;
    
    void pushParam(std::string const &name, std::string const &value = "") {
      params.push_back(name);
      values[name] = value;
    }

    std::string &
    operator[](std::string const &name)
    {
      return values[name];
    }

    bool isSet(std::string const &name) const;

    void set(std::string const &, std::string const &);
  };

  struct RecipeElementStep {
    std::string name;
    std::string factory;
    std::list<ParamAssignExpression *> positionalParams;
    std::map<std::string, ParamAssignExpression *> params;

    int s_index = -1;
    bool delayedCreation = false;
    RecipeContext *parent = nullptr;
    Recipe *owner = nullptr;
    
    void set(std::string const &name, std::string const &expr);
  };

  struct RecipeParameter {
    Real defaultVal;
    Real min;
    Real max;
  };

  enum RecipeContextType {
    RECIPE_CONTEXT_TYPE_ROOT,
    RECIPE_CONTEXT_TYPE_ROTATION,
    RECIPE_CONTEXT_TYPE_TRANSLATION,
    RECIPE_CONTEXT_TYPE_PORT
  };

  struct RecipeContext {
    std::string name;        // May be empty
    RecipeContextType type;

    std::list<RecipeContext *>     contexts;
    std::list<RecipeElementStep *> elements;
    
    std::list<std::string>                         varNames;
    std::map<std::string, ParamAssignExpression *> variables;
    std::map<std::string, ParamAssignExpression *> params;

    int s_index = -1;

    bool delayed = false;
    RecipeContext *parent = nullptr;
    Recipe *owner = nullptr;
    std::string parentNS;
    
    // Only set if type == PORT
    RecipeElementStep *element;
    std::string port;

    inline std::string
    currNS() const
    {
      if (parentNS.size() == 0)
        return name;

      return name.size() > 0 
      ? parentNS + "." + name 
      : parentNS;
    }

    std::string to_string() const;
  };

  struct RecipeOpticalPath {
    std::string name;
    std::list<std::string> steps;
    RecipeContext *parent = nullptr;

    inline void
    plug(std::string const &element)
    {
      steps.push_back(element);
    }
  };

  class Recipe {
      RecipeContext *m_rootContext = nullptr;
      Recipe *m_parent = nullptr;

      // Search paths (forwarded to the composite model)
      std::list<std::string>             m_searchPaths;
      
      // Allocation
      std::vector<RecipeContext *>       m_contexts;        // Tracks frames
      std::vector<RecipeElementStep *>   m_elementSteps;    // Tracks elements
      std::vector<RecipeOpticalPath *>   m_pathSteps;       // Optical paths
      std::vector<Recipe *>              m_subRecipes;      // Subrecipes to construct elements
      std::list<std::string>             m_scripts;         // Python scripts

      std::vector<ParamAssignExpression *> m_elemParameters;  // Tracks how to configure elements
      std::vector<ParamAssignExpression *> m_frameParameters; // Tracks how to configure frames
      std::vector<ParamAssignExpression *> m_variables;       // Tracks how to update variables

      std::map<std::string, RecipeContext *>     m_frames;
      std::map<std::string, RecipeElementStep *> m_elements;
      std::map<std::string, Recipe *>            m_customElements; // Tracks composite elements
      std::map<std::string, RecipeOpticalPath *> m_paths;             // Tracks optical paths
      std::map<std::string, RecipeParameter>     m_parameters;
      std::map<std::string, RecipeParameter>     m_dofs;
      std::map<std::string, RecipeContext *>     m_ports;

      // Set during edition
      RecipeContext *m_currContext = nullptr;
      unsigned int   m_nestedPorts = 0;

      std::string currNS() const;

    private:
      std::string genElementName(std::string const &type);
      std::string genReferenceFrameName(std::string const &type);

      std::string genElementName(std::string const &parent, std::string const &type);
      std::string genReferenceFrameName(std::string const &parent, std::string const &type);

      RecipeContext *makeContext(RecipeContext *);
      RecipeElementStep *makeElementStep(RecipeContext *);
      RecipeOpticalPath *makeOpticalPathStep(RecipeContext *);
      RecipeParameter *makeRecipeParam(std::map<std::string, RecipeParameter> &, std::string const &);

      ParamAssignExpression *makeElementParameter(
        RecipeElementStep *elem,
        std::string const &name,
        std::string const &expression);
      
      ParamAssignExpression *makeReferenceFrameParameter(
        RecipeContext *ctx,
        std::string const &name,
        std::string const &expression);
      
      ParamAssignExpression *makeVariable(
        RecipeContext *ctx,
        std::string const &name,
        std::string const &expression);

      void push(RecipeContext *ctx);

    public:
      Recipe(std::string const &, Recipe *);
      Recipe();
      ~Recipe();

      inline std::list<std::string> const &
      scripts() const
      {
        return m_scripts;
      }
      
      inline std::list<std::string> const &
      searchPaths() const
      {
        return m_searchPaths;
      }
      
      inline RecipeContext *
      currentContext() const
      {
        return m_currContext;
      }

      Recipe *parent() const;

      RecipeContext *rootContext() const;
      std::vector<RecipeContext *> const &contexts() const;
      std::vector<RecipeElementStep *> const &elements() const;
      std::vector<RecipeOpticalPath *> const &paths() const;
      std::map<std::string, RecipeContext *> const &ports() const;

      std::map<std::string, Recipe *> const &customElements() const;
      std::map<std::string, RecipeParameter> const &dofs() const;
      std::map<std::string, RecipeParameter> const &params() const;
      
      RecipeContext     *lookupReferenceFrame(std::string const &) const;
      RecipeElementStep *lookupElement(std::string const &) const;
      RecipeOpticalPath *lookupOpticalPath(std::string const & = "") const;
      RecipeElementStep *resolveElement(std::string const &) const;
      Recipe            *makeCustomElement(std::string const &);
      
      bool addScript(std::string const &scriptPath);
      void pushSearchPath(std::string const &path);
      void pushVariable(std::string const &name, std::string const &value);

      void pushRotation(
        std::string const &angle,
        std::string const &eX,
        std::string const &eY,
        std::string const &eZ,
        std::string const &name = "");
      
      void pushTranslation(
        std::string const &dX,
        std::string const &dY,
        std::string const &dZ,
        std::string const &name = "");

      void pushPortContext(RecipeElementStep *, std::string const &port);

      bool pop();

      RecipeElementStep *addElement(
        std::string const &name,
        std::string const &factory,
        std::map<std::string, std::string> const &parameters 
          = std::map<std::string, std::string>());
      
      RecipeOpticalPath *allocatePath(std::string const &name = "");

      void addPort(std::string const &name);
      
      bool addDof(
        std::string const &name,
        Real defVal,
        Real min = -INFINITY,
        Real max = +INFINITY);
      
      bool addParam(
        std::string const &name,
        Real defVal,
        Real min = -INFINITY,
        Real max = +INFINITY);

      void debug();

    friend class RecipeElementStep;
  };

  class GenericCompositeModel;
}
#endif // _RECIPE_H
