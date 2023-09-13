#ifndef SIMULATIONSESSION_H
#define SIMULATIONSESSION_H

#include <QObject>
#include <Recipe.h>
#include <TopLevelModel.h>
#include <ExprTkEvaluator.h>
#include <QTimer>
#include "GUIHelpers.h"

#define RZGUI_MODEL_REFRESH_MS 100

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

enum BeamReference {
  BEAM_REFERENCE_INPUT_ELEMENT,
  BEAM_REFERENCE_APERTURE_STOP,
  BEAM_REFERENCE_FOCAL_PLANE
};

struct SimulationProperties {
  SimulationType type  = SIM_TYPE_ONE_SHOT;
  BeamType       beam  = BEAM_TYPE_COLLIMATED;
  BeamReference  ref   = BEAM_REFERENCE_INPUT_ELEMENT;

  QString diameter     = "40e-3";       // m
  QString refAperture  = "200e-3";      // m
  QString focalPlane   = "";
  QString apertureStop = "";
  QString fNum         = "17.37";
  QString azimuth      = "90";      // deg
  QString elevation    = "0";       // deg
  QString offsetX      = "0";       // m
  QString offsetY      = "0";       // m

  int rays             = 1000;
  int Ni               = 10;
  int Nj               = 10;

  QString detector;
  QString path;

  std::map<std::string, std::string> dofs;

  // Artifact generation properties
  bool    saveArtifacts = false;
  bool    saveCSV       = true;
  bool    clearDetector = false;
  bool    overwrite     = false;

  QString saveDir = "artifacts";
  QString saveDetector = "";

  void loadDefaults();
  QString lastError() const;

  QByteArray serialize() const;
  bool deserialize(QByteArray const &);

private:
  QString m_lastError;

  bool deserialize(QJsonObject const &, QString const &, SimulationType &);
  bool deserialize(QJsonObject const &, QString const &, BeamType &);
  bool deserialize(QJsonObject const &, QString const &, BeamReference &);

  bool deserialize(QJsonObject const &, QString const &, QString &);
  bool deserialize(QJsonObject const &, QString const &, int &);
  bool deserialize(QJsonObject const &, QString const &, bool &);
  bool deserialize(QJsonObject const &, QString const &, std::map<std::string, std::string> &);
};

class SimulationState {
  SimulationProperties     m_properties;
  RZ::TopLevelModel       *m_topLevelModel = nullptr;
  RZ::ExprRandomState     *m_randState     = nullptr;

  RZ::ExprTkEvaluator *m_diamExpr      = nullptr;
  RZ::ExprTkEvaluator *m_refApExpr     = nullptr;
  RZ::ExprTkEvaluator *m_fNumExpr      = nullptr;

  RZ::ExprTkEvaluator *m_azimuthExpr   = nullptr;
  RZ::ExprTkEvaluator *m_elevationExpr = nullptr;

  RZ::ExprTkEvaluator *m_offsetXExpr   = nullptr;
  RZ::ExprTkEvaluator *m_offsetYExpr   = nullptr;

  std::map<std::string, RZ::ExprTkEvaluator *> m_dofExprs;

  // Variables available to the simulation parameter expressions
  std::map<std::string, RZ::RecipeParameter>     m_varDescriptions;
  RZ::GenericEvaluatorSymbolDict                 m_dictionary;

  std::string m_lastCompileError = "";
  bool m_complete = false;

  // Simulation objects
  std::list<std::list<RZ::Ray>> m_beamAlloc;
  std::list<std::list<RZ::Ray>>::iterator m_currBeam;

  // Data saving (set if and only if data saving is enabled)
  RZ::Detector *m_saveDetector = nullptr;
  FILE         *m_csvFp = nullptr;
  unsigned int  m_pfxCount = 0;
  QString       m_currentSavePrefix;

  // Simulation progress
  int64_t m_simCount = 0;
  int m_steps = 1;
  int m_currStep = 0;
  int m_i = 0;
  int m_j = 0;

  // Private functions
  void clearAll();
  bool trySetExpr(RZ::ExprTkEvaluator * &, std::string const &);
  void defineVariable(
      std::string const &,
      RZ::Real value = 0,
      RZ::Real min = -std::numeric_limits<RZ::Real>::infinity(),
      RZ::Real max = +std::numeric_limits<RZ::Real>::infinity());
  RZ::Real setVariable(std::string const &, RZ::Real);
  void applyDofs();
  void resetPrefix();
  void genPrefix();
  QString getCurrentOutputCSVFileName() const;
  QString getCurrentOutputFileName() const;
  void bumpPrefix();

  RZ::Detector *findDetectorForPath(std::string const &);

  bool openCSV();
  void saveCSV();
  void closeCSV();

public:
  SimulationState(RZ::TopLevelModel *);
  ~SimulationState();

  bool canRun() const;
  bool initSimulation();
  bool sweepStep();
  bool done() const;

  int steps() const;
  int currStep() const;
  int simCount() const;

  void saveArtifacts();
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

  struct timeval     m_lastModelRefresh;

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
  void onSimulationDone(bool);
  void onSimulationAborted();
  void onSimulationError(QString);
};

#endif // SIMULATIONSESSION_H
