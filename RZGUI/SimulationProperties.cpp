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

#include "SimulationProperties.h"
#include <QJsonParseError>

void
SimulationBeamProperties::loadDefaults()
{
  *this = SimulationBeamProperties();
}

void
SimulationProperties::loadDefaults()
{
  *this = SimulationProperties(); // Haha C++
}


bool
SimulationBeamProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    BeamType &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      setLastError("Invalid value for property `" + key + "' (not a string)");
      return false;
    }

    auto asString = obj[key].toString();

    if (asString == "COLLIMATED")
      value = BEAM_TYPE_COLLIMATED;
    else if (asString == "CONVERGING")
      value = BEAM_TYPE_CONVERGING;
    else if (asString == "DIVERGING")
      value = BEAM_TYPE_DIVERGING;
    else {
      setLastError("Unknown beam type `" + asString + "'");
      return false;
    }
  }

  return true;
}

bool
SimulationBeamProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    RZ::BeamShape &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      setLastError("Invalid value for property `" + key + "' (not a string)");
      return false;
    }

    auto asString = obj[key].toString();

    if (asString == "CIRCULAR")
      value = RZ::Circular;
    else if (asString == "RING")
      value = RZ::Ring;
    else if (asString == "POINT")
      value = RZ::Point;
    else if (asString == "CUSTOM")
      value = RZ::Custom;
    else {
      setLastError("Unknown beam type `" + asString + "'");
      return false;
    }
  }

  return true;
}

bool
SimulationBeamProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    BeamReference &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      setLastError("Invalid value for property `" + key + "' (not a string)");
      return false;
    }

    auto asString = obj[key].toString();

    if (asString == "INPUT_ELEMENT")
      value = BEAM_REFERENCE_INPUT_ELEMENT;
    else if (asString == "FOCAL_PLANE")
      value = BEAM_REFERENCE_FOCAL_PLANE;
    else if (asString == "APERTURE_STOP")
      value = BEAM_REFERENCE_APERTURE_STOP;
    else {
      setLastError("Unknown beam reference `" + asString + "'");
      return false;
    }
  }

  return true;
}

QJsonObject
SimulationBeamProperties::serialize() const
{
  QJsonObject object;

  switch (beam) {
    case BEAM_TYPE_COLLIMATED:
      object["beam"] = "COLLIMATED";
      break;

    case BEAM_TYPE_CONVERGING:
      object["beam"] = "CONVERGING";
      break;

    case BEAM_TYPE_DIVERGING:
      object["beam"] = "DIVERGING";
      break;
  }

  switch (shape) {
    case RZ::Circular:
      object["shape"] = "CIRCULAR";
      break;

    case RZ::Ring:
      object["shape"] = "RING";
      break;

    case RZ::Point:
      object["shape"] = "POINT";
      break;

    case RZ::Custom:
      object["shape"] = "CUSTOM";
      break;
  }

  switch (ref) {
    case BEAM_REFERENCE_INPUT_ELEMENT:
      object["ref"] = "INPUT_ELEMENT";
      break;

    case BEAM_REFERENCE_FOCAL_PLANE:
      object["ref"] = "FOCAL_PLANE";
      break;

    case BEAM_REFERENCE_APERTURE_STOP:
      object["ref"] = "APERTURE_STOP";
      break;
  }

  object["color"] = color.name();

#define SERIALIZE(what) object[#what] = what
  SERIALIZE(name);
  SERIALIZE(path);

  SERIALIZE(diameter);
  SERIALIZE(focalPlane);
  SERIALIZE(apertureStop);
  SERIALIZE(fNum);
  SERIALIZE(uX);
  SERIALIZE(uY);
  SERIALIZE(offsetX);
  SERIALIZE(offsetY);
  SERIALIZE(offsetZ);
  SERIALIZE(wavelength);
  SERIALIZE(random);
  SERIALIZE(rays);
#undef SERIALIZE

  return object;
}

