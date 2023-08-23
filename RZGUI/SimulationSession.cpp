#include "SimulationSession.h"
#include <ParserContext.h>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <QTimer>
#include <QFileInfo>
#include <RayBeamElement.h>

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
  clearAll();

  m_dictionary.clear();

  // Recreate dictionary
  m_dictionary["i"]     = &m_i;
  m_dictionary["j"]     = &m_i;
  m_dictionary["Ni"]    = &m_Ni;
  m_dictionary["Nj"]    = &m_Nj;
  m_dictionary["D"]     = &m_D;
  m_dictionary["fNum"]  = &m_fNum;
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
  auto path = m_topLevelModel->lookupOpticalPath(m_properties.path.toStdString());
  if (path == nullptr) {
    m_lastCompileError = "The defined optical path does not exist";
    return false;
  }

  if (path->m_sequence.size() == 0) {
    m_lastCompileError = "Optical path contains no elements";
    return false;
  }

  auto element = path->m_sequence.begin()->parent;

  m_beam.clear();

  RZ::OMModel::addElementRelativeBeam(
        m_beam,
        element,
        static_cast<unsigned>(m_properties.rays),
        .5 * m_diamExpr->evaluate(),
        m_azimuthExpr->evaluate(),
        m_elevationExpr->evaluate(),
        m_offsetXExpr->evaluate(),
        m_offsetYExpr->evaluate(),
        1);

  return true;
}


SimulationProperties
SimulationState::properties() const
{
  return m_properties;
}

std::list<RZ::Ray> const &
SimulationState::beam() const
{
  return m_beam;
}

SimulationState::SimulationState(RZ::TopLevelModel *model)
{
  m_topLevelModel = model;
}

SimulationState::~SimulationState()
{
  clearAll();
}

////////////////////////////////// Simulation session //////////////////////////
class FileParserContext : public RZ::ParserContext {
    using ParserContext::ParserContext;
    FILE *m_fp = stdin;

  public:
    void setFile(FILE *fp, std::string const &name);
    virtual int read() override;
    virtual ~FileParserContext();
};

void
FileParserContext::setFile(FILE *fp, std::string const &name)
{
  m_fp = fp;
  ParserContext::setFile(name);
}

int
FileParserContext::read()
{
  return fgetc(m_fp);
}

FileParserContext::~FileParserContext()
{
  if (m_fp != nullptr && m_fp != stdin)
    fclose(m_fp);
}

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
  m_context = new FileParserContext(m_recipe);
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
}

SimulationSession::~SimulationSession()
{
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

QString
SimulationSession::fileName() const
{
  return m_fileName;
}

bool
SimulationSession::runSimulation()
{
  if (!m_simState->canRun())
    return false;

  if (!m_simState->allocateRays())
    return false;

  RZ::RayBeamElement *element =
      static_cast<RZ::RayBeamElement *>(m_topLevelModel->beam());

  element->setList(m_simState->beam());
  emit modelChanged();

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
