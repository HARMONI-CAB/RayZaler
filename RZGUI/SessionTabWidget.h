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

#ifndef SESSIONTABWIDGET_H
#define SESSIONTABWIDGET_H

#include <QWidget>
#include "SimulationSession.h"

namespace Ui {
  class SessionTabWidget;
}

namespace RZ {
  class ReferenceFrame;
  class OpticalPath;
}

class SimulationSession;
class RZGUIGLWidget;
class SimulationProgressDialog;
class DetectorWindow;
class SourceEditorWindow;
class ColorSettings;
class SpotDiagramWindow;

class SessionTabWidget : public QWidget
{
  Q_OBJECT

  SimulationSession        *m_session;                        // Borrowed
  RZGUIGLWidget            *m_glWidget;                       // Borrowed
  DetectorWindow           *m_detWindow           = nullptr;  // Owned
  SimulationProgressDialog *m_progressDialog      = nullptr;  // Owned
  SourceEditorWindow       *m_sourceEditorWindow  = nullptr;
  const RZ::ReferenceFrame *m_selectedFrame       = nullptr;
  RZ::Element              *m_selectedElement     = nullptr;
  bool                      m_displayNames        = false;
  bool                      m_displayApertures    = false;
  bool                      m_displayElements     = true;
  bool                      m_displayRefFrames    = false;
  bool                      m_displayGrid         = true;
  bool                      m_displayMeasurements = false;

  std::map<std::string, std::list<SurfaceFootprint>> m_footprintQueues;
  std::map<std::string, SpotDiagramWindow *>         m_footprintWindows;

  void connectAll();
  void addGridStep(QString const &, qreal);
  void addGridDiv(unsigned);
  void resetFootprintWindows();
  void reconnectTracer();

  SpotDiagramWindow *openNewFootprintWindow(std::string const &);

public:
  explicit SessionTabWidget(SimulationSession *, QWidget *parent = nullptr);
  ~SessionTabWidget() override;

  SimulationSession *session() const;
  RZGUIGLWidget *glWidget() const;
  void applyColorSettings(ColorSettings const &);

  std::list<std::string> footprints() const;

  void clearBeam();
  void showDetectorWindow();
  void updateDetectorWindow();
  void updateModel();
  void updateFootprintWindows();

  void reloadModel();
  void reloadModelFromEditor();
  void setRotation(qreal, qreal, qreal);
  void showSourceWindow();
  void centerSelectedFrame();

  bool openSpotDiagramWindow(std::string const &);
  bool displayNames() const;
  bool displayApertures() const;
  bool displayElements() const;
  bool displayRefFrames() const;
  bool displayGrid() const;
  bool displayMeasurements() const;
  qreal gridStep() const;
  unsigned int gridDivs() const;

  const RZ::ReferenceFrame *selectedFrame() const;

  void setDisplayNames(bool);
  void setDisplayApertures(bool);
  void setDisplayElements(bool);
  void setDisplayRefFrames(bool);
  void setDisplayGrid(bool);
  void setDisplayMeasurements(bool);

  void setSelectedReferenceFrame(RZ::ReferenceFrame *, const char * = nullptr);
  void setSelectedOpticalPath(const RZ::OpticalPath *);
  void setSelectedElement(RZ::Element *);

  void setGridStep(qreal);
  void setGridDivs(unsigned);

  void keyPressEvent(QKeyEvent *event) override;


signals:
  void simulationResults();

private:
  Ui::SessionTabWidget *ui;

public slots:
  void onModelChanged();
  void onSimulationTriggered(QString, int, int);
  void onSweepFinished();
  void onSourceEditorBuild();
  void onGridStepChanged(int);
  void onGridDivChanged(int);
  void onNewCoords(qreal, qreal);

  void onOpenFootprintWindow();
  void onCloseFootprintWindow();

};

#endif // SESSIONTABWIDGET_H
