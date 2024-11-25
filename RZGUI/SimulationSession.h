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

#ifndef SIMULATIONSESSION_H
#define SIMULATIONSESSION_H

#include <QObject>
#include <Recipe.h>
#include <TopLevelModel.h>
#include <ExprTkEvaluator.h>
#include <QTimer>
#include <QColor>
#include "SimulationProperties.h"
#include "SpotDiagramWindow.h"
#include "GUIHelpers.h"
#include "ExprEvaluationContext.h"

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

class SimulationBeamState {
  QString name;

};

// Beam properties must be copied as they must be visible even after the
// user changes the properties of the current simulation.
struct BeamSimulationState {
  uint32_t                 id = 0;
  QString                  stateName;
  SimulationBeamProperties properties;
  ExprEvaluationContext    evalCtx;
  bool                     complete = false;
  qreal                    wavelength = 555e-9;

  BeamSimulationState(const SimulationBeamProperties &, ExprEvaluationContext *);
};

typedef std::list<RZ::Ray> RayGroup;

class SimulationSession;
class SimulationState {
  QString                  m_simName;
  SimulationProperties     m_properties;
  RZ::TopLevelModel       *m_topLevelModel = nullptr;
  RZ::ExprRandomState     *m_randState     = nullptr;
  RZ::RayBeamElement      *m_beamElement   = nullptr;
  ExprEvaluationContext    m_evalSimCtx;
  ExprEvaluationContext   *m_evalModelCtx = nullptr;
  std::string              m_lastCompileError = "";
  std::string              m_firstFailedExpr = "";
  std::string              m_firstFailedBeamExpr = "";
  int32_t                  m_failedBeamId = -1;
  SimulationSession       *m_session = nullptr;
  bool                     m_complete = false;

  // Simulation objects
  std::list<BeamSimulationState *>          m_previousBeams;
  std::list<BeamSimulationState *>          m_currentBeams;
  std::map<uint32_t, BeamSimulationState *> m_idToBeam;
  uint32_t                                  m_nextBeamId = 0;

  std::list<RayGroup>                       m_rayGroupAlloc;
  std::list<RayGroup>::iterator             m_currentRayGroup;

  // Data saving (set if and only if data saving is enabled)
  RZ::Detector *m_saveDetector = nullptr;
  FILE         *m_csvFp        = nullptr;
  unsigned int  m_pfxCount     = 0;
  QString       m_currentSavePrefix;

  // Data products
  std::list<SurfaceFootprint> m_footprints;

  // Simulation progress
  bool    m_running  = false;
  int64_t m_simCount = 0;
  int     m_steps    = 1;
  int     m_currStep = 0;
  int     m_i        = 0;
  int     m_j        = 0;

  // Private functions
  BeamSimulationState *makeBeamState(SimulationBeamProperties const &);
  void clearAndSavePreviousBeams();
  void initStateEvalCtx();
  void applyDofs();
  void resetPrefix();
  void genPrefix();
  QString getCurrentOutputCSVFileName() const;
  QString getCurrentOutputFileName() const;
  void bumpPrefix();
  void clearBeams();

  RZ::Detector *findDetectorForPath(std::string const &);

  void applyRecordHits();

  void extractFootprintsFromSurface(
      std::string const &path,
      RZ::OpticalSurface *);

  bool openCSV();
  void saveCSV();
  void closeCSV();

  bool createNewBeamStates();

public:
  SimulationState(SimulationSession *);
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
  bool allocateRays();
  void releaseRays();

  void extractFootprints();
  std::list<SurfaceFootprint> &footprints();
  void clearFootprints();

  bool setProperties(SimulationProperties const &);
  std::string getLastError() const;
  SimulationProperties properties() const;
  std::list<RZ::Ray> const &rayGroup() const;
  BeamSimulationState *getBeamState(uint32_t id);
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
  RZ::RayColoring   *m_beamColoring;
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

  uint32_t             idToRgba(uint32_t) const;

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
