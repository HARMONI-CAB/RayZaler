%module pyRayZaler
%{
//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//


#define PYTHON_SCRIPT_SUPPORT
#define SWIG_FILE_WITH_INIT
static PyObject* g_rzException;
%}

%include "stdint.i"
%include "std_vector.i"
%include "std_list.i"
%include "std_set.i"
%include "std_map.i"
%include "std_string.i"
%include "std_pair.i"
%include "numpy.i"

namespace std {
  %template(StringList)  list<string>;
  %template(StringVec)   vector<string>;
  %template(StringSet)   set<string>;
}

%init %{
  RZ::Logger::setDefaultLogger(new RZ::StdErrLogger());
  import_array();
  g_rzException = PyErr_NewException("_pyRayZaler.EngineError", nullptr, nullptr);
  Py_INCREF(g_rzException);
  PyModule_AddObject(m, "EngineError", g_rzException);
%}

%apply (double IN_ARRAY1[ANY]) {(RZ::Real vec3Coef[3]), (const RZ::Real coords[3])};
%apply (double IN_ARRAY2[ANY][ANY]) {(const RZ::Real coef[3][3])};

%{
#define RZ_NO_HELPER_MACROS

#include <Vector.h>
#include <Matrix.h>
#include <Linalg.h>
#include <GenericCompositeModel.h>
#include <CompositeElement.h>
#include <RayTracingEngine.h>
#include <OMModel.h>
#include <TopLevelModel.h>
#include <Logger.h>
#include <ExprTkEvaluator.h>
#include <GLModel.h>
#include <GLRenderEngine.h>
#include <ModelRenderer.h>
#include <ParserContext.h>
#include <RayBeam.h>
#include <Recipe.h>
#include <RotatedFrame.h>
#include <Singleton.h>
#include <SkySampler.h>
#include <SurfaceShape.h>
#include <TranslatedFrame.h>
#include <TripodFrame.h>
#include <WorldFrame.h>
#include <OpticalElement.h>
#include <Elements/All.h>
#include <RayTracingHeuristics/All.h>
#include <EMInterfaces/All.h>
#include <Simulation.h>

using namespace RZ;
PyMODINIT_FUNC PyInit_RZ();

%}

namespace std {
  %template(PropertyMap)      map<string, RZ::PropertyValue>;
  %template(OpticSurfList)    list<RZ::OpticalSurface *>;
  %rename(ConstOpticSurfList) list<const RZ::OpticalSurface *>;
  %template(PureRayList)      list<RZ::Ray, allocator<RZ::Ray>>;
  %template(PureRayVec)       vector<RZ::Ray, allocator<RZ::Ray>>;
}

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
%include "EMInterface.h"
%include "Element.h"
%include "OpticalElement.h"
%include "GenericCompositeModel.h"
%include "OMModel.h"
%include "TopLevelModel.h"
%include "Logger.h"
%include "SurfaceShape.h"

%include "CompositeElement.h"
%include "ExprTkEvaluator.h"
%include "GLRenderEngine.h"
%include "GLModel.h"
%include "MediumBoundary.h"
%include "ModelRenderer.h"
%include "ParserContext.h"
%include "RayBeam.h"
%include "RayTracingEngine.h"

%include "Recipe.h"
%include "RotatedFrame.h"
%include "Simulation.h"
%include "Singleton.h"
%include "SkySampler.h"
%include "TranslatedFrame.h"
%include "TripodFrame.h"
%include "WorldFrame.h"

%include "Elements/ApertureStop.h"
%include "Elements/BenchElement.h"
%include "Elements/BlockElement.h"
%include "Elements/CircularWindow.h"
%include "Elements/ConicLens.h"
%include "Elements/ConicMirror.h"
%include "Elements/Detector.h"
%include "Elements/FlatMirror.h"
%include "Elements/IdealLens.h"
%include "Elements/LensletArray.h"
%include "Elements/Obstruction.h"
%include "Elements/ParabolicLens.h"
%include "Elements/ParabolicMirror.h"
%include "Elements/PhaseScreen.h"
%include "Elements/RayBeamElement.h"
%include "Elements/RectangularStop.h"
%include "Elements/RodElement.h"
%include "Elements/SphericalLens.h"
%include "Elements/SphericalMirror.h"
%include "Elements/StlMesh.h"
%include "Elements/Tripod.h"
%include "Elements/TubeElement.h"

%include "RayTracingHeuristics/Dummy.h"

%include "EMInterfaces/DielectricEMInterface.h"
%include "EMInterfaces/DummyEMInterface.h"
%include "EMInterfaces/ParaxialEMInterface.h"
%include "EMInterfaces/ParaxialZernikeEMInterface.h"
%include "EMInterfaces/ReflectiveEMInterface.h"

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

%extend RZ::Element {
  PyObject *
  __getitem__(std::string const &key)
  {
    RZ::PropertyValue value = self->get(key);

    switch (value.type()) {
      case IntegerValue:
        return PyLong_FromLong(std::get<int64_t>(value));
      
      case RealValue:
        return PyFloat_FromDouble(std::get<RZ::Real>(value));

      case BooleanValue:
        return PyBool_FromLong(std::get<bool>(value));

      case StringValue:
        return PyUnicode_FromString(std::get<std::string>(value).c_str());

      default:
        throw std::runtime_error("Element `" + self->name() + "' has no property `" + key + "'");  
    }
  }
}

