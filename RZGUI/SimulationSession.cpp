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
#include <QJsonObject>
#include <QJsonDocument>

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

  DESERIALIZE(type);
  DESERIALIZE(beam);
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
  DESERIALIZE(rays);
  DESERIALIZE(Ni);
  DESERIALIZE(Nj);
  DESERIALIZE(detector);
  DESERIALIZE(path);
  DESERIALIZE(dofs);

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

  for (auto p : m_dofExprs)
    if (p.second != nullptr)
      delete p.second;

  m_dofExprs.clear();
  m_dofValues.clear();

  m_complete = false;
}

bool
SimulationState::trySetExpr(
    SimpleExpressionEvaluator * &dest,
    std::string const &expr)
{
  SimpleExpressionEvaluator *evaluator =
      new SimpleExpressionEvaluator(m_dictionary);
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
SimulationState::canRun() const
{
  return m_complete;
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

  m_dictionary.clear();

  // Recreate dictionary
  m_dictionary["i"]     = &m_i;
  m_dictionary["j"]     = &m_j;
  m_dictionary["Ni"]    = &m_Ni;
  m_dictionary["Nj"]    = &m_Nj;
  m_dictionary["D"]     = &m_D;
  m_dictionary["fNum"]  = &m_fNum;
  m_dictionary["A"]     = &m_refAp;
  m_dictionary["az"]    = &m_azimuth;
  m_dictionary["el"]    = &m_elevation;
  m_dictionary["x0"]    = &m_offsetX;
  m_dictionary["y0"]    = &m_offsetY;

  for (auto p : prop.dofs) {
    m_dofExprs[p.first]  = nullptr;
    m_dofValues[p.first] = 0.;
    m_dictionary["dof_" + p.first] = &m_dofValues[p.first];
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

  for (auto p : prop.dofs)
    if (!trySetExpr(m_dofExprs[p.first], p.second))
        return false;

  m_complete = true;

  return true;
}

bool
SimulationState::allocateRays()
{
  const RZ::OpticalPath *path = nullptr;
  RZ::OpticalElement *element = nullptr;
  RZ::ReferenceFrame *fp = nullptr;

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

      element = path->m_sequence.begin()->parent;

      switch (m_properties.beam) {
        case BEAM_TYPE_COLLIMATED:
          RZ::OMModel::addElementRelativeBeam(
                *m_currBeam,
                element,
                static_cast<unsigned>(m_properties.rays),
                .5 * m_diamExpr->evaluate(),
                m_azimuthExpr->evaluate(),
                m_elevationExpr->evaluate(),
                m_offsetXExpr->evaluate(),
                m_offsetYExpr->evaluate(),
                1);
          break;

        case BEAM_TYPE_CONVERGING:
          RZ::OMModel::addElementRelativeFocusBeam(
                *m_currBeam,
                element,
                static_cast<unsigned>(m_properties.rays),
                .5 * m_diamExpr->evaluate(),
                m_fNumExpr->evaluate(),
                m_refApExpr->evaluate(),
                m_azimuthExpr->evaluate(),
                m_elevationExpr->evaluate(),
                m_offsetXExpr->evaluate(),
                m_offsetYExpr->evaluate(),
                1);
          break;

        case BEAM_TYPE_DIVERGING:
          RZ::OMModel::addElementRelativeFocusBeam(
                *m_currBeam,
                element,
                static_cast<unsigned>(m_properties.rays),
                .5 * m_diamExpr->evaluate(),
                -m_fNumExpr->evaluate(),
                m_refApExpr->evaluate(),
                m_azimuthExpr->evaluate(),
                m_elevationExpr->evaluate(),
                m_offsetXExpr->evaluate(),
                m_offsetYExpr->evaluate(),
                1);
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
                m_fNumExpr->evaluate(),
                m_azimuthExpr->evaluate(),
                m_elevationExpr->evaluate(),
                m_offsetXExpr->evaluate(),
                m_offsetYExpr->evaluate(),
                1);
          break;

        case BEAM_TYPE_DIVERGING:
          RZ::OMModel::addFocalPlaneFocusedBeam(
                *m_currBeam,
                fp,
                static_cast<unsigned>(m_properties.rays),
                -m_fNumExpr->evaluate(),
                m_azimuthExpr->evaluate(),
                m_elevationExpr->evaluate(),
                m_offsetXExpr->evaluate(),
                m_offsetYExpr->evaluate(),
                1);
          break;
      }
      break;
  }

  return true;
}

void
SimulationState::applyDofs()
{
  for (auto p : m_dofExprs) {
    m_dofValues[p.first] = p.second->evaluate();
    m_topLevelModel->setDof(p.first, m_dofValues[p.first]);
  }
}

