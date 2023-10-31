#ifndef _SCRIPT_LOADER_H
#define _SCRIPT_LOADER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string>
#include <list>
#include <map>
#include "GenericCompositeModel.h"

namespace RZ {
  struct ScriptFunction : public GenericCustomFunction {
    using GenericCustomFunction::GenericCustomFunction;
    PyObject *pFunc = nullptr;

    virtual Real evaluate(Real const *args, unsigned argc) override;
  };

  class ScriptLoader;

  class Script {
      std::string m_path;
      PyObject   *m_pModule = nullptr;

      std::list<ScriptFunction>               m_functions;
      std::map<std::string, ScriptFunction *> m_nameToFunction;

      Script(std::string const &);
      ~Script();

      bool load(std::string const &moduleName);

    public:
      friend class ScriptLoader;
      bool registerFunction(ScriptFunction const &);
      std::list<ScriptFunction> &customFunctions() const;
  };

  class ScriptLoader {
      PyObject                       *m_pSys = nullptr;
      PyObject                       *m_pSysPath = nullptr;
      pthread_mutex_t                 m_registerMutex;
      Script                         *m_currScript = nullptr;
      std::map<std::string, Script *> m_pathToScript;
      static ScriptLoader            *m_instance;
      
      bool enableScriptDirectory(std::string const &);
      bool explodeScriptPath(
        std::string const &,
        std::string &,
        std::string &);
      
      bool pythonLibraryInit();

      ScriptLoader();
      ~ScriptLoader();

    public:
      static ScriptLoader *instance();
      Script *load(std::string const &path);
      Script *getCurrentScript();
  }; 
}

#endif // _SCRIPT_LOADER_H
