#ifndef SIMULATIONSESSION_H
#define SIMULATIONSESSION_H

#include <QObject>
#include <Recipe.h>
#include <TopLevelModel.h>

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

class FileParserContext;

class SimulationSession : public QObject
{
  Q_OBJECT

  QString            m_path;
  QString            m_fileName;
  FileParserContext *m_context       = nullptr;
  RZ::Recipe        *m_recipe        = nullptr;
  RZ::TopLevelModel *m_topLevelModel = nullptr;

public:
  explicit SimulationSession(QString const &path, QObject *parent = nullptr);
  virtual ~SimulationSession() override;

  RZ::Recipe        *recipe() const;
  RZ::TopLevelModel *topLevelModel() const;
  QString            fileName() const;

signals:

};

#endif // SIMULATIONSESSION_H
