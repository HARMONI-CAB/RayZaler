#include <ScriptLoader.h>
#include <Logger.h>
#include <libgen.h>
#include <cstring>

using namespace RZ;

PyMODINIT_FUNC PyInit_RZ();

///////////////////////////////// Helper functions /////////////////////////////
static std::string
getLastPythonError()
{
  std::string result;

  PyObject *pType = nullptr, *pValue = nullptr, *pTraceback = nullptr;
  PyErr_Fetch(&pType, &pValue, &pTraceback);

  if (pValue != nullptr) {
    const char *err = nullptr;
    PyObject *pStr = PyObject_Str(pValue);

    if (pStr != nullptr) 
      err = PyUnicode_AsUTF8(pStr);

    if (err == nullptr)
        err = "(null error)";
    result = err;
  } else {
    result = "(No exceptions)";
  }

  Py_XDECREF(pType);
  Py_XDECREF(pValue);
  Py_XDECREF(pTraceback);

  return result;
}

////////////////////////////////// Script function /////////////////////////////
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

/////////////////////////////////// Script /////////////////////////////////////
Script::Script(std::string const &path)
{
  m_path = path;
}

Script::~Script()
{
  Py_XDECREF(m_pModule);

  for (auto &p : m_functions)
    Py_DECREF(p.pFunc);
}

bool
Script::load(std::string const &name)
{
  PyObject *pModule;

  bool ok = false;

  pModule = PyImport_ImportModuleEx(name.c_str(), nullptr, nullptr, nullptr);
  if (pModule == nullptr) {
    RZError(
      "Failed to load `%s': %s\n",
      m_path.c_str(),
      getLastPythonError().c_str());
    goto done;
  }

  ok = true;

done:
  if (ok)
    std::swap(m_pModule, pModule);
  
  Py_XDECREF(pModule);

  return ok;
}

bool
Script::registerFunction(ScriptFunction const &func)
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

std::list<ScriptFunction> &
Script::customFunctions() const
{
  return const_cast<std::list<ScriptFunction> &>(m_functions);
}

///////////////////////////////// ScriptLoader /////////////////////////////////
bool
ScriptLoader::pythonLibraryInit()
{
  PyImport_AppendInittab("RZLink", PyInit_RZ);
  Py_Initialize();

  m_pSys = PyImport_ImportModule("sys");
  if (m_pSys == nullptr) {
    RZError("Python initialization fatal error: failed to import sys module\n");
    return false;
  }

  m_pSysPath = PyObject_GetAttrString(m_pSys, "path");
  if (m_pSysPath == nullptr) {
    RZError("Python initialization fatal error: failed to find sys.path\n");
    return false;
  }

  if (pthread_mutex_init(&m_registerMutex, nullptr) != 0) {
    RZError("Failed to initialize registration mutex\n");
    return false;
  }

  return true;
}

Script *
ScriptLoader::getCurrentScript()
{
  return m_currScript;
}


ScriptLoader::ScriptLoader()
{
}

ScriptLoader::~ScriptLoader()
{ 
}

bool
ScriptLoader::enableScriptDirectory(std::string const &path)
{
  bool ok = false;
  bool found = false;
  PyObject *pPath = nullptr;
  PyObject *pDirPath = nullptr;

  auto size = PyList_Size(m_pSysPath);
  for (unsigned i = 0; !found && i < size; ++i) {
    pPath = PyList_GetItem(m_pSysPath, i);
    if (pPath != nullptr && PyUnicode_Check(pPath)) {
      const char *thisPath = PyUnicode_AsUTF8(pPath);
      if (strcmp(path.c_str(), thisPath) == 0)
        found = true;
    }
  }

  if (!found) {
    pDirPath = PyUnicode_FromString(path.c_str());
    if (pDirPath == nullptr) {
      RZError("Failed to create string object\n");
      goto done;
    }

    if (PyList_Insert(m_pSysPath, 0, pDirPath) != 0) {
      RZError("Failed to insert new path in sys.path\n");
      goto done;
    }
  }

  ok = true;

done:
  Py_XDECREF(pDirPath);
  return ok;
}

bool
ScriptLoader::explodeScriptPath(
  std::string const &path,
  std::string &dir,
  std::string &name)
{
  char *dirNameCopy  = nullptr;
  char *baseNameCopy = nullptr;
  char *strDirName, *strBaseName;
  char *ext = nullptr;

  bool ok = false;
  
  dirNameCopy = strdup(path.c_str());
  if (dirNameCopy == nullptr) {
    RZError("Failed to duplicate script path (directory)\n");
    goto done;
  }

  baseNameCopy = strdup(path.c_str());
  if (baseNameCopy == nullptr) {
    RZError("Failed to duplicate script path (directory)\n");
    goto done;
  }
  
  strDirName  = dirname(dirNameCopy);
  strBaseName = basename(baseNameCopy);
  
  ext = strrchr(strBaseName, '.');
  if (ext == nullptr || strcmp(ext, ".py") != 0) {
    RZError("%s: invalid script name. Scripts must have the .py extension.\n", strBaseName);
    goto done;
  }

  *ext = '\0';

  dir  = strDirName;
  name = strBaseName;

  ok = true;

done:
  if (dirNameCopy != nullptr)
    free(dirNameCopy);

  if (baseNameCopy != nullptr)
    free(baseNameCopy);
  
  return ok;
}

Script *
ScriptLoader::load(std::string const &path)
{
  Script *script = nullptr;
  std::string dirName, modName;
  bool mutexAcquired = false;

  // Already loaded?
  auto it = m_pathToScript.find(path);
  if (it != m_pathToScript.end()) {
    script = it->second;
    goto done;
  }

  // Not quite, load it now
  if (!explodeScriptPath(path, dirName, modName))
    goto done;
  
  (void) pthread_mutex_lock(&m_registerMutex);
  mutexAcquired = true;

  if (!enableScriptDirectory(dirName))
    goto done;
  
  script = new Script(path);
  
  m_currScript = script;

  if (!script->load(modName)) {
    delete script;
    script = nullptr;
    goto done;
  }

  // Register
  m_pathToScript[path] = script;

done:
  if (mutexAcquired)
    (void) pthread_mutex_unlock(&m_registerMutex);
  
  m_currScript = nullptr;

  return script;
}

ScriptLoader *ScriptLoader::m_instance = nullptr;

ScriptLoader *
ScriptLoader::instance()
{
  if (m_instance ==  nullptr) {
    m_instance = new ScriptLoader();

    if (!m_instance->pythonLibraryInit()) {
      delete m_instance;
      m_instance = nullptr;
    }
  }

  return m_instance;
}
