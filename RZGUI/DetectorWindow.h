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

#ifndef DETECTORWINDOW_H
#define DETECTORWINDOW_H

#include <QMainWindow>
#include <list>

namespace Ui {
  class DetectorWindow;
}

namespace RZ {
  class Detector;
}

class QResizeEvent;
class ImageNavWidget;
class SimulationSession;
class QFileDialog;
class QLabel;

class DetectorWindow : public QMainWindow
{
  Q_OBJECT

  ImageNavWidget       *m_navWidget;
  RZ::Detector         *m_detector;
  SimulationSession    *m_session = nullptr;
  QFileDialog          *m_saveDialog = nullptr;

  QLabel               *m_pxSizeLabel  = nullptr;
  QLabel               *m_detSizeLabel = nullptr;
  QLabel               *m_pixelsLabel  = nullptr;
  QLabel               *m_rangeLabel   = nullptr;
  QLabel               *m_countsLabel  = nullptr;
  QLabel               *m_posLabel     = nullptr;
  QLabel               *m_pxLabel      = nullptr;
  int                   m_lastX    = 0;
  int                   m_lastY    = 0;

  int                   m_scrollDx = 0;
  int                   m_scrollDy = 0;

  bool                  m_showPhotons = true;
  std::list<QAction *>  m_detectorActions;

  void populateDetectorMenu();
  void populateStatusBar();
  void connectAll();
  void refreshUi();
  void refreshDetectorParams();

public:
  explicit DetectorWindow(QWidget *parent = nullptr);
  ~DetectorWindow() override;

  void setSession(SimulationSession *session);
  void setDetector(RZ::Detector *);
  void refreshImage();

  void closeEvent(QCloseEvent *) override;
  void showEvent(QShowEvent *) override;
  void resizeEvent(QResizeEvent *event) override;

public slots:
  void onScrollBarsChanged();
  void onViewChanged();
  void onClearDetector();
  void onToggleLogScale();
  void onChangeDetector();
  void onHoverPixel(QPointF loc);
  void onChangeDetectorRep();
  void onToggleShowPhase();
  void onToggleGrid();
  void onExport();

private:
  Ui::DetectorWindow *ui;
};

#endif // DETECTORWINDOW_H
