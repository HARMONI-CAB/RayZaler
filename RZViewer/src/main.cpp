#include <Recipe.h>
#include <TopLevelModel.h>
#include <GLUTEngine.h>
#include <RZGLModel.h>
#include <ParserContext.h>

using namespace RZ;

class FileParserContext : public ParserContext {
    using ParserContext::ParserContext;
    FILE *fp = stdin;

  public:
    void setFile(FILE *fp, std::string const &name);
    virtual int read() override;
};

void
FileParserContext::setFile(FILE *fp, std::string const &name)
{
  this->fp = fp;
  ParserContext::setFile(name);
}

int
FileParserContext::read()
{
  return fgetc(fp);
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
main(int argc, char **argv)
{
  Recipe *recipe = new Recipe();
  recipe->addDof("t", 0, 0, 1e6);

  if (argc == 1) {
    FileParserContext *ctx = new FileParserContext(recipe);
    ctx->setFile(stdin, "<STDIN>");
    if (!ctx->parse())
      exit(EXIT_FAILURE);
  } else {
    for (int i = 1; i < argc; ++i) {
      FileParserContext *ctx = new FileParserContext(recipe);
      FILE *fp = fopen(argv[i], "r");
      
      if (fp == nullptr) {
        fprintf(
          stderr,
          "%s: cannot open %s: %s\n",
          argv[0],
          argv[1],
          strerror(errno));
        exit(EXIT_FAILURE);
      }

      ctx->setFile(fp, argv[i]);
      if (!ctx->parse())
        exit(EXIT_FAILURE);

      fclose(fp);
    }
  }

  try {
    TopLevelModel *tlModel = new TopLevelModel(recipe);
    MyEventListener listener(tlModel);

    // Create OpenGL model
    RZGLModel *model = new RZGLModel();
    GLUTEngine *engine = GLUTEngine::instance();
    
    model->pushOptoMechanicalModel(tlModel);
    model->setEventListener(&listener);

    engine->setModel(model);
    engine->start();
  } catch (std::runtime_error const &e) {
    fprintf(stderr, "%s: %s\n", argv[0], e.what());
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
