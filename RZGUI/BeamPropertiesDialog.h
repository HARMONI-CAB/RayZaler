#ifndef BEAMPROPERTIESDIALOG_H
#define BEAMPROPERTIESDIALOG_H

#include <QDialog>
#include "SimulationProperties.h"

namespace Ui {
  class BeamPropertiesDialog;
}

class ColorChooserButton;
class SimulationSession;

class BeamPropertiesDialog : public QDialog
{
  Q_OBJECT

  SimulationBeamProperties m_properties;
  SimulationSession       *m_session = nullptr;
  ColorChooserButton      *m_colorChooser = nullptr;

  void connectAll();
  void parseProperties();
  void refreshUi();
  void refreshUiState();

public:
  explicit BeamPropertiesDialog(SimulationSession *, QWidget *parent = nullptr);
  virtual ~BeamPropertiesDialog() override;

  void setNameHint(QString const &);
  void setColorHint(QColor const &);
  void setSession(SimulationSession *);

  void setBeamProperties(SimulationBeamProperties const &);
  SimulationBeamProperties getProperties();
  void highlightFaultyField(QString const &);

public slots:
  void onBrowse();
  void onEditDirectionCosines();
  void onExprEditChanged();
  void onDataChanged();

private:
  Ui::BeamPropertiesDialog *ui;
};

#endif // BEAMPROPERTIESDIALOG_H
