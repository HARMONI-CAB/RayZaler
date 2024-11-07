#ifndef SIMULATIONSESSION_H
#define SIMULATIONSESSION_H

#include <QObject>
#include <Recipe.h>
#include <TopLevelModel.h>
#include <ExprTkEvaluator.h>
#include <QTimer>
#include <QColor>
#include "SpotDiagramWindow.h"
#include "GUIHelpers.h"

#define RZGUI_MODEL_REFRESH_MS 100
#define RZGUI_MODEL_DEFAULT_RAY_COLOR 0xffff00 // Yellow

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
//

class AsyncRayTracer;
class FileParserContext;
class QAbstractTableModel;

namespace RZ {
  class RayBeamElement;
  class RayColoring;
  class ParserContext;
  class DataProduct;
  class ScatterDataProduct;
};

enum ColoringMode {
  COLORING_FIXED,
  COLORING_WAVELENGTH,
  COLORING_CYCLE
};
Q_DECLARE_METATYPE(ColoringMode);

struct RepresentationProperties {
  bool accumulate           = false;
  ColoringMode coloringMode = COLORING_FIXED;
  QColor fixedBeamColor     = QColor(1, 1, 0);

  bool
  operator==(const RepresentationProperties & o){
    bool equals = true;

    equals = equals && o.accumulate == accumulate;
    equals = equals && o.coloringMode == coloringMode;
    equals = equals && o.fixedBeamColor == fixedBeamColor;

    return equals;
  }

  friend QDataStream &operator<<(QDataStream &out, const RepresentationProperties &rhs){
    out << rhs.accumulate;
    out << rhs.coloringMode;
    out << rhs.fixedBeamColor;
    return out;
  }
  friend QDataStream &operator>>(QDataStream &in, RepresentationProperties &rhs){
    in >> rhs.accumulate;
    in >> rhs.coloringMode;
    in >> rhs.fixedBeamColor;
    return in;
  }

};
Q_DECLARE_METATYPE(RepresentationProperties);

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

struct SimulationProperties {
  TracerType     ttype = TRACER_TYPE_GEOMETRIC_OPTICS;
  SimulationType type  = SIM_TYPE_ONE_SHOT;
  BeamType       beam  = BEAM_TYPE_COLLIMATED;
  BeamReference  ref   = BEAM_REFERENCE_INPUT_ELEMENT;
  RZ::BeamShape  shape = RZ::BeamShape::Circular;

  QString diameter     = "40e-3";       // m
  QString refAperture  = "200e-3";      // m
  QString focalPlane   = "";
  QString apertureStop = "";
  QString fNum         = "17.37";
  QString azimuth      = "90";      // deg
  QString elevation    = "0";       // deg
  QString offsetX      = "0";       // m
  QString offsetY      = "0";       // m
  QString wavelength   = "525";     // nm

  int  rays            = 1000;
  int  Ni              = 10;
  int  Nj              = 10;
  bool random          = false; // Random sampling

  std::list<std::string> footprints;

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

  bool deserialize(QJsonObject const &, QString const &, TracerType &);
  bool deserialize(QJsonObject const &, QString const &, SimulationType &);
  bool deserialize(QJsonObject const &, QString const &, BeamType &);
  bool deserialize(QJsonObject const &, QString const &, BeamReference &);
  bool deserialize(QJsonObject const &, QString const &, RZ::BeamShape &);
  bool deserialize(QJsonObject const &, QString const &, std::list<std::string> &);

  bool deserialize(QJsonObject const &, QString const &, QString &);
  bool deserialize(QJsonObject const &, QString const &, int &);
  bool deserialize(QJsonObject const &, QString const &, bool &);
  bool deserialize(QJsonObject const &, QString const &, qreal &);
  bool deserialize(QJsonObject const &, QString const &, std::map<std::string, std::string> &);
};

class SimulationState {
  SimulationProperties     m_properties;
  RepresentationProperties m_repProp;
  RZ::TopLevelModel       *m_topLevelModel = nullptr;
  RZ::ExprRandomState     *m_randState     = nullptr;
  RZ::RayBeamElement      *m_beamElement   = nullptr;

  RZ::ExprTkEvaluator *m_diamExpr      = nullptr;
  RZ::ExprTkEvaluator *m_refApExpr     = nullptr;
  RZ::ExprTkEvaluator *m_fNumExpr      = nullptr;

  RZ::ExprTkEvaluator *m_azimuthExpr   = nullptr;
  RZ::ExprTkEvaluator *m_elevationExpr = nullptr;

  RZ::ExprTkEvaluator *m_offsetXExpr   = nullptr;
  RZ::ExprTkEvaluator *m_offsetYExpr   = nullptr;

  RZ::ExprTkEvaluator *m_wavelengthExpr = nullptr;

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

  // Data products
  std::list<SurfaceFootprint> m_footprints;

  // Simulation progress
  bool m_running = false;
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
  uint32_t beamColorCycle() const;
  QString getCurrentOutputCSVFileName() const;
  QString getCurrentOutputFileName() const;
  void bumpPrefix();

  RZ::Detector *findDetectorForPath(std::string const &);

  void applyRecordHits();

  bool openCSV();
  void saveCSV();
  void closeCSV();

public:
  SimulationState(RZ::TopLevelModel *);
  ~SimulationState();

  bool setTopLevelModel(RZ::TopLevelModel *);
  bool canRun() const;
  bool running() const;
  bool initSimulation();
  bool sweepStep();
  bool done() const;

  int steps() const;
  int currStep() const;
  int simCount() const;

  void saveArtifacts();
  bool allocateRays(uint32_t color = RZGUI_MODEL_DEFAULT_RAY_COLOR);
  void releaseRays();

  void extractFootprints();
  std::list<SurfaceFootprint> &footprints();

  bool setProperties(SimulationProperties const &);
  void setRepresentationProperties(RepresentationProperties const &repProp);
  std::string getFirstInvalidExpr() const;
  std::string getLastError() const;
  SimulationProperties properties() const;
  RepresentationProperties repProperties() const;
  std::list<RZ::Ray> const &beam() const;
};

class SimulationSession : public QObject
{
  Q_OBJECT

  QString            m_path;
  QString            m_fileName;
  QString            m_searchPath;
  RZ::Recipe        *m_recipe            = nullptr;
  RZ::TopLevelModel *m_topLevelModel     = nullptr;
  RZ::Element       *m_selectedElement   = nullptr;
  AsyncRayTracer    *m_tracer            = nullptr;
  QThread           *m_tracerThread      = nullptr;
  SimulationState   *m_simState          = nullptr;
  QTimer            *m_timer             = nullptr;
  RZ::RayColoring   *m_rgbColoring;
  qreal              m_t                 = 0;
  bool               m_paused            = false;
  bool               m_playing           = false;
  int                m_simPending        = 0;

  struct timeval     m_simulationStart;
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
  void                 toggleCurrent();
  RZ::Element         *getSelectedElement() const;
  QString              path() const;
  QString              searchPath() const;
  QString              fileName() const;

  bool                 runSimulation();

  void                 reload(RZ::ParserContext *context = nullptr);
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
