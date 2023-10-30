#include <Python.h>
#include <ScriptLoader.h>

using namespace RZ;

static ScriptLoader *
getScriptLoaderRef()
{
  PyObject *globals    = PyEval_GetGlobals();
  PyObject *capsule    = nullptr;
  void *ptr            = nullptr;
  ScriptLoader *loader = nullptr;
  PyObject *key        = PyUnicode_FromString("_CPP_API");

  if (key == nullptr) {
    PyErr_SetString(PyExc_RuntimeError, "Failed to initialize C++ API link");
    goto done;

  }
  if (!PyDict_Check(globals)) {
    PyErr_SetString(PyExc_RuntimeError, "Globals not a dictionary");
    goto done;
  }

  capsule = PyDict_GetItem(globals, key);
  if (capsule == nullptr) {
    PyErr_SetString(PyExc_RuntimeError, "Calling outside RayZaler context!");
    goto done;
  }

  if (!PyCapsule_CheckExact(capsule)) {
    PyErr_SetString(PyExc_RuntimeError, "C++ API link was overwritten by script!");
    goto done;
  }

  ptr = PyCapsule_GetPointer(capsule, nullptr);
  if (ptr == nullptr) {
    PyErr_SetString(PyExc_RuntimeError, "Cannot retrieve link to C++ API!");
    goto done;
  }

  loader = reinterpret_cast<ScriptLoader *>(ptr);

done:
  Py_XDECREF(key);

  return loader;
}

static PyObject *
registerFunction(PyObject *self, PyObject *args)
{
  PyObject *pName      = nullptr;
  PyObject *pFunc      = nullptr;
  ScriptFunction newFunc;
  unsigned int argno = 0;
  ScriptLoader *loader = getScriptLoaderRef();
  std::string name;

  if (loader == nullptr)
    goto done;
  
  if (!PyArg_ParseTuple(args, "sIO", &pName, &argno, &pFunc))
    goto done;
  
  if (!PyCallable_Check(pFunc)) {
    PyErr_SetString(PyExc_ValueError, "Object is not callable");
    goto done;
  }

  newFunc.argc  = argno;
  newFunc.name  = PyUnicode_AsUTF8(pName);
  newFunc.pFunc = pFunc;

  if (loader->registerFunction(newFunc)) {
    Py_INCREF(pFunc);
  } else {
    PyErr_SetString(
      PyExc_KeyError,
      "There is a function already registered with this name");
    goto done;
  }

done:
  return nullptr;
}

static PyMethodDef g_methods[] = {
  {"register", registerFunction, METH_VARARGS,
   "Register a Python function as a RayZaler expression function"},
  {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef g_module = {
  PyModuleDef_HEAD_INIT,
  "RZ",
  "RayZaler C++ API link",
  -1,
  g_methods
};

PyMODINIT_FUNC PyInit_RZ() {
  return PyModule_Create(&g_module);
}
