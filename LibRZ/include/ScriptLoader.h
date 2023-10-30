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

  class ScriptLoader {
      std::string m_path;
      PyObject   *m_pModule = nullptr;

      bool        m_loaded  = false;

      std::list<ScriptFunction>               m_functions;
      std::map<std::string, ScriptFunction *> m_nameToFunction;

    public:
      ScriptLoader(std::string const &);
      ~ScriptLoader();

      bool registerFunction(ScriptFunction const &);

      static bool pythonLibraryInit();
      bool load();
  };
}

#endif // _SCRIPT_LOADER_H
