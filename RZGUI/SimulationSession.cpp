#include "SimulationSession.h"
#include <ParserContext.h>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <QTimer>
#include <QFileInfo>
#include <RayBeamElement.h>
#include <QThread>
#include <QDir>
#include "AsyncRayTracer.h"
#include <DataProducts/Scatter.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <sys/stat.h>
#include <Logger.h>
#include <cmath>

class RGBRayColoring : public RZ::RayColoring {
  virtual void id2color(uint32_t id, GLfloat *rgb) const override;
};

void
RGBRayColoring::id2color(uint32_t id, GLfloat *rgb) const
{
  rgb[0] = ((id >> 16) & 0xff) / 255.;
  rgb[1] = ((id >> 8)  & 0xff) / 255.;
  rgb[2] = ((id >> 0)  & 0xff) / 255.;
}

static RGBRayColoring g_rgbColoring;

void
SimulationProperties::loadDefaults()
{
  *this = SimulationProperties(); // Haha C++
}

QByteArray
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

#define SERIALIZE(what) object[#what] = what
  SERIALIZE(diameter);
  SERIALIZE(refAperture);
  SERIALIZE(focalPlane);
  SERIALIZE(fNum);
  SERIALIZE(azimuth);
  SERIALIZE(elevation);
  SERIALIZE(offsetX);
  SERIALIZE(offsetY);
  SERIALIZE(wavelength);
  SERIALIZE(random);
  SERIALIZE(rays);
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

  for (auto p : dofs)
    dofObj[QString::fromStdString(p.first)] = QString::fromStdString(p.second);

  object["dofs"] = dofObj;

  return QJsonDocument(object).toJson();
}

QString
SimulationProperties::lastError() const
{
  return m_lastError;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    std::list<std::string> &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isArray()) {
      m_lastError = "Invalid value for property `" + key + "' (not an array)";
      return false;
    }

    value.clear();
    for (auto el : obj[key].toArray())
      value.push_back(el.toString().toStdString());
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    QString &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
      return false;
    }

    value = obj[key].toString();
  }

  return true;
}


bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    TracerType &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
      return false;
    }

    auto asString = obj[key].toString();

    if (asString == "GEOMETRIC_OPTICS")
      value = TRACER_TYPE_GEOMETRIC_OPTICS;
    else if (asString == "DIFFRACTION")
      value = TRACER_TYPE_DIFFRACTION;
    else {
      m_lastError = "Unknown tracer type `" + asString + "'";
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
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
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
      m_lastError = "Unknown simulation type `" + asString + "'";
      return false;
    }
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    BeamType &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
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
      m_lastError = "Unknown beam type `" + asString + "'";
      return false;
    }
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    RZ::BeamShape &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
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
      m_lastError = "Unknown beam type `" + asString + "'";
      return false;
    }
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    BeamReference &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isString()) {
      m_lastError = "Invalid value for property `" + key + "' (not a string)";
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
      m_lastError = "Unknown beam reference `" + asString + "'";
      return false;
    }
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    int &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isDouble()) {
      m_lastError = "Invalid value for property `" + key + "' (not a number)";
      return false;
    }

    value = obj[key].toInt();
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    qreal &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isDouble()) {
      m_lastError = "Invalid value for property `" + key + "' (not a number)";
      return false;
    }

    value = obj[key].toDouble();
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    bool &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isBool()) {
      m_lastError = "Invalid value for property `" + key + "' (not a boolean)";
      return false;
    }

    value = obj[key].toBool(value);
  }

  return true;
}

bool
SimulationProperties::deserialize(
    QJsonObject const &obj,
    QString const &key,
    std::map<std::string, std::string> &value)
{
  if (obj.contains(key)) {
    if (!obj[key].isObject()) {
      m_lastError = "Invalid value for property `" + key + "' (not a JSON object)";
      return false;
    }

    value.clear();
    auto asObject = obj[key].toObject();
    for (auto p : asObject.keys()) {
      if (!asObject[p].isString()) {
        m_lastError = "Invalid entry `" + p + "` for dictionary `" + key + "' (not a string)";
        return false;
      }

      value[p.toStdString()] = asObject[p].toString().toStdString();
    }
  }

  return true;
}

