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

#ifndef SIMULATIONPROPERTIES_H
#define SIMULATIONPROPERTIES_H

#include "JsonSerializable.h"
#include <OMModel.h>
#include <vector>

enum TracerType {
  TRACER_TYPE_GEOMETRIC_OPTICS,
  TRACER_TYPE_DIFFRACTION
};

enum SimulationType {
  SIM_TYPE_ONE_SHOT,
  SIM_TYPE_1D_SWEEP,
  SIM_TYPE_2D_SWEEP
};

enum BeamType {
  BEAM_TYPE_COLLIMATED,
  BEAM_TYPE_CONVERGING,
  BEAM_TYPE_DIVERGING
};

enum BeamReference {
  BEAM_REFERENCE_INPUT_ELEMENT,
  BEAM_REFERENCE_APERTURE_STOP,
  BEAM_REFERENCE_FOCAL_PLANE
};

////////////////////////// SimulationBeamProperties ////////////////////////////
struct SimulationBeamProperties : public JsonSerializable {
  using JsonSerializable::deserialize;

  QString        name;
  QColor         color;
  BeamType       beam            = BEAM_TYPE_COLLIMATED;
  BeamReference  ref             = BEAM_REFERENCE_INPUT_ELEMENT;
  RZ::BeamShape  shape           = RZ::BeamShape::Circular;
  RZ::SkyObjectShape objectShape = RZ::SkyObjectShape::PointLike;

  int     rays         = 1000;
  QString path         = "";
  QString diameter     = "40e-3";   // m
  QString span         = "0";       // deg
  QString focalPlane   = "";
  QString apertureStop = "";
  QString fNum         = "17.37";
  QString uX           = "0";       // (cos)
  QString uY           = "0";       // (cos)
  QString offsetX      = "0";       // m
  QString offsetY      = "0";       // m
  QString offsetZ      = "0";       // m
  QString wavelength   = "525";     // nm
  bool    colorByWl    = false;
  bool    random       = false; // Random sampling

  int     index        = -1;

  virtual QJsonObject serialize() const override;
  virtual bool deserialize(QJsonObject const &) override;
  virtual void loadDefaults() override;

  SimulationBeamProperties() = default;
  SimulationBeamProperties(const SimulationBeamProperties &) = default;
  SimulationBeamProperties(SimulationBeamProperties &&) = default;
  SimulationBeamProperties& operator=(const SimulationBeamProperties &) = default;
  SimulationBeamProperties& operator=(SimulationBeamProperties &&) = default;

protected:
  bool deserialize(QJsonObject const &, QString const &, BeamType &);
  bool deserialize(QJsonObject const &, QString const &, BeamReference &);
  bool deserialize(QJsonObject const &, QString const &, RZ::BeamShape &);
  bool deserialize(QJsonObject const &, QString const &, RZ::SkyObjectShape &);
};

////////////////////////// SimulationProperties ////////////////////////////////
struct SimulationProperties : public JsonSerializable {
  using JsonSerializable::deserialize;

  TracerType     ttype = TRACER_TYPE_GEOMETRIC_OPTICS;
  SimulationType type  = SIM_TYPE_ONE_SHOT;
  int  Ni              = 10;
  int  Nj              = 10;

  std::list<SimulationBeamProperties>      beams;
  std::vector<SimulationBeamProperties *>  beamVector;

  std::list<std::string> footprints;

  QString detector;
  QString path;

  std::map<std::string, std::string> dofs;

  // Artifact generation properties
  bool    saveArtifacts = false;
  bool    saveCSV       = true;
  bool    clearDetector = false;
  bool    overwrite     = false;

  QString saveDir       = "artifacts";
  QString saveDetector  = "";

  void addBeam(SimulationBeamProperties const &prop);
  bool removeBeam(int index);
  void clearBeams();

  virtual QJsonObject serialize() const override;
  virtual bool deserialize(QJsonObject const &) override;
  virtual void loadDefaults() override;

  SimulationProperties() = default;
  SimulationProperties(const SimulationProperties &);
  SimulationProperties(SimulationProperties &&);
  SimulationProperties& operator=(const SimulationProperties &);
  SimulationProperties& operator=(SimulationProperties &&);

private:
  bool deserialize(QJsonObject const &, QString const &, TracerType &);
  bool deserialize(QJsonObject const &, QString const &, SimulationType &);

  void regenerateBeamVector();
};

#endif // SIMULATIONPROPERTIES_H
