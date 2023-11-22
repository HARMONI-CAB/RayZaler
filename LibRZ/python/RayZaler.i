%module pyRayZaler
%{
#define SWIG_FILE_WITH_INIT
static PyObject* g_rzException;
%}

%include "stdint.i"
%include "std_vector.i"
%include "std_list.i"
%include "std_string.i"
%include "std_pair.i"
%include "numpy.i"

namespace std {
  %template(StringList) list<string>;
}

%init %{
  import_array();
  g_rzException = PyErr_NewException("_pyRayZaler.EngineError", nullptr, nullptr);
  Py_INCREF(g_rzException);
  PyModule_AddObject(m, "EngineError", g_rzException);
%}

%{
#define RZ_NO_HELPER_MACROS

#include <Vector.h>
#include <Matrix.h>
#include <Linalg.h>
#include <GenericCompositeModel.h>
#include <OMModel.h>
#include <TopLevelModel.h>

#include <ApertureStop.h>
#include <BlockElement.h>
#include <CompositeElement.h>
#include <ConvexLens.h>
#include <Detector.h>
#include <ExprTkEvaluator.h>
#include <FlatMirror.h>
#include <LensletArray.h>
#include <Obstruction.h>
#include <ParabolicMirror.h>
#include <ParserContext.h>
#include <PhaseScreen.h>
#include <RayBeamElement.h>
#include <Recipe.h>
#include <RectangularStop.h>
#include <RodElement.h>
#include <RotatedFrame.h>
#include <Singleton.h>
#include <SphericalMirror.h>
#include <TranslatedFrame.h>
#include <Tripod.h>
#include <TripodFrame.h>
#include <TubeElement.h>
#include <WorldFrame.h>

using namespace RZ;
%}

%exception {
  try {
    $action
  } catch (std::runtime_error &e) {
    PyErr_SetString(g_rzException, const_cast<char*>(e.what()));
    SWIG_fail;
  }
}

%include "Vector.h"
%include "Matrix.h"
%include "Linalg.h"
%include "ReferenceFrame.h"
%include "Element.h"
%include "OpticalElement.h"
%include "GenericCompositeModel.h"
%include "OMModel.h"
%include "TopLevelModel.h"

%include "ApertureStop.h"
%include "BlockElement.h"
%include "CompositeElement.h"
%include "ConvexLens.h"
%include "Detector.h"
%include "ExprTkEvaluator.h"
%include "FlatMirror.h"
%include "LensletArray.h"
%include "Obstruction.h"
%include "ParabolicMirror.h"
%include "ParserContext.h"
%include "PhaseScreen.h"
%include "RayBeamElement.h"
%include "RayTracingEngine.h"
%include "Recipe.h"
%include "RectangularStop.h"
%include "RodElement.h"
%include "RotatedFrame.h"
%include "Singleton.h"
%include "SphericalMirror.h"
%include "TranslatedFrame.h"
%include "Tripod.h"
%include "TripodFrame.h"
%include "TubeElement.h"
%include "WorldFrame.h"

%pythoncode %{
    EngineError = _pyRayZaler.EngineError
%}

%extend RZ::GenericCompositeModel {
  double
  __getitem__(std::string const &key)
  {
    auto param = self->lookupDof(key);
    if (param == nullptr)
      throw std::runtime_error("No such degree of freedom: " + key);
    
    return param->value;
  }

  void
  __setitem__(std::string const &key, double value)
  {
    if (!self->setDof(key, value))
      throw std::runtime_error(
        "Value " 
        + std::to_string(value) 
        + " rejected for degree of freedom `" + key + "'");
  }
}

%extend RZ::Detector {
  PyObject *
  image() {
    unsigned int imgWidth  = self->cols();
    unsigned int imgHeight = self->rows();
    unsigned int imgStride = self->stride();
    const uint32_t *data   = self->data();

    unsigned int i, j;
        
    npy_intp dims[]    = {imgHeight, imgWidth};
    npy_intp strides[] = {imgStride * 4, 4};
    PyObject *outArray = PyArray_New(
      &PyArray_Type,
      2,
      dims,
      NPY_UINT32,
      strides,
      const_cast<uint32_t *>(data),
      0,
      NPY_ARRAY_CARRAY,
      nullptr);

    return outArray;
  }
}

%extend RZ::RayList {
  void
  addSkyBeam(
        unsigned int number = 1000,
        Real radius = .5,
        Real azimuth = 0,
        Real elevation = 90,
        Real distance = 10)
  {
    OMModel::addSkyBeam(*self, number, radius, azimuth, elevation, distance);
  }

  void
  addElementRelativeBeam(
        Element *element,
        unsigned int number = 1000,
        Real radius = .5,
        Real azimuth = 0,
        Real elevation = 90,
        Real offX = 0,
        Real offY = 0,
        Real distance = 10)
  {
    OMModel::addElementRelativeBeam(
      *self,
      element,
      number,
      radius,
      azimuth,
      elevation,
      offX,
      offY,
      distance);
  }

  void
  addElementRelativeFocusBeam(
        Element *element,
        unsigned int number = 1000,
        Real radius = .5,
        Real fNum = 17.37,
        Real refAperture = 200e-3,
        Real azimuth = 0,
        Real elevation = 90,
        Real offX = 0,
        Real offY = 0,
        Real distance = 10)
  {
    OMModel::addElementRelativeFocusBeam(
      *self,
      element,
      number,
      radius,
      fNum,
      refAperture,
      azimuth,
      elevation,
      offX,
      offY,
      distance);
  }

  void
  addFocalPlaneFocusedBeam(
    const ReferenceFrame *focalPlane,
    unsigned int number = 1000,
    Real fNum = 17.37,
    Real azimuth = 0,
    Real elevation = 90,
    Real offX = 0,
    Real offY = 0,
    Real distance = 10)
  {
    OMModel::addFocalPlaneFocusedBeam(
      *self,
      focalPlane,
      number,
      fNum,
      azimuth,
      elevation,
      offX,
      offY,
      distance);
  }
}