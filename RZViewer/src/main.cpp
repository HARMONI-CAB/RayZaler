#include <Recipe.h>
#include <TopLevelModel.h>
#include <GLUTEngine.h>
#include <RZGLModel.h>
#include <ParserContext.h>
#include <libgen.h>
#include <ModelRenderer.h>

using namespace RZ;

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

static TopLevelModel *g_tlModel = nullptr;
static RZGLModel *g_model = nullptr;
static Recipe *g_recipe = nullptr;

void
graceful_cleanup()
{
  if (g_model != nullptr) {
    delete g_model;
    g_model = nullptr;
  }

  if (g_tlModel != nullptr) {
    delete g_tlModel;
    g_tlModel = nullptr;
  }

  if (g_recipe != nullptr) {
    delete g_recipe;
    g_recipe = nullptr;
  }
}

static inline void
fileExplode(std::string &dir, std::string &filename, const char *path)
{
  std::vector<char> alloc;
  size_t strSz = strlen(path) + 1;

  alloc.resize(strSz);

  memcpy(alloc.data(), path, strSz);
  dir = dirname(alloc.data());

  memcpy(alloc.data(), path, strSz);
  filename = basename(alloc.data());
}


int
main(int argc, char **argv)
{
  Recipe *recipe = new Recipe();
  recipe->addDof("t", 0, 0, 1e6);

  g_recipe = recipe;
  atexit(graceful_cleanup);

  if (argc == 1) {
    FileParserContext *ctx = new FileParserContext(recipe);
    ctx->addSearchPath(".");
    ctx->setFile(stdin, "<STDIN>");
    try {
      if (!ctx->parse())
        exit(EXIT_FAILURE);
    } catch (std::runtime_error &e) {
      fprintf(stderr, "%s: failed to parse from stdin\n", argv[0]);
      fprintf(stderr, "%s: error: %s\n", argv[0], e.what());
      exit(EXIT_FAILURE);
    }
    delete ctx;
  } else {
    for (int i = 1; i < argc; ++i) {
      std::string fileName, dirName;

      fileExplode(dirName, fileName, argv[i]);
      FileParserContext *ctx = new FileParserContext(recipe);
      ctx->addSearchPath(dirName);

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

      ctx->setFile(fp, fileName);

      try {
        if (!ctx->parse())
          exit(EXIT_FAILURE);
      } catch (std::runtime_error &e) {
        fprintf(stderr, "%s: failed to parse `%s'\n", argv[0], fileName.c_str());
        fprintf(stderr, "%s: error: %s\n", argv[0], e.what());
        exit(EXIT_FAILURE);
      }
      
      delete ctx;
    }
  }

  try {
    TopLevelModel *tlModel = new TopLevelModel(recipe);
    MyEventListener listener(tlModel);


    // Create OpenGL model
    RZGLModel *model = new RZGLModel();

    g_tlModel = tlModel;
    g_model   = model;

    model->pushOptoMechanicalModel(tlModel);
    model->setEventListener(&listener);

    {
      ModelRenderer *renderer = ModelRenderer::fromOMModel(tlModel, 1024, 768);
      renderer->roll(180);
      renderer->render();
      renderer->savePNG("model.png");
      delete renderer;
    }

    GLUTEngine *engine = GLUTEngine::instance();
    auto defPath = tlModel->lookupOpticalPath();
    if (defPath != nullptr) {
      fprintf(
        stderr, 
        "%s: note: model exposes a default optical path with %d stages\n",
        argv[0],
        defPath->m_sequence.size());
      int i = 0;
      for (auto p : defPath->m_sequence)
        fprintf(stderr, "  %2d. %s\n", ++i, p->frame->name().c_str());
    }

    engine->setModel(model);
    engine->start();
    engine->setModel(nullptr);
  } catch (std::runtime_error const &e) {
    fprintf(stderr, "%s: %s\n", argv[0], e.what());
    exit(EXIT_FAILURE);
  }

  graceful_cleanup();
  
  exit(EXIT_SUCCESS);
}
