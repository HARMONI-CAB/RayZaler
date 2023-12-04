#include <Python.h>
#include <ScriptLoader.h>
#include <Logger.h>

using namespace RZ;


static PyObject *
registerFunction(PyObject *self, PyObject *args)
{
  PyObject *pFunc      = nullptr;
  ScriptFunction newFunc;
  unsigned int argno = 0;
  Script *script = ScriptLoader::instance()->getCurrentScript();
  const char *name;

  if (script == nullptr) {
    PyErr_SetString(
      PyExc_KeyError,
      "Calling register outside script loading context");
    goto done;
  }
  
  if (!PyArg_ParseTuple(args, "sIO", &name, &argno, &pFunc))
    goto done;
  
  if (!PyCallable_Check(pFunc)) {
    PyErr_SetString(PyExc_ValueError, "Object is not callable");
    goto done;
  }

  newFunc.argc  = argno;
  newFunc.name  = name;
  newFunc.pFunc = pFunc;

  if (script->registerFunction(newFunc)) {
    Py_INCREF(pFunc);
  } else {
    PyErr_SetString(
      PyExc_KeyError,
      "There is a function already registered with this name");
    goto done;
  }

  Py_RETURN_NONE;

done:
  return nullptr;
}

static PyMethodDef g_methods[] = {
  {
    "register",
    registerFunction,
    METH_VARARGS,
   "Register a Python function as a RayZaler expression function"
  },
  {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef g_module = {
  PyModuleDef_HEAD_INIT,
  "RZLink",
  "RayZaler C++ API link",
  -1,
  g_methods
};

PyMODINIT_FUNC PyInit_RZ() {
  return PyModule_Create(&g_module);
}