bool
SimulationState::initSimulation()
{
  m_i  = m_j = 0;
  m_Ni = m_properties.Ni;
  m_Nj = m_properties.Nj;

  m_steps = m_properties.Ni * m_properties.Nj;
  m_currStep = 0;

  applyDofs();

  return allocateRays();
}

bool
SimulationState::sweepStep()
{
  if (done())
    return false;

  m_i += 1;

  if (done())
    return false;

  if (m_i >= m_Ni) {
    m_i = 0;
    m_j += 1;
    if (done())
      return false;

  }

  if (done())
    return false;

  // Iteration done, apply Dofs
  applyDofs();

  ++m_currStep;

  return allocateRays();
}

bool
SimulationState::done() const
{
  if (m_properties.type == SIM_TYPE_1D_SWEEP)
    return m_i >= m_Ni;

  if (m_properties.type == SIM_TYPE_2D_SWEEP)
    return m_j >= m_Nj;

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


void
SimulationState::releaseRays()
{
  m_currBeam = m_beamAlloc.end();
}

SimulationProperties
SimulationState::properties() const
{
  return m_properties;
}

std::list<RZ::Ray> const &
SimulationState::beam() const
{
  return *m_currBeam;
}

SimulationState::SimulationState(RZ::TopLevelModel *model)
{
  m_topLevelModel = model;
  m_currBeam      = m_beamAlloc.end();
}

SimulationState::~SimulationState()
{
  clearAll();
}

////////////////////////////////// Simulation session /////////////////////////

SimulationSession::SimulationSession(
    QString const &path,
    QObject *parent)
  : QObject{parent}
{
  QFileInfo info(path);

  m_path     = path;
  m_fileName = info.fileName();

  m_timer    = new QTimer(this);
  connect(
        m_timer,
        SIGNAL(timeout()),
        this,
        SLOT(onTimerTick()));

  std::string strPath = path.toStdString();
  std::string strName = m_fileName.toStdString();

  FILE *fp = fopen(strPath.c_str(), "r");

  if (fp == nullptr) {
    std::string error = "Cannot open " + strName + " for reading: ";
    error += strerror(errno);

    throw std::runtime_error(error);
  }

  m_recipe  = new RZ::Recipe();
  m_recipe->addDof("t", 0, 0, 1e6);
  m_context = new RZ::FileParserContext(m_recipe);
  m_context->addSearchPath(info.dir().absolutePath().toStdString());
  m_context->setFile(fp, strName.c_str());

  try {
    m_context->parse();
  } catch (std::runtime_error &e) {
    std::string error = "Model file has errors:<pre>";
    error += e.what();
    error += "</pre>";

    throw std::runtime_error(error);
  }

  delete m_context;
  m_context = nullptr;

  try {
    m_topLevelModel = new RZ::TopLevelModel(m_recipe);
  } catch (std::runtime_error &e) {
    std::string error = "Model has errors: ";
    error += e.what();

    throw std::runtime_error(error);
  }

  m_simState = new SimulationState(m_topLevelModel);
  m_tracerThread = new QThread;
  m_tracer       = new AsyncRayTracer(m_topLevelModel);

  m_tracer->moveToThread(m_tracerThread);

  connect(
        m_tracerThread,
        SIGNAL(finished()),
        m_tracerThread,
        SLOT(deleteLater()));

  connect(
        this,
        SIGNAL(triggerSimulation(QString, int, int)),
        m_tracer,
        SLOT(onStartRequested(QString, int, int)));

  connect(
        m_tracer,
        SIGNAL(finished()),
        this,
        SLOT(onSimulationDone()));

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

 if (m_context != nullptr)
   delete m_context;
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

QString
SimulationSession::fileName() const
{
  return m_fileName;
}

void
SimulationSession::iterateSimulation()
{
  tracer()->setBeam(m_simState->beam());

  if (m_simState->currStep() + 1 >= m_simState->steps())
    emit modelChanged();

  ++m_simPending;

  emit triggerSimulation(
        m_simState->properties().path,
        m_simState->currStep(),
        m_simState->steps());
}

bool
SimulationSession::runSimulation()
{
  if (!m_simState->canRun())
    return false;

  if (!m_simState->initSimulation())
    return false;

  m_tracer->setUpdateBeam(m_simState->steps() == 1);

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

void
SimulationSession::onSimulationDone()
{
  if (m_simPending > 0)
    --m_simPending;

  if (m_simPending == 0)
    m_simState->releaseRays();

  if (m_simState->sweepStep()) {
    iterateSimulation();
  } else {
    emit sweepFinished();
    emit modelChanged();
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