bool
SimulationProperties::deserialize(QByteArray const &json)
{
  QJsonDocument doc;
  QJsonObject obj;
  QJsonParseError errors;

  doc = QJsonDocument::fromJson(json, &errors);

  if (doc.isNull()) {
    m_lastError = errors.errorString();
    return false;
  }

  obj = doc.object();

#define DESERIALIZE(field)                \
  if (!deserialize(obj, #field, field))   \
    return false

  DESERIALIZE(ttype);
  DESERIALIZE(type);
  DESERIALIZE(beam);
  DESERIALIZE(shape);
  DESERIALIZE(ref);
  DESERIALIZE(diameter);
  DESERIALIZE(refAperture);
  DESERIALIZE(focalPlane);
  DESERIALIZE(apertureStop);
  DESERIALIZE(fNum);
  DESERIALIZE(azimuth);
  DESERIALIZE(elevation);
  DESERIALIZE(offsetX);
  DESERIALIZE(offsetY);
  DESERIALIZE(wavelength);
  DESERIALIZE(random);
  DESERIALIZE(rays);
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

void
SimulationState::clearAll()
{

  if (m_diamExpr != nullptr) {
    delete m_diamExpr;
    m_diamExpr = nullptr;
  }

  if (m_fNumExpr != nullptr) {
    delete m_fNumExpr;
    m_fNumExpr = nullptr;
  }

  if (m_refApExpr != nullptr) {
    delete m_refApExpr;
    m_refApExpr = nullptr;
  }
  if (m_azimuthExpr != nullptr) {
    delete m_azimuthExpr;
    m_azimuthExpr = nullptr;
  }

  if (m_elevationExpr != nullptr) {
    delete m_elevationExpr;
    m_elevationExpr = nullptr;
  }

  if (m_offsetXExpr != nullptr) {
    delete m_offsetXExpr;
    m_offsetXExpr = nullptr;
  }

  if (m_offsetYExpr != nullptr) {
    delete m_offsetYExpr;
    m_offsetYExpr = nullptr;
  }

  if (m_wavelengthExpr != nullptr) {
    delete m_wavelengthExpr;
    m_wavelengthExpr = nullptr;
  }

  for (auto p : m_dofExprs)
    if (p.second != nullptr)
      delete p.second;

  for (auto p : m_dictionary)
    if (p.second != nullptr)
      delete p.second;

  m_dictionary.clear();
  m_varDescriptions.clear();
  m_dofExprs.clear();
  m_complete = false;
}

bool
SimulationState::trySetExpr(
    RZ::ExprTkEvaluator * &dest,
    std::string const &expr)
{
  RZ::ExprTkEvaluator *evaluator =
      new RZ::ExprTkEvaluator(&m_dictionary);
  bool ok = true;

  if (!evaluator->compile(expr)) {
    m_lastCompileError = evaluator->getLastParserError();
    delete evaluator;
    evaluator = nullptr;
    ok = false;
  }

  dest = evaluator;

  return ok;
}

std::string
SimulationState::getFirstInvalidExpr() const
{
  if (m_diamExpr == nullptr)
    return "diameter";

  if (m_fNumExpr == nullptr)
    return "fnum";

  if (m_refApExpr == nullptr)
    return "refap";

  if (m_azimuthExpr == nullptr)
    return "azimuth";

  if (m_elevationExpr == nullptr)
    return "elevation";

  if (m_offsetXExpr == nullptr)
    return "offsetx";

  if (m_offsetYExpr == nullptr)
    return "offsety";

  if (m_wavelengthExpr == nullptr)
    return "wavelength";

  for (auto p : m_dofExprs)
    if (p.second == nullptr)
      return "dof:" + p.first;

  return "";
}

std::string
SimulationState::getLastError() const
{
  return m_lastCompileError;
}

bool
SimulationState::running() const
{
  return m_running;
}

bool
SimulationState::canRun() const
{
  return m_complete;
}

void
SimulationState::saveArtifacts()
{
  if (m_properties.saveArtifacts) {
    QString path = getCurrentOutputFileName();
    if (!m_properties.overwrite) {
      while (QFile(getCurrentOutputFileName()).exists())
        bumpPrefix();

      path = getCurrentOutputFileName();
    }

    std::string fileName = path.toStdString();

    if (m_properties.saveCSV)
      saveCSV();

    if (m_properties.clearDetector) {  
      RZInfo("Saving detector state to %s\n", fileName.c_str());
      m_saveDetector->savePNG(fileName);
      m_saveDetector->clear();
    } else if (done()) {
      RZInfo("Saving final state to %s\n", fileName.c_str());
      m_saveDetector->savePNG(fileName);
    }
  }
}

void
SimulationState::defineVariable(
    std::string const &name,
    RZ::Real value,
    RZ::Real min,
    RZ::Real max)
{
  m_varDescriptions[name].min        = min;
  m_varDescriptions[name].max        = max;
  m_varDescriptions[name].defaultVal = value;

  if (m_dictionary.find(name) == m_dictionary.end())
    m_dictionary[name] = new RZ::GenericModelParam();

  m_dictionary[name]->description    = &m_varDescriptions[name];
  m_dictionary[name]->value          = value;
}

RZ::Real
SimulationState::setVariable(std::string const &name, RZ::Real value)
{
  if (m_dictionary.find(name) == m_dictionary.end()) {
    fprintf(stderr, "Ayee! Variable %s not in dict\n", name.c_str());
    abort();
  }

  m_dictionary[name]->value = value;

  return value;
}

void
SimulationState::setRepresentationProperties(RepresentationProperties const &repProp)
{
  m_repProp = repProp;
}

bool
SimulationState::setProperties(SimulationProperties const &prop)
{
  m_properties = prop;

  // Sanitize this
  if (prop.type == SIM_TYPE_ONE_SHOT)
    m_properties.Ni = m_properties.Nj = 1;
  else if (prop.type == SIM_TYPE_1D_SWEEP)
    m_properties.Nj = 1;

  clearAll();

  // Recreate dictionary
  defineVariable("i");
  defineVariable("j");
  defineVariable("Ni");
  defineVariable("Nj");
  defineVariable("wavelength");
  defineVariable("D");
  defineVariable("fNum");
  defineVariable("A");
  defineVariable("az");
  defineVariable("el");
  defineVariable("x0");
  defineVariable("y0");
  defineVariable("simU");
  defineVariable("simN");
  defineVariable("stepU");
  defineVariable("stepN");
  defineVariable("step");
  defineVariable("sim");

  for (auto p : prop.dofs) {
    m_dofExprs[p.first]  = nullptr;
    defineVariable("dof_" + p.first);
  }

  if (!trySetExpr(m_diamExpr, prop.diameter.toStdString()))
    return false;

  if (!trySetExpr(m_fNumExpr, prop.fNum.toStdString()))
    return false;

  if (!trySetExpr(m_refApExpr, prop.refAperture.toStdString()))
    return false;

  if (!trySetExpr(m_azimuthExpr, prop.azimuth.toStdString()))
    return false;

  if (!trySetExpr(m_elevationExpr, prop.elevation.toStdString()))
    return false;

  if (!trySetExpr(m_offsetXExpr, prop.offsetX.toStdString()))
    return false;

  if (!trySetExpr(m_offsetYExpr, prop.offsetY.toStdString()))
    return false;

  if (!trySetExpr(m_wavelengthExpr, prop.wavelength.toStdString()))
    return false;

  for (auto p : prop.dofs)
    if (!trySetExpr(m_dofExprs[p.first], p.second))
        return false;

  m_complete = true;

  return true;
}

bool
SimulationState::allocateRays(uint32_t color)
{
  const RZ::OpticalPath *path = nullptr;
  RZ::OpticalElement *element = nullptr;
  RZ::ReferenceFrame *fp = nullptr;
  RZ::BeamProperties prop;
  bool random = m_properties.random;

  // TODO: prevent continuous reallocation of beams
  if (m_currBeam == m_beamAlloc.end())
    m_currBeam = m_beamAlloc.begin();
  else
    ++m_currBeam;

  if (m_currBeam == m_beamAlloc.end()) {
    // End of the list, alloc new beam
    m_beamAlloc.push_back(std::list<RZ::Ray>());
    m_currBeam = m_beamAlloc.end();
    --m_currBeam;
  }

  m_currBeam->clear();

  switch (m_properties.ref) {
    case BEAM_REFERENCE_INPUT_ELEMENT:
      path = m_topLevelModel->lookupOpticalPath(m_properties.path.toStdString());
      if (path == nullptr) {
        m_lastCompileError = "The defined optical path does not exist";
        return false;
      }

      if (path->m_sequence.size() == 0) {
        m_lastCompileError = "Optical path contains no elements";
        return false;
      }

      element = path->m_sequence.front()->parent;

      switch (m_properties.beam) {
        case BEAM_TYPE_COLLIMATED:
          prop.setElementRelative(element);
          prop.shape     = m_properties.shape;
          prop.numRays   = static_cast<unsigned>(m_properties.rays);
          prop.diameter  = setVariable("D", m_diamExpr->evaluate());
          prop.direction = -RZ::Matrix3::azel(
                RZ::deg2rad(setVariable("az", m_azimuthExpr->evaluate())),
                RZ::deg2rad(setVariable("el",   m_elevationExpr->evaluate()))).vz();
          prop.offset    = RZ::Vec3(
                setVariable("x0",   m_offsetXExpr->evaluate()),
                setVariable("y0",   m_offsetYExpr->evaluate()),
                0);
          prop.length    = 1;
          prop.id        = color;
          prop.random    = random;
          prop.collimate();

          break;

        case BEAM_TYPE_CONVERGING:
          prop.setElementRelative(element);
          prop.shape     = m_properties.shape;
          prop.numRays   = static_cast<unsigned>(m_properties.rays);
          prop.diameter  = setVariable("D", m_diamExpr->evaluate());
          prop.direction = -RZ::Matrix3::azel(
                RZ::deg2rad(setVariable("az", m_azimuthExpr->evaluate())),
                RZ::deg2rad(setVariable("el",   m_elevationExpr->evaluate()))).vz();
          prop.offset    = RZ::Vec3(
                setVariable("x0",   m_offsetXExpr->evaluate()),
                setVariable("y0",   m_offsetYExpr->evaluate()),
                0);
          prop.length    = 1;
          prop.id        = color;
          prop.random    = random;
          prop.setFNum(fabs(setVariable("fNum", m_fNumExpr->evaluate())), RZ::BeamDiameter);
          break;

        case BEAM_TYPE_DIVERGING:
          prop.setElementRelative(element);
          prop.shape     = m_properties.shape;
          prop.numRays   = static_cast<unsigned>(m_properties.rays);
          prop.diameter  = setVariable("D", m_diamExpr->evaluate());
          prop.direction = -RZ::Matrix3::azel(
                RZ::deg2rad(setVariable("az", m_azimuthExpr->evaluate())),
                RZ::deg2rad(setVariable("el",   m_elevationExpr->evaluate()))).vz();
          prop.offset    = RZ::Vec3(
                setVariable("x0",   m_offsetXExpr->evaluate()),
                setVariable("y0",   m_offsetYExpr->evaluate()),
                0);
          prop.length    = 1;
          prop.id        = color;
          prop.random    = random;
          prop.setFNum(-fabs(setVariable("fNum", m_fNumExpr->evaluate())), RZ::BeamDiameter);
          break;
      }
      break;

    case BEAM_REFERENCE_APERTURE_STOP:
      switch (m_properties.beam) {
        case BEAM_TYPE_COLLIMATED:
          // TODO
          break;

        case BEAM_TYPE_CONVERGING:
          // TODO
          break;

        case BEAM_TYPE_DIVERGING:
          // TODO
          break;
      }

      m_lastCompileError = "Aperture-defined beams not yet implemented";
      return false;
      // break;

    case BEAM_REFERENCE_FOCAL_PLANE:
      fp = m_topLevelModel->getFocalPlane(m_properties.focalPlane.toStdString());
      if (fp == nullptr) {
        m_lastCompileError = "The specified focal plane `" + m_properties.focalPlane.toStdString() + "' does not exist";
        return false;
      }

      switch (m_properties.beam) {
        case BEAM_TYPE_COLLIMATED:
          // TODO
          m_lastCompileError = "Focal plane-defined collimated beams not yet implemented";
          return false;
          //break;

        case BEAM_TYPE_CONVERGING:
          RZ::OMModel::addFocalPlaneFocusedBeam(
                *m_currBeam,
                fp,
                static_cast<unsigned>(m_properties.rays),
                setVariable("fNum", m_fNumExpr->evaluate()),
                setVariable("az",   m_azimuthExpr->evaluate()),
                setVariable("el",   m_elevationExpr->evaluate()),
                setVariable("x0",   m_offsetXExpr->evaluate()),
                setVariable("y0",   m_offsetYExpr->evaluate()),
                1,
                color,
                random);
          break;

        case BEAM_TYPE_DIVERGING:
          RZ::OMModel::addFocalPlaneFocusedBeam(
                *m_currBeam,
                fp,
                static_cast<unsigned>(m_properties.rays),
                setVariable("fNum", -m_fNumExpr->evaluate()),
                setVariable("az",    m_azimuthExpr->evaluate()),
                setVariable("el",    m_elevationExpr->evaluate()),
                setVariable("x0",    m_offsetXExpr->evaluate()),
                setVariable("y0",    m_offsetYExpr->evaluate()),
                1,
                color,
                random);
          break;
      }
      break;
  }

  RZ::OMModel::addBeam(*m_currBeam, prop);

  return true;
}

void
SimulationState::applyDofs()
{
  m_randState->update();

  for (auto p : m_dofExprs) {
    setVariable("dof_" + p.first, p.second->evaluate());
    m_topLevelModel->setDof(p.first, m_dictionary["dof_" + p.first]->value);
  }

  if (!m_repProp.accumulate)
   m_topLevelModel->clearBeam();
}

void
SimulationState::resetPrefix()
{
  m_pfxCount = 0;
  genPrefix();
}

void
SimulationState::genPrefix()
{
  std::string name = m_saveDetector->name();

  if (name.empty())
    name = "default";

  m_currentSavePrefix =
      QString::asprintf("sim_%03u_", m_pfxCount)
      + QString::fromStdString(name)
      + "_";
}

QString
SimulationState::getCurrentOutputFileName() const
{
  if (m_properties.clearDetector)
    return m_properties.saveDir
        + "/"
        + m_currentSavePrefix
        + QString::asprintf("step_%03u", m_currStep)
        + ".png";
  else
    return m_properties.saveDir
        + "/"
        + m_currentSavePrefix
        + "integrated.png";
}

QString
SimulationState::getCurrentOutputCSVFileName() const
{
  return m_properties.saveDir
        + "/"
        + m_currentSavePrefix
        + "steps.csv";
}


void
SimulationState::bumpPrefix()
{
  while (QFile(getCurrentOutputFileName()).exists()
         || QFile(getCurrentOutputCSVFileName()).exists()) {
    ++m_pfxCount;
    genPrefix();
  }
}

RZ::Detector *
SimulationState::findDetectorForPath(std::string const &name)
{
  RZ::Detector *detector = nullptr;

  if (name.empty()) {
    std::string pathName = m_properties.path.toStdString();
    const RZ::OpticalPath *path = m_topLevelModel->lookupOpticalPath(pathName);

    if (path == nullptr) {
      if (pathName.empty())
        m_lastCompileError = "Model contains no optical paths";
      else
        m_lastCompileError = "No such optical path `" + pathName + "'";
      return nullptr;
    }

    for (auto p : path->m_sequence)
      if (p->parent->factory()->name() == "Detector")
        detector = static_cast<RZ::Detector *>(p->parent);

    if (detector == nullptr) {
      if (pathName.empty())
        m_lastCompileError = "Default optical path has no detectors";
      else
        m_lastCompileError = "Optical path `" + name + "' has no detectors";
    }

  } else {
    detector = m_topLevelModel->lookupDetector(name);
    if (detector == nullptr)
      m_lastCompileError = "Detector `" + name + "' not found";
  }

  return detector;
}

bool
SimulationState::openCSV()
{
  bumpPrefix(); // This is required, to make sure we do not overwrite the CSV

  std::string csvFileName = getCurrentOutputCSVFileName().toStdString();

  closeCSV();

  m_csvFp = fopen(csvFileName.c_str(), "wb");
  if (m_csvFp == nullptr) {
    RZError(
          "fopen(): cannot openCSV file `%s': %s\n",
          csvFileName.c_str(),
          strerror(errno));
    m_lastCompileError =
        "Failed to create CSV file: " +
        std::string(strerror(errno));
    return false;
  }

  std::string headers;

  headers += "step,i,j,";

  for (auto dof : m_properties.dofs)
    headers += "dof_" + dof.first + ",";

  headers += "filename\n";

  if (fwrite(headers.c_str(), headers.size(), 1, m_csvFp) < 1) {
    RZError("fwrite(): failed to write CSV header: %s\n", strerror(errno));
    m_lastCompileError =
        "Failed to create CSV file: " +
        std::string(strerror(errno));
    closeCSV();
    return false;
  }

  RZInfo("Simulation log created on %s\n", csvFileName.c_str());

  return true;
}

template <typename T>
static std::string
toStringPrecision(const T a_value, const int n = 16)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return std::move(out).str();
}

void
SimulationState::saveCSV()
{
  if (m_csvFp == nullptr) {
    RZError("saveCSV() called with no CSV file!\n");
    return;
  }

  std::string line;

  line += std::to_string(m_currStep) + ","
       + std::to_string(m_i) + ","
       + std::to_string(m_j) + ",";

  for (auto dof : m_properties.dofs) {
    auto value = m_dictionary["dof_" + dof.first]->value;
    line += toStringPrecision(value) + ",";
  }

  line += getCurrentOutputFileName().toStdString() + "\n";

  if (fwrite(line.c_str(), line.size(), 1, m_csvFp) < 1)
    RZError(
          "fwrite(): failed to write state to CSV file: %s\n",
          strerror(errno));
}

void
SimulationState::closeCSV()
{
  // Lazy close previous file
  if (m_csvFp != nullptr) {
    fclose(m_csvFp);
    m_csvFp = nullptr;
  }
}

static inline uint32_t
QColor2uint32_t(QColor const &color)
{
  uint32_t tuple = 0;

  tuple |= color.red()   << 16;
  tuple |= color.green() << 8;
  tuple |= color.blue();

  return tuple;
}

// https://www.johndcook.com/wavelength_to_RGB.html
static inline uint32_t
wl2uint32_t(qreal w)
{
  qreal red, green, blue;
  qreal factor, gamma;
  int R, G, B;
  uint32_t tuple = 0;

  if (w >= 380 && w < 440) {
    red   = -(w - 440) / (440 - 380);
    green = 0.0;
    blue  = 1.0;
  } else if (w >= 440 && w < 490) {
    red   = 0.0;
    green = (w - 440) / (490 - 440);
    blue  = 1.0;
  } else if (w >= 490 && w < 510) {
    red   = 0.0;
    green = 1.0;
    blue  = -(w - 510) / (510 - 490);
  } else if (w >= 510 && w < 580) {
    red   = (w - 510) / (580 - 510);
    green = 1.0;
    blue  = 0.0;
  } else if (w >= 580 && w < 645) {
    red   = 1.0;
    green = -(w - 645) / (645 - 580);
    blue  = 0.0;
  } else if (w >= 645 && w < 781) {
    red   = 1.0;
    green = 0.0;
    blue  = 0.0;
  } else {
    red   = 0.0;
    green = 0.0;
    blue  = 0.0;
  }


  // Let the intensity fall off near the vision limits

  if (w >= 380 && w < 420)
      factor = 0.3 + 0.7*(w - 380) / (420 - 380);
  else if (w >= 420 && w < 701)
      factor = 1.0;
  else if (w >= 701 && w < 781)
      factor = 0.3 + 0.7*(780 - w) / (780 - 700);
  else
      factor = 0.0;

  gamma = 0.80;

  R = static_cast<int>(qBound(0., 255 * pow(red   * factor, gamma), 255.));
  G = static_cast<int>(qBound(0., 255 * pow(green * factor, gamma), 255.));
  B = static_cast<int>(qBound(0., 255 * pow(blue  * factor, gamma), 255.));

  tuple |= R << 16;
  tuple |= G << 8;
  tuple |= B;

  return tuple;
}

uint32_t
SimulationState::beamColorCycle() const
{
  uint32_t colors[] =
  {
    0xff0000, 0x00ff00, 0x0000ff,
    0xffff00, 0xff00ff, 0x00ffff,
    0xffffff
  };

  switch (m_repProp.coloringMode) {
    case COLORING_FIXED:
      return QColor2uint32_t(m_repProp.fixedBeamColor);

    case COLORING_WAVELENGTH:
      return wl2uint32_t(m_topLevelModel->wavelength() * 1e9);

    case COLORING_CYCLE:
      return colors[m_simCount % (sizeof(colors) / sizeof(colors[0]))];

  }

  return QColor2uint32_t(m_repProp.fixedBeamColor);
}

void
SimulationState::applyRecordHits()
{
  auto allOpticalElements = m_topLevelModel->allOpticalElements();

  for (auto element : allOpticalElements)
    element->setRecordHits(false);

  for (auto &path : m_properties.footprints) {
    auto element = m_topLevelModel->resolveOpticalElement(path);
    if (element != nullptr)
      element->setRecordHits(true);
  }
}

std::list<std::string>
SimulationState::footprints() const
{
  std::list<std::string> fullNames;

  for (auto &p : m_footprintDiagrams)
    fullNames.push_back(p.first);

  return fullNames;
}

RZ::ScatterDataProduct *
SimulationState::getFootprint(std::string const &fullName) const
{
  auto it = m_footprintDiagrams.find(fullName);

  if (it == m_footprintDiagrams.end())
    return nullptr;

  return it->second;
}

std::list<std::string>
SimulationState::takeFootprintData()
{
  auto allOpticalElementNames = m_topLevelModel->opticalElementHierarchy("");
  std::list<std::string> changes;

  // TODO: Perhaps we can speed this up by adding another hash between
  // optical elements with recordHits enabled and their paths
  for (auto path : allOpticalElementNames) {
    auto element = m_topLevelModel->resolveOpticalElement(path);
    if (element != nullptr && element->recordHits()) {
      for (auto &surf : element->opticalSurfaces()) {
        auto fullName = path + "." + surf->name;

        if (m_footprintDiagrams.find(fullName) == m_footprintDiagrams.end())
          m_footprintDiagrams[fullName] = new RZ::ScatterDataProduct(
                surf->name + " of " + element->name());

        if (!surf->hits.empty()) {
          m_footprintDiagrams[fullName]->addSurface(
                0xff000000 | surf->hits[0].id,
                surf,
                "Simulation");

          changes.push_back(fullName);
          surf->hits.clear();
          surf->clearCache();
        }
      }
    }
  }

  return changes;
}

bool
SimulationState::initSimulation()
{
  RZ::Real wavelength = m_wavelengthExpr->evaluate();
  m_i = m_j = 0;

  setVariable("i", m_i);
  setVariable("j", m_j);

  setVariable("Ni", m_properties.Ni);
  setVariable("Nj", m_properties.Nj);
  setVariable("wavelength", wavelength);

  m_steps = m_properties.Ni * m_properties.Nj;
  m_currStep = 0;
  setVariable("step", 0);

  setVariable("stepN", randNormal());
  setVariable("stepU", randUniform());

  setVariable("simN", randNormal());
  setVariable("simU", randUniform());

  applyRecordHits();

  m_topLevelModel->updateRandState();
  m_topLevelModel->assignEverything();
  m_topLevelModel->setWavelength(wavelength * 1e-9);

  if (m_properties.saveArtifacts) {
    if (!QFile(m_properties.saveDir).exists()) {
      std::string asStdString = m_properties.saveDir.toStdString();
      if (mkdir(asStdString.c_str(), 0755) == -1) {
        m_lastCompileError =
            "Failed to create save directory: " +
            std::string(strerror(errno));
        RZError(
              "mkdir(): cannot create `%s': %s\n",
              asStdString.c_str(),
              strerror(errno));
        return false;
      }
    }

    std::string detector = m_properties.saveDetector.toStdString();
    m_saveDetector = findDetectorForPath(detector);
    if (m_saveDetector == nullptr)
      return false;

    resetPrefix();

    if (m_properties.saveCSV)
      if (!openCSV())
        return false;

  } else {
    m_saveDetector = nullptr;
  }

  ++m_simCount;

  setVariable("sim", SCAST(RZ::Real, m_simCount));

  applyDofs();

  // We transition to running state only if we manage to allocate rays.
  m_running = allocateRays(beamColorCycle());

  return m_running;
}

bool
SimulationState::sweepStep()
{
  RZ::Real wavelength = m_wavelengthExpr->evaluate();

  setVariable("stepN", randNormal());
  setVariable("stepU", randUniform());

  if (done())
    goto done;

  ++m_i;

  if (done())
    goto done;

  if (m_i >= m_properties.Ni) {
    m_i = 0;
    ++m_j;
    if (done())
      goto done;

  }

  if (done())
    goto done;

  setVariable("i", m_i);
  setVariable("j", m_j);
  setVariable("step", ++m_currStep);

  setVariable("wavelength", wavelength);
  m_topLevelModel->setWavelength(wavelength * 1e-9);

  // Iteration done, apply Dofs
  applyDofs();

  if (allocateRays(beamColorCycle()))
    return true;

done:
  closeCSV();
  m_running = false;
  return false;
}

bool
SimulationState::done() const
{
  if (m_properties.type == SIM_TYPE_1D_SWEEP)
    return m_i >= m_properties.Ni;

  if (m_properties.type == SIM_TYPE_2D_SWEEP)
    return m_j >= m_properties.Nj;

  return true;
}

int
SimulationState::steps() const
{
  return m_steps;
}

int
SimulationState::currStep() const
{
  return m_currStep;
}

int
SimulationState::simCount() const
{
  return m_simCount;
}

void
SimulationState::releaseRays()
{
  m_currBeam = m_beamAlloc.end();
  m_running = false;
}

SimulationProperties
SimulationState::properties() const
{
  return m_properties;
}

RepresentationProperties
SimulationState::repProperties() const
{
  return m_repProp;
}

std::list<RZ::Ray> const &
SimulationState::beam() const
{
  return *m_currBeam;
}

bool
SimulationState::setTopLevelModel(RZ::TopLevelModel *model)
{
  if (m_running) {
    RZError("Attempting to change top level model while a simulation is running!\n");
    return false;
  }

  m_topLevelModel = model;
  m_simCount = 0;

  return true;
}

SimulationState::SimulationState(RZ::TopLevelModel *model)
{
  m_currBeam      = m_beamAlloc.end();
  m_randState     = new RZ::ExprRandomState();

  setTopLevelModel(model);
}

SimulationState::~SimulationState()
{
  clearAll();

  if (m_randState != nullptr)
    delete m_randState;

  closeCSV();
}

////////////////////////////////// Simulation session /////////////////////////

void
SimulationSession::reload(RZ::ParserContext *context)
{
  bool ok = false;
  RZ::Recipe *recipe = nullptr;
  RZ::FileParserContext *fileCtx = nullptr;
  RZ::ParserError parserError;
  RZ::TopLevelModel *topLevelModel = nullptr;
  std::string strPath = m_path.toStdString();
  std::string strName = m_fileName.toStdString();
  std::string error;
  bool isParserError = false;
  FILE *fp = fopen(strPath.c_str(), "r");

  if (fp == nullptr) {
    error = "Cannot open " + strName + " for reading: ";
    error += strerror(errno);
    goto done;
  }

  if (context == nullptr) {
    recipe = new RZ::Recipe();
    recipe->addDof("t", 0, 0, 1e6);

    fileCtx = new RZ::FileParserContext(recipe);
    fileCtx->addSearchPath(m_searchPath.toStdString());
    fileCtx->setFile(fp, strName.c_str());
    context = fileCtx;
  } else {
    recipe = context->recipe();
  }

  try {
    context->parse();
  } catch (RZ::ParserError const &e) {
    isParserError = true;
    parserError = e;

    goto done;
  } catch (std::runtime_error const &e) {
    error = "Model file has errors:<pre>";
    error += e.what();
    error += "</pre>";

    goto done;
  }

  try {
    topLevelModel = new RZ::TopLevelModel(recipe);
  } catch (std::runtime_error const &e) {
    error = "Model has errors: ";
    error += e.what();
    goto done;
  }

  topLevelModel->setBeamColoring(m_rgbColoring);

  /////////////////////// From here, nothing should fail //////////////////////
  if (m_simState == nullptr) {
    m_simState = new SimulationState(topLevelModel);
  } else {
    if (!m_simState->setTopLevelModel(topLevelModel)) {
      error = "Failed to top model of simulation state (memory leak)!";
      goto done;
    }
  }

  if (m_tracer == nullptr) {
    m_tracer = new AsyncRayTracer(topLevelModel);
    m_tracer->moveToThread(m_tracerThread);

    connect(
          this,
          SIGNAL(triggerSimulation(QString, int, int)),
          m_tracer,
          SLOT(onStartRequested(QString, int, int)));

    connect(
          m_tracer,
          SIGNAL(finished(bool)),
          this,
          SLOT(onSimulationDone(bool)));

    connect(
          m_tracer,
          SIGNAL(aborted()),
          this,
          SLOT(onSimulationAborted()));

    connect(
          m_tracer,
          SIGNAL(error(QString)),
          this,
          SLOT(onSimulationError(QString)));
  } else {
    if (!m_tracer->setModel(topLevelModel)) {
      error = "Failed to top model of simulation state (memory leak)!";
      goto done;
    }
  }

  ok = true;

done:
  if (ok) {
    std::swap(recipe, m_recipe);
    std::swap(topLevelModel, m_topLevelModel);

    m_selectedElement = nullptr;
  }

  if (topLevelModel != nullptr)
    delete topLevelModel;

  if (fileCtx != nullptr)
    delete fileCtx;

  if (recipe != nullptr)
    delete recipe;

  if (!ok) {
    if (isParserError)
      throw parserError;
    else
      throw std::runtime_error(error);
  }
}

SimulationSession::SimulationSession(
    QString const &path,
    QObject *parent)
  : QObject{parent}
{
  QFileInfo info(path);

  m_path        = path;
  m_fileName    = info.fileName();
  m_searchPath  = info.dir().absolutePath();
  m_rgbColoring = &g_rgbColoring;

  m_timer    = new QTimer(this);
  connect(
        m_timer,
        SIGNAL(timeout()),
        this,
        SLOT(onTimerTick()));

  m_tracerThread = new QThread;
  connect(
        m_tracerThread,
        SIGNAL(finished()),
        m_tracerThread,
        SLOT(deleteLater()));

  reload();

  m_tracerThread->start();
}

SimulationSession::~SimulationSession()
{
  if (m_tracerThread != nullptr) {
    m_tracerThread->quit();
    m_tracerThread->wait();
  }

  if (m_topLevelModel != nullptr)
    delete m_topLevelModel;

 if (m_recipe != nullptr)
   delete m_recipe;
}

void
SimulationSession::updateAnim()
{
  m_topLevelModel->setDof("t", m_t);
  emit modelChanged();
}

SimulationState *
SimulationSession::state() const
{
  return m_simState;
}

AsyncRayTracer *
SimulationSession::tracer() const
{
  return m_tracer;
}

RZ::Recipe *
SimulationSession::recipe() const
{
  return m_recipe;
}

RZ::TopLevelModel *
SimulationSession::topLevelModel() const
{
  return m_topLevelModel;
}

void
SimulationSession::selectElement(RZ::Element *element)
{
  if (m_selectedElement != element) {
    if (m_selectedElement != nullptr)
      m_selectedElement->setSelected(false);

    m_selectedElement = element;

    if (m_selectedElement != nullptr)
      m_selectedElement->setSelected(true);

    emit modelChanged();
  }
}

void
SimulationSession::toggleCurrent()
{
  if (m_selectedElement != nullptr) {
    m_selectedElement->setVisible(!m_selectedElement->visible());
    emit modelChanged();
  }
}

RZ::Element *
SimulationSession::getSelectedElement() const
{
  return m_selectedElement;
}

QString
SimulationSession::path() const
{
  return m_path;
}

QString
SimulationSession::searchPath() const
{
  return m_searchPath;
}

QString
SimulationSession::fileName() const
{
  return m_fileName;
}

void
SimulationSession::iterateSimulation()
{
  struct timeval now, diff;
  bool refreshTimeout;

  gettimeofday(&now, nullptr);
  timersub(&now, &m_lastModelRefresh, &diff);

  refreshTimeout
      = diff.tv_sec * 1000 + diff.tv_usec / 1000 > RZGUI_MODEL_REFRESH_MS;

  refreshTimeout = true;

  if (m_simState->steps() > 1)
    tracer()->setUpdateBeam(refreshTimeout);

  if (m_simState->currStep() + 1 >= m_simState->steps())
    tracer()->setUpdateBeam(true);

  tracer()->setBeam(m_simState->beam());

  ++m_simPending;

  emit triggerSimulation(
        m_simState->properties().path,
        m_simState->currStep(),
        m_simState->steps());
}

bool
SimulationSession::runSimulation()
{
  if (m_simState->running()) {
    RZWarning("Cannot start simulation: another simulation is in progress\n");
    return true;
  }

  if (!m_simState->canRun()) {
    RZError("Cannot run simulation: simulation parameters are not completely defined\n");
    return false;
  }

  if (!m_simState->initSimulation())
    return false;

  RZInfo(
        "Triggering simulation #%d on %s: %d steps, %d rays per beam\n",
        m_simState->simCount(),
        m_fileName.toStdString().c_str(),
        m_simState->steps(),
        m_simState->properties().rays);


  gettimeofday(&m_lastModelRefresh, nullptr);

  m_tracer->setUpdateBeam(m_simState->steps() == 1);
  m_tracer->setAccumulate(m_simState->repProperties().accumulate);
  m_tracer->setDiffraction(
        m_simState->properties().ttype == TRACER_TYPE_DIFFRACTION);
  gettimeofday(&m_simulationStart, nullptr);
  iterateSimulation();

  return true;
}

void
SimulationSession::animPause()
{
  m_paused = true;
}

void
SimulationSession::animStop()
{
  m_timer->stop();
  m_t       = 0;
  m_playing = false;
  m_paused  = false;

  updateAnim();
}

void
SimulationSession::animBegin()
{
  m_t = 0;
  updateAnim();
}

void
SimulationSession::animEnd()
{
  m_t = 1e6;
  updateAnim();
}

void
SimulationSession::animPlay()
{
  if (!m_playing)
    m_timer->start(10);

  m_playing = true;
  m_paused  = false;
}


bool
SimulationSession::playing() const
{
  return m_playing && !m_paused;
}

bool
SimulationSession::stopped() const
{
  return !m_playing;
}

void
SimulationSession::onTimerTick()
{
  if (!m_paused) {
    m_t += .5;
    updateAnim();
  }
}
#include <QCoreApplication>
void
SimulationSession::onSimulationDone(bool haveBeam)
{
  if (m_simPending > 0)
    --m_simPending;

  if (haveBeam) {
    gettimeofday(&m_lastModelRefresh, nullptr);
    emit modelChanged();
    QCoreApplication::processEvents();
  }

  auto changes = m_simState->takeFootprintData();
  for (auto &p : changes)
    emit footprintDiagramChange(QString::fromStdString(p));

  if (m_simPending == 0)
    m_simState->releaseRays();

  m_simState->saveArtifacts();

  if (m_simState->sweepStep()) {
    iterateSimulation();
  } else {
    struct timeval now, diff;
    gettimeofday(&now, nullptr);

    timersub(&now, &m_simulationStart, &diff);


    emit sweepFinished();
    RZInfo(
          "Simulation finished (%s)\n",
          timeDeltaToString(diff).toStdString().c_str());
  }
}

void
SimulationSession::onSimulationAborted()
{
  if (m_simPending > 0)
    --m_simPending;

  if (m_simPending == 0)
    m_simState->releaseRays();

  emit sweepFinished();
  emit modelChanged();

  RZInfo("Simulation cancelled by user\n");
}


void
SimulationSession::onSimulationError(QString err)
{
  if (m_simPending > 0)
    --m_simPending;

  if (m_simPending == 0)
    m_simState->releaseRays();

  emit simulationError(err);
  emit sweepFinished();
}
