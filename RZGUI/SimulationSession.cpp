#include "SimulationSession.h"
#include <ParserContext.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <QFileInfo>

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
