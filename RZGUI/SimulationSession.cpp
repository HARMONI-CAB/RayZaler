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

#include "SimulationSession.h"
#include <ParserContext.h>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <QTimer>
#include <QFileInfo>
#include <Elements/RayBeamElement.h>
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
#include <QElapsedTimer>
#include <GUIHelpers.h>


static inline uint32_t
QColor2uint32_t(QColor const &color)
{
  uint32_t tuple = 0;

  tuple |= static_cast<uint32_t>(color.red())   << 16;
  tuple |= static_cast<uint32_t>(color.green()) << 8;
  tuple |= static_cast<uint32_t>(color.blue());

  return tuple;
}

////////////////////////////// RayColoring ////////////////////////////////////
class RZGUIBeamColoring : public RZ::RayColoring {
  SimulationSession *m_session = nullptr;
public:
  RZGUIBeamColoring(SimulationSession *);
  virtual ~RZGUIBeamColoring() override;
  virtual void id2color(uint32_t id, GLfloat *rgb) const override;
};

RZGUIBeamColoring::RZGUIBeamColoring(SimulationSession *session)
{
  m_session = session;
}

RZGUIBeamColoring::~RZGUIBeamColoring()
{

}

void
RZGUIBeamColoring::id2color(uint32_t id, GLfloat *rgb) const
{
  uint32_t color = m_session->idToRgba(id);

  rgb[0] = static_cast<GLfloat>((color >> 16) & 0xff) / 255.f;
  rgb[1] = static_cast<GLfloat>((color >> 8)  & 0xff) / 255.f;
  rgb[2] = static_cast<GLfloat>((color >> 0)  & 0xff) / 255.f;
}

/////////////////////////// BeamSimulationState ///////////////////////////////
BeamSimulationState::BeamSimulationState(
    const SimulationBeamProperties &prop,
    ExprEvaluationContext *ctx) : evalCtx(ctx)
{
  properties = prop;
  stateName  = prop.name;
}

//////////////////////////// SimulationState //////////////////////////////////
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
SimulationState::clearAndSavePreviousBeams()
{
  auto next = m_currentBeams.begin();

  while (next != m_currentBeams.end()) {
    auto it = next++;
    BeamSimulationState *beam = *it;
    if (beam->complete) {
      m_previousBeams.splice(m_previousBeams.end(), m_currentBeams, it);
    } else {
      auto node = m_idToBeam.find(beam->id);
      if (node != m_idToBeam.end())
        m_idToBeam.erase(node);
      delete beam;
    }
  }

  m_currentBeams.clear();
}

BeamSimulationState *
SimulationState::makeBeamState(SimulationBeamProperties const &props)
{
  BeamSimulationState *beam = new BeamSimulationState(props, m_evalModelCtx);

  beam->id = m_nextBeamId++;

  m_idToBeam[beam->id] = beam;
  m_currentBeams.push_back(beam);

  if (m_properties.type == SIM_TYPE_1D_SWEEP)
    beam->stateName += QString::asprintf("[%d]", m_i);
  else if (m_properties.type == SIM_TYPE_2D_SWEEP)
    beam->stateName += QString::asprintf("[%d, %d]", m_i, m_j);

  return beam;
}

