#include <ScriptLoader.h>
#include <Logger.h>

using namespace RZ;

PyMODINIT_FUNC PyInit_RZ();


static std::string
getLastPythonError()
{
  std::string result;

  PyObject *pType = nullptr, *pValue = nullptr, *pTraceback = nullptr;
  PyErr_Fetch(&pType, &pValue, &pTraceback);

  if (pType != nullptr)
    result = PyUnicode_AsUTF8(pType);
  else
    result = "(No exceptions)";

  Py_XDECREF(pType);
  Py_XDECREF(pValue);
  Py_XDECREF(pTraceback);

  return result;
}

Real
ScriptFunction::evaluate(Real const *args, unsigned argc)
{
  PyGILState_STATE state = PyGILState_Ensure();
  PyObject *pArgs = PyTuple_New(argc);
  Real result = std::numeric_limits<Real>::quiet_NaN();
  Real suggested;

  for (unsigned i = 0; i < argc; ++i)
    PyTuple_SetItem(pArgs, i, PyFloat_FromDouble(args[i]));

  PyObject *retVal = PyObject_CallObject(pFunc, pArgs);
  Py_DECREF(pArgs);

  if (retVal != nullptr) {
    suggested = PyFloat_AsDouble(retVal);
    if (PyErr_Occurred()) {
      RZError(
        "%s: failed to convert result to Real: %s\n",
        name.c_str(),
        getLastPythonError().c_str());
    } else {
      result = suggested;
    }
  } else {
    if (PyErr_Occurred()) {
      RZError(
        "%s: function failed: %s\n",
        name.c_str(),
        getLastPythonError().c_str());
    }
  }
  
  Py_DECREF(retVal);
  PyGILState_Release(state);

  return result;
}

bool
ScriptLoader::pythonLibraryInit()
{
  PyImport_AppendInittab("RZ", PyInit_RZ);
  Py_Initialize();

  return true;
}


ScriptLoader::ScriptLoader(std::string const &path)
{
  m_path = path;
}

ScriptLoader::~ScriptLoader()
{
  Py_XDECREF(m_pModule);

  for (auto &p : m_functions)
    Py_DECREF(p.pFunc);
}

bool
ScriptLoader::registerFunction(ScriptFunction const &func)
{
  if (m_nameToFunction.find(func.name) != m_nameToFunction.end()) {
    RZError(
      "Python: Cannot register function `%s': already registered\n",
      func.name.c_str());
    return false;
  }

  m_functions.push_back(func);
  m_nameToFunction[func.name] = &m_functions.back();

  return true;
}

bool
ScriptLoader::load()
{
  PyObject *pModule  = nullptr;
  PyObject *pCapsule = nullptr;
  PyObject *pGlobals = nullptr;

  bool ok = false;

  if (m_loaded) {
    ok = true;
    goto done;
  }

  // Encapsulate pointer to loader
  pCapsule = PyCapsule_New(this, nullptr, nullptr);
  if (pCapsule == nullptr) {
    RZError("Python: Failed to create python capsule for loader\n");
    goto done;
  }

  // Create global dictionary
  pGlobals = PyDict_New();
  if (pGlobals == nullptr) {
    RZError("Python: Failed to create global dictionary\n");
    goto done;
  }

  PyDict_SetItemString(pGlobals, "_CPP_API", pCapsule);

  pModule = PyImport_ImportModuleEx(m_path.c_str(), pGlobals, nullptr, nullptr);
  if (pModule == nullptr) {
    RZError(
      "Failed to load `%s': %s\n",
      m_path.c_str(),
      getLastPythonError().c_str());
  }

  ok = true;

done:
  if (ok)
    std::swap(m_pModule, pModule);
  
  Py_XDECREF(pModule);
  Py_XDECREF(pGlobals);
  Py_XDECREF(pCapsule);

  return ok;
}
