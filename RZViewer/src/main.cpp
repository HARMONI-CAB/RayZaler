#include <Recipe.h>
#include <TopLevelModel.h>
#include <GLUTEngine.h>
#include <RZGLModel.h>
#include <ParserContext.h>

using namespace RZ;

class FileParserContext : public ParserContext {
    using ParserContext::ParserContext;

  public:
    virtual int read() override;
};

int
FileParserContext::read()
{
  return getchar();
}


class MyEventListener : public GLModelEventListener {
    TopLevelModel *m_model = nullptr;
    int m_count = 0;

  public:
    MyEventListener(TopLevelModel *);
    virtual void tick() override;
};

MyEventListener::MyEventListener(TopLevelModel *model)
{
  m_model = model;
}

void
MyEventListener::tick()
{
  m_model->setDof("t", m_count++);
}


int
main(void)
{
  Recipe *recipe = new Recipe();
  recipe->addDof("t", 0, 0, 1e6);


  FileParserContext *ctx = new FileParserContext(recipe);

  yyparse(ctx);

  recipe->debug();

  TopLevelModel *tlModel = new TopLevelModel(recipe);
  MyEventListener listener(tlModel);

  // Create OpenGL model
  RZGLModel *model = new RZGLModel();
  GLUTEngine *engine = GLUTEngine::instance();
  
  model->pushOptoMechanicalModel(tlModel);
  model->setEventListener(&listener);

  engine->setModel(model);
  engine->start();

  // Done
  exit(EXIT_SUCCESS);
}
