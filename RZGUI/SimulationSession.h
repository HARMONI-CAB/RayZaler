#ifndef SIMULATIONSESSION_H
#define SIMULATIONSESSION_H

#include <QObject>
#include <Recipe.h>
#include <TopLevelModel.h>
#include <SimpleExpressionEvaluator.h>
#include <QTimer>
#include "GUIHelpers.h"

//
// A Simulation Session is an object that keeps the current state of the
// opened model. From here we trigger simulations, adjust degrees of freedom,
// detect errors, etc.
//
// The Simulation Session also keeps ownership on the Opto-Mechanical model,
// and cannot exist if the model failed to be loaded.
//
// Things that can be done to a Simulation Session:
//
// + getRecipe(): gets the current recipe object
// + getModel(): gets the current, top-level, optomechanical model
// + getFileName(): gets the filename

class AsyncRayTracer;
class FileParserContext;
class QAbstractTableModel;

namespace RZ {
  class FileParserContext;
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

struct SimulationProperties {
  SimulationType type = SIM_TYPE_ONE_SHOT;
  BeamType       beam = BEAM_TYPE_COLLIMATED;

  QString diameter    = "40e-3";       // m
  QString refAperture = "200e-3";      // m
  QString fNum        = "17.37";
  QString azimuth     = "90";      // deg
  QString elevation   = "0";       // deg
  QString offsetX     = "0";       // m
  QString offsetY     = "0";       // m

  int rays            = 1000;
  int Ni              = 10;
  int Nj              = 10;

  QString detector;
  QString path;

  std::map<std::string, std::string> dofs;
};

class SimulationState {
  SimulationProperties       m_properties;
  RZ::TopLevelModel         *m_topLevelModel = nullptr;

  SimpleExpressionEvaluator *m_diamExpr      = nullptr;
  SimpleExpressionEvaluator *m_refApExpr     = nullptr;
  SimpleExpressionEvaluator *m_fNumExpr      = nullptr;

  SimpleExpressionEvaluator *m_azimuthExpr   = nullptr;
  SimpleExpressionEvaluator *m_elevationExpr = nullptr;

  SimpleExpressionEvaluator *m_offsetXExpr   = nullptr;
  SimpleExpressionEvaluator *m_offsetYExpr   = nullptr;

  std::map<std::string, SimpleExpressionEvaluator *> m_dofExprs;
  std::map<std::string, RZ::Real> m_dofValues;

  // Evaluated members
  RZ::Real m_i,  m_j;
  RZ::Real m_Ni, m_Nj;
  RZ::Real m_D,  m_fNum, m_refAp;

  RZ::Real m_azimuth, m_elevation;
  RZ::Real m_offsetX, m_offsetY;

  SimpleExpressionDict m_dictionary;

  std::string m_lastCompileError = "";
  bool m_complete = false;

  // Simulation objects
  std::list<std::list<RZ::Ray>> m_beamAlloc;
  std::list<std::list<RZ::Ray>>::iterator m_currBeam;

  void clearAll();
  bool trySetExpr(SimpleExpressionEvaluator * &, std::string const &);
  void applyDofs();

  int m_steps = 1;
  int m_currStep = 0;

public:
  SimulationState(RZ::TopLevelModel *);
  ~SimulationState();

  bool canRun() const;
  bool initSimulation();
  bool sweepStep();
  bool done() const;

  int steps() const;
  int currStep() const;

  bool allocateRays();
  void releaseRays();
  bool setProperties(SimulationProperties const &);
  std::string getFirstInvalidExpr() const;
  std::string getLastError() const;
  SimulationProperties properties() const;
  std::list<RZ::Ray> const &beam() const;
};

class SimulationSession : public QObject
{
  Q_OBJECT

  QString            m_path;
  QString            m_fileName;
  RZ::FileParserContext *m_context       = nullptr;
  RZ::Recipe        *m_recipe            = nullptr;
  RZ::TopLevelModel *m_topLevelModel     = nullptr;
  RZ::Element       *m_selectedElement   = nullptr;
  AsyncRayTracer    *m_tracer            = nullptr;
  QThread           *m_tracerThread      = nullptr;
  SimulationState   *m_simState          = nullptr;
  QTimer            *m_timer             = nullptr;

  qreal              m_t                 = 0;
  bool               m_paused            = false;
  bool               m_playing           = false;
  int                m_simPending        = 0;

  void               updateAnim();

  void               iterateSimulation();

public:
  explicit SimulationSession(QString const &path, QObject *parent = nullptr);
  virtual ~SimulationSession() override;

  SimulationState     *state() const;
  RZ::Recipe          *recipe() const;
  RZ::TopLevelModel   *topLevelModel() const;
  AsyncRayTracer      *tracer() const;

  // Session actions
  void                 selectElement(RZ::Element *);

  QString              fileName() const;

  bool                 runSimulation();

  void                 animPause();
  void                 animStop();
  void                 animBegin();
  void                 animEnd();
  void                 animPlay();

  bool                 playing() const;
  bool                 stopped() const;

signals:
  void modelChanged();
  void triggerSimulation(QString, int, int);
  void simulationError(QString);
  void sweepFinished();

public slots:
  void onTimerTick();
  void onSimulationDone();
  void onSimulationAborted();
  void onSimulationError(QString);
};

#endif // SIMULATIONSESSION_H