bool
SimulationState::createNewBeamStates()
{
  clearAndSavePreviousBeams();

#define TRY_DEFINE_BEAM_EXPR(name)                                           \
  if (!beamState->evalCtx.defineExpression(#name, bp.name.toStdString())) {  \
    m_firstFailedBeamExpr = #name;                                           \
    m_failedBeamId = static_cast<int32_t>(beamState->id);                    \
    return false;                                                            \
  } do {} while (false)

  for (auto &bp : m_properties.beams) {
    auto beamState = makeBeamState(bp);

    TRY_DEFINE_BEAM_EXPR(diameter);
    TRY_DEFINE_BEAM_EXPR(span);
    TRY_DEFINE_BEAM_EXPR(fNum);
    TRY_DEFINE_BEAM_EXPR(uX);
    TRY_DEFINE_BEAM_EXPR(uY);
    TRY_DEFINE_BEAM_EXPR(offsetX);
    TRY_DEFINE_BEAM_EXPR(offsetY);
    TRY_DEFINE_BEAM_EXPR(offsetZ);
    TRY_DEFINE_BEAM_EXPR(wavelength);

    beamState->complete = true;
  }
#undef TRY_DEFINE_BEAM_EXPR

  return true;
}

bool
SimulationState::setProperties(SimulationProperties const &prop)
{
  m_complete   = false;
  m_properties = prop;

  // Sanitize this
  if (prop.type == SIM_TYPE_ONE_SHOT)
    m_properties.Ni = m_properties.Nj = 1;
  else if (prop.type == SIM_TYPE_1D_SWEEP)
    m_properties.Nj = 1;

  if (m_evalModelCtx != nullptr)
    delete m_evalModelCtx;

  m_evalModelCtx = new ExprEvaluationContext(&m_evalSimCtx);

  // Start by DOF expressions
  for (auto p : prop.dofs) {
    auto dofExprName = p.first;
    auto dofVarName  = "dof_" + p.first;
    m_evalModelCtx->defineVariable(dofVarName);
    if (!m_evalModelCtx->defineExpression(dofExprName, p.second)) {
      m_firstFailedExpr = dofExprName;
      return false;
    }
  }

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
  if (m_currentRayGroup == m_rayGroupAlloc.end())
    m_currentRayGroup = m_rayGroupAlloc.begin();
  else
    ++m_currentRayGroup;

  if (m_currentRayGroup == m_rayGroupAlloc.end()) {
    // End of the list, alloc new group
    m_rayGroupAlloc.push_back(RayGroup());
    m_currentRayGroup = m_rayGroupAlloc.end();
    --m_currentRayGroup;
  }

  m_currentRayGroup->clear();

  for (auto &beamState : m_currentBeams) {
    RZ::BeamProperties prop;
    auto &beam = beamState->properties;

    // Define beam reference
    switch (beam.ref) {
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
        prop.setElementRelative(element);
        break;

      case BEAM_REFERENCE_APERTURE_STOP:
        element = m_topLevelModel->lookupOpticalElement(
              beam.apertureStop.toStdString());
        if (element == nullptr) {
          m_lastCompileError =
              "The specified element `" + beam.apertureStop.toStdString() + "' does not exist.";
          return false;
        }

        prop.setElementRelative(element);
        break;

      case BEAM_REFERENCE_FOCAL_PLANE:
        fp = m_topLevelModel->getFocalPlane(beam.focalPlane.toStdString());
        if (fp == nullptr) {
          m_lastCompileError = "The specified focal plane `" + beam.focalPlane.toStdString() + "' does not exist";
          return false;
        }
        break;
    }

    // Other properties
    auto D  = beamState->evalCtx.eval("diameter");
    auto S  = beamState->evalCtx.eval("span");
    auto ux = beamState->evalCtx.eval("uX");
    auto uy = beamState->evalCtx.eval("uY");
    auto x0 = beamState->evalCtx.eval("offsetX");
    auto y0 = beamState->evalCtx.eval("offsetY");
    auto z0 = beamState->evalCtx.eval("offsetZ");
    auto wl = beamState->evalCtx.eval("wavelength");

    auto uz = -sqrt(1 - ux * ux - uy * uy);

    prop.direction   = RZ::Vec3(ux, uy, uz);
    prop.offset      = RZ::Vec3(x0, y0, z0);
    prop.length      = 1;
    prop.id          = beamState->id;
    prop.wavelength  = wl;
    beamState->wavelength = wl;

    // Chief ray
    prop.vignetting  = false;
    prop.shape       = RZ::BeamShape::Point;
    prop.numRays     = 1;

    RZ::OMModel::addBeam(*m_currentRayGroup, prop);

    // Main beam
    prop.vignetting      = true;
    prop.shape           = beam.shape;
    prop.numRays         = static_cast<unsigned>(beam.rays);
    prop.diameter        = D;
    prop.random          = beam.random;
    prop.objectShape     = beam.objectShape;
    prop.angularDiameter = S;
    prop.objectPath      = beam.path.toStdString();

    // Define beam focus
    switch (beam.beam) {
      case BEAM_TYPE_COLLIMATED:
        prop.collimate();
        break;

      case BEAM_TYPE_CONVERGING:
        prop.setFNum(
              fabs(beamState->evalCtx.eval("fNum")),
              RZ::BeamDiameter);
        break;

      case BEAM_TYPE_DIVERGING:
        prop.setFNum(
            -fabs(beamState->evalCtx.eval("fNum")),
             RZ::BeamDiameter);
        break;
    }

    RZ::OMModel::addBeam(*m_currentRayGroup, prop);
  }

  return true;
}

void
SimulationState::clearBeams()
{
  if (m_topLevelModel != nullptr)
    m_topLevelModel->clearBeam();

  for (auto &beamState : m_previousBeams)
    delete beamState;

  for (auto &beamState : m_currentBeams)
    delete beamState;

  m_idToBeam.clear();
  m_nextBeamId = 0;

  m_previousBeams.clear();
  m_currentBeams.clear();
}

void
SimulationState::applyDofs()
{
  m_randState->update();

  auto dofExprs = m_evalModelCtx->expressions();

  for (auto p : dofExprs) {
    auto dofVal = m_evalModelCtx->eval(p);
    m_evalSimCtx.setVariable("dof_" + p, dofVal);
    m_topLevelModel->setDof(p, dofVal);
  }
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

    for (auto dof : m_topLevelModel->dofs()) {
      auto param = m_topLevelModel->lookupDof(dof);
      auto value = param == nullptr ? std::nan("unavail") : param->value;

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

void
SimulationState::clearFootprints()
{
  m_footprints.clear();
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

std::list<SurfaceFootprint> &
SimulationState::footprints()
{
  return m_footprints;
}

void
SimulationState::extractFootprintsFromSurface(
    std::string const &path,
    RZ::OpticalSurface *surf)
{
  uint32_t currId = 0xffffffff;
  BeamSimulationState *beamState = nullptr;
  SurfaceFootprint *footprint = nullptr;
  auto &locations = surf->locations();
  auto &directions = surf->directions();
  size_t i = 0;
  size_t prevIndex = 0;
  size_t hitsCount = surf->hits.size();

  std::map<uint32_t, SurfaceFootprint *> idToFootprint;

  for (i = 0; i < hitsCount; ++i) {
    auto &hit = surf->hits[i];

    if (hit.id != currId) {
      if (footprint != nullptr) {
        footprint->locations.insert(
              footprint->locations.end(),
              &locations[3 * prevIndex],
              &locations[3 * i]);

        footprint->directions.insert(
              footprint->directions.end(),
              &directions[3 * prevIndex],
              &directions[3 * i]);
      }

      prevIndex  = i;

      currId     = hit.id;
      beamState  = nullptr;
      footprint  = nullptr;

      auto it = m_idToBeam.find(currId);
      if (it == m_idToBeam.end())
        continue;

      beamState = it->second;

      // What??? What is this even for
      auto fit = idToFootprint.find(currId);
      if (fit == idToFootprint.end()) {
        SurfaceFootprint fp;
        fp.fullName    = path + "." + surf->name;
        fp.label       = beamState->stateName.toStdString();
        fp.color       = 0xff000000 | m_session->idToRgba(currId);
        fp.id          = currId;
        auto sit = surf->statistics.find(currId);
        // We artificall
        if (sit != surf->statistics.end()) {
          fp.vignetted   = sit->second.vignetted;
          fp.transmitted = sit->second.intercepted;
        }

        m_footprints.push_back(std::move(fp));
        footprint = &m_footprints.back();
        idToFootprint[currId] = footprint;
      } else {
        footprint = fit->second;
      }
    }
  }

  if (footprint != nullptr) {
    footprint->locations.insert(
          footprint->locations.end(),
          &locations[3 * prevIndex],
          &locations[3 * hitsCount]);

    footprint->directions.insert(
          footprint->directions.end(),
          &directions[3 * prevIndex],
          &directions[3 * hitsCount]);
  }

  surf->hits.clear();
  surf->clearCache();
  surf->clearStatistics();
}

// We are going to find lots of rays with different ids in the same
// surface. We need to go one by one and decide.
void
SimulationState::extractFootprints()
{
  auto allOpticalElementNames = m_topLevelModel->opticalElementHierarchy("");

  for (auto path : allOpticalElementNames) {
    auto element = m_topLevelModel->resolveOpticalElement(path);
    if (element != nullptr && element->recordHits()) {
      for (auto &surf : element->opticalSurfaces()) {
        if (!surf->hits.empty()) {
          auto mutSurf = const_cast<RZ::OpticalSurface *>(surf);
          extractFootprintsFromSurface(path, mutSurf);
        }
      }
    }
  }
}

bool
SimulationState::initSimulation()
{
  m_i = m_j = 0;

  m_evalSimCtx.setVariable("i", m_i);
  m_evalSimCtx.setVariable("j", m_j);

  m_evalSimCtx.setVariable("Ni", m_properties.Ni);
  m_evalSimCtx.setVariable("Nj", m_properties.Nj);


  m_steps = m_properties.Ni * m_properties.Nj;
  m_currStep = 0;
  m_evalSimCtx.setVariable("step", 0);

  m_evalSimCtx.setVariable("stepN", randNormal());
  m_evalSimCtx.setVariable("stepU", randUniform());

  m_evalSimCtx.setVariable("simN", randNormal());
  m_evalSimCtx.setVariable("simU", randUniform());

  applyRecordHits();

  m_topLevelModel->updateRandState();
  m_topLevelModel->assignEverything();

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

  m_evalSimCtx.setVariable("sim", SCAST(RZ::Real, m_simCount));

  applyDofs();

  if (!createNewBeamStates()) {
    RZError("Failed to create new beam states\n");
    return false;
  }

  // We transition to running state only if we manage to allocate rays.
  m_running = allocateRays();
  return m_running;
}

bool
SimulationState::sweepStep()
{
  m_evalSimCtx.setVariable("stepN", randNormal());
  m_evalSimCtx.setVariable("stepU", randUniform());

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

  m_evalSimCtx.setVariable("i", m_i);
  m_evalSimCtx.setVariable("j", m_j);
  m_evalSimCtx.setVariable("step", ++m_currStep);

  // Iteration done, apply Dofs
  applyDofs();

  if (!createNewBeamStates())
    goto done;

  if (allocateRays())
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
  m_currentRayGroup = m_rayGroupAlloc.end();
  m_running = false;
}

SimulationProperties
SimulationState::properties() const
{
  return m_properties;
}

std::list<RZ::Ray> const &
SimulationState::rayGroup() const
{
  return *m_currentRayGroup;
}

BeamSimulationState *
SimulationState::getBeamState(uint32_t id)
{
  auto it = m_idToBeam.find(id);

  if (it != m_idToBeam.end())
    return it->second;

  return nullptr;
}

void
SimulationState::initStateEvalCtx()
{
  m_evalSimCtx.defineVariable("i");
  m_evalSimCtx.defineVariable("j");
  m_evalSimCtx.defineVariable("Ni");
  m_evalSimCtx.defineVariable("Nj");
  m_evalSimCtx.defineVariable("simU");
  m_evalSimCtx.defineVariable("simN");
  m_evalSimCtx.defineVariable("stepU");
  m_evalSimCtx.defineVariable("stepN");
  m_evalSimCtx.defineVariable("step");
  m_evalSimCtx.defineVariable("sim");
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

SimulationState::SimulationState(SimulationSession *session)
{
  m_randState      = new RZ::ExprRandomState();
  m_session         = session;
  m_currentRayGroup = m_rayGroupAlloc.end();

  initStateEvalCtx();

  setTopLevelModel(session->topLevelModel());
}

SimulationState::~SimulationState()
{
  clearBeams();

  if (m_evalModelCtx != nullptr)
    delete m_evalModelCtx;

  if (m_randState != nullptr)
    delete m_randState;

  closeCSV();
}

////////////////////////////////// Simulation session /////////////////////////
uint32_t
SimulationSession::idToRgba(uint32_t id) const
{
  if (m_simState != nullptr) {
    auto beamState = m_simState->getBeamState(id);
    if (beamState != nullptr) {
      if (beamState->properties.colorByWl)
        return wl2uint32_t(beamState->wavelength);
      else
        return QColor2uint32_t(beamState->properties.color);
    }
  }

  return 0xffffee00;
}

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

  topLevelModel->setBeamColoring(m_beamColoring);

  /////////////////////// From here, nothing should fail //////////////////////
  if (m_simState == nullptr)
    m_simState = new SimulationState(this);

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
    if (!m_simState->setTopLevelModel(topLevelModel)) {
      error = "Failed to top model of simulation state (memory leak)!";
      ok = false;
    } else {
      std::swap(recipe, m_recipe);
      std::swap(topLevelModel, m_topLevelModel);
      m_selectedElement = nullptr;
    }
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

  m_path         = path;
  m_fileName     = info.fileName();
  m_searchPath   = info.dir().absolutePath();
  m_beamColoring = new RZGUIBeamColoring(this);

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

  if (m_tracer != nullptr)
    delete m_tracer;

  if (m_simState != nullptr)
    delete m_simState;

  if (m_topLevelModel != nullptr)
    delete m_topLevelModel;

 if (m_recipe != nullptr)
   delete m_recipe;

 if (m_beamColoring != nullptr)
   delete m_beamColoring;
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

  tracer()->setBeam(m_simState->rayGroup());

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
        "Triggering simulation #%d on %s: %d steps, %ld beams total\n",
        m_simState->simCount(),
        m_fileName.toStdString().c_str(),
        m_simState->steps(),
        m_simState->properties().beams.size());


  gettimeofday(&m_lastModelRefresh, nullptr);

  m_tracer->setUpdateBeam(m_simState->steps() == 1);
  m_tracer->setAccumulate(false);
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

  if (m_simPending == 0)
    m_simState->releaseRays();

  m_simState->saveArtifacts();

  if (m_simState->sweepStep()) {
    iterateSimulation();
  } else {
    struct timeval now, diff;
    gettimeofday(&now, nullptr);

    timersub(&now, &m_simulationStart, &diff);

    m_simState->extractFootprints();

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