%extend RZ::OpticalElement {
  PyObject *
  hitArray(std::string const &name = "") const
  {
    unsigned int cols = 3;
    unsigned int rows = self->hits(name).size() / 3;
    const Real *data  = self->hits(name).data();

    unsigned int i, j;
        
    npy_intp dims[]    = {rows, cols};
    npy_intp strides[] = {
      static_cast<npy_intp>(cols * sizeof(Real)),
      static_cast<npy_intp>(sizeof(Real))};
    PyObject *outArray = PyArray_New(
      &PyArray_Type,
      2,
      dims,
      NPY_DOUBLE,
      strides,
      const_cast<Real *>(data),
      0,
      NPY_ARRAY_CARRAY,
      nullptr);

    return outArray;
  }

  PyObject *
  dirArray(std::string const &name = "") const
  {
    unsigned int cols = 3;
    unsigned int rows = self->directions(name).size() / 3;
    const Real *data  = self->directions(name).data();

    unsigned int i, j;
        
    npy_intp dims[]    = {rows, cols};
    npy_intp strides[] = {
      static_cast<npy_intp>(cols * sizeof(Real)),
      static_cast<npy_intp>(sizeof(Real))};
    PyObject *outArray = PyArray_New(
      &PyArray_Type,
      2,
      dims,
      NPY_DOUBLE,
      strides,
      const_cast<Real *>(data),
      0,
      NPY_ARRAY_CARRAY,
      nullptr);

    return outArray;
  }
}

%extend RZ::Matrix3 {
  PyObject *
  array() {
    RZ::Real *data = &self->coef[0][0];

    unsigned int i, j;
        
    npy_intp dims[]    = {3, 3};
    npy_intp strides[] = {sizeof(RZ::Real) * 3, sizeof(RZ::Real)};
    PyObject *outArray = PyArray_New(
      &PyArray_Type,
      2,
      dims,
      NPY_DOUBLE,
      strides,
      data,
      0,
      NPY_ARRAY_CARRAY,
      nullptr);

    return outArray;
  }
}

%extend RZ::Vec3 {
  PyObject *
  array()
  {
    RZ::Real *data = &self->coords[0];

    unsigned int i, j;
        
    npy_intp dims[]    = {3};
    npy_intp strides[] = {sizeof(RZ::Real)};
    PyObject *outArray = PyArray_New(
      &PyArray_Type,
      1,
      dims,
      NPY_DOUBLE,
      strides,
      data,
      0,
      NPY_ARRAY_CARRAY,
      nullptr);

    return outArray;
  }

  Vec3 &
  fromArray(const double *vec3Coef)
  {
    self->coords[0] = vec3Coef[0];
    self->coords[1] = vec3Coef[1];
    self->coords[2] = vec3Coef[2];

    return *self;
  }

  Real
  x()
  {
    return self->x;
  }

  Real
  y()
  {
    return self->y;
  }

  Real
  z()
  {
    return self->z;
  }
}

%extend RZ::ModelRenderer {
  PyObject *
  image()
  {
    unsigned int imgWidth  = self->width();
    unsigned int imgHeight = self->height();
    const uint32_t *data   = self->pixels();

    unsigned int i, j;
        
    npy_intp dims[]    = {imgHeight, imgWidth, 4};
    npy_intp strides[] = {imgWidth * 4, 4, 1};
    PyObject *outArray = PyArray_New(
      &PyArray_Type,
      3,
      dims,
      NPY_UINT8,
      strides,
      const_cast<uint32_t *>(data),
      0,
      NPY_ARRAY_CARRAY,
      nullptr);

    return outArray;
  }
}

%extend RZ::Detector {
  PyObject *
  image()
  {
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
  uint64_t
  size() const
  {
    return self->size();
  }

  RZ::Ray *
  getRay(uint64_t i)
  {
    if (i >= self->size())
      return nullptr;

    for (auto &p : *self)
      if (i-- == 0)
        return &p;
    
    return nullptr;
  }

  void
  addSkyBeam(
        unsigned int number = 1000,
        Real radius = .5,
        Real azimuth = 0,
        Real elevation = 90,
        Real distance = 10,
        uint32_t id = 0,
        bool random = true)
  {
    OMModel::addSkyBeam(
      *self,
      number,
      radius,
      azimuth,
      elevation,
      distance,
      id,
      random);
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
        Real distance = 10,
        uint32_t id = 0,
        bool random = true)
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
      distance,
      id,
      random);
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
        Real distance = 10,
        uint32_t id = 0,
        bool random = true)
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
      distance,
      id,
      random);
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
    Real distance = 10,
    uint32_t id = 0,
    bool random = true,
    Real offZ = 0)
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
      distance,
      id,
      random,
      offZ);
  }

  void
  addBeam(BeamProperties const &properties)
  {
    OMModel::addBeam(*self, properties);
  }
}