bool
SimulationBeamProperties::deserialize(QJsonObject const &obj)
{
#define DESERIALIZE(field)                \
  if (!deserialize(obj, #field, field))   \
    return false

  DESERIALIZE(name);
  DESERIALIZE(color);
  DESERIALIZE(beam);
  DESERIALIZE(shape);
  DESERIALIZE(ref);
  DESERIALIZE(path);

  DESERIALIZE(diameter);
  DESERIALIZE(focalPlane);
  DESERIALIZE(apertureStop);
  DESERIALIZE(fNum);
  DESERIALIZE(uX);
  DESERIALIZE(uY);
  DESERIALIZE(offsetX);
  DESERIALIZE(offsetY);
  DESERIALIZE(offsetZ);
  DESERIALIZE(wavelength);
  DESERIALIZE(random);
  DESERIALIZE(rays);
#undef DESERIALIZE

  return true;
}

/////////////////////////////////// SimulationProperties ///////////////////////
void
SimulationProperties::addBeam(SimulationBeamProperties const &prop)
{
  beams.push_back(prop);

  auto last = beams.end();
  --last;

  last->index = static_cast<int>(beamVector.size());

  beamVector.push_back(&*last);
}

QJsonObject
SimulationProperties::serialize() const
{
  QJsonObject object;
  QJsonObject dofObj;

  switch (ttype) {
    case TRACER_TYPE_GEOMETRIC_OPTICS:
      object["ttype"] = "GEOMETRIC_OPTICS";
      break;

    case TRACER_TYPE_DIFFRACTION:
      object["ttype"] = "DIFFRACTION";
      break;
  }

  switch (type) {
    case SIM_TYPE_ONE_SHOT:
      object["type"] = "ONE_SHOT";
      break;

    case SIM_TYPE_1D_SWEEP:
      object["type"] = "1D_SWEEP";
      break;

    case SIM_TYPE_2D_SWEEP:
      object["type"] = "2D_SWEEP";
      break;
  }


#define SERIALIZE(what) object[#what] = what
  SERIALIZE(Ni);
  SERIALIZE(Nj);
  SERIALIZE(detector);
  SERIALIZE(path);
  SERIALIZE(saveArtifacts);
  SERIALIZE(clearDetector);
  SERIALIZE(overwrite);
  SERIALIZE(saveDir);
  SERIALIZE(saveDetector);
#undef SERIALIZE

  QJsonArray footprintArray;
  for (auto &p : footprints)
    footprintArray.push_back(QString::fromStdString(p));
  object["footprints"] = footprintArray;

  QJsonArray beamArray;
  for (auto &p : beams)
    beamArray.push_back(p.serialize());
  object["beams"] = beamArray;

  for (auto p : dofs)
    dofObj[QString::fromStdString(p.first)] = QString::fromStdString(p.second);

  object["dofs"] = dofObj;

  return object;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    TracerType &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      setLastError("Invalid value for property `" + key + "' (not a string)");
      return false;
    }

    auto asString = obj[key].toString();

    if (asString == "GEOMETRIC_OPTICS")
      value = TRACER_TYPE_GEOMETRIC_OPTICS;
    else if (asString == "DIFFRACTION")
      value = TRACER_TYPE_DIFFRACTION;
    else {
      setLastError("Unknown tracer type `" + asString + "'");
      return false;
    }
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    SimulationType &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      setLastError("Invalid value for property `" + key + "' (not a string)");
      return false;
    }

    auto asString = obj[key].toString();

    if (asString == "ONE_SHOT")
      value = SIM_TYPE_ONE_SHOT;
    else if (asString == "1D_SWEEP")
      value = SIM_TYPE_1D_SWEEP;
    else if (asString == "2D_SWEEP")
      value = SIM_TYPE_2D_SWEEP;
    else {
      setLastError("Unknown simulation type `" + asString + "'");
      return false;
    }
  }

  return true;
}


bool
SimulationProperties::deserialize(QJsonObject const &obj)
{
#define DESERIALIZE(field)                \
  if (!deserialize(obj, #field, field))   \
    return false

  DESERIALIZE(ttype);
  DESERIALIZE(type);

  DESERIALIZE(beams);
  DESERIALIZE(Ni);
  DESERIALIZE(Nj);
  DESERIALIZE(detector);
  DESERIALIZE(path);
  DESERIALIZE(dofs);
  DESERIALIZE(footprints);

  DESERIALIZE(saveArtifacts);
  DESERIALIZE(clearDetector);
  DESERIALIZE(overwrite);
  DESERIALIZE(saveDir);
  DESERIALIZE(saveDetector);

#undef DESERIALIZE

  return true;
}

