#include <Recipe.h>
#include <TopLevelModel.h>
#include <GLUTEngine.h>
#include <RZGLModel.h>

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

int
main(void)
{
  Recipe *recipe = new Recipe();

  ////////////////////////////// SCRIPT START /////////////////////////////////
  recipe->addDof("t", 0, 0, 1e6);

  auto bench = recipe->addElement("bench", "BenchElement");
  bench->set("height", "1");

  recipe->pushPort(bench, "surface");
    recipe->pushTranslation("0", "0", "2.5");
      auto detector = recipe->addElement("detector", "Detector");
    recipe->pop();

    recipe->pushTranslation("0", "0", ".5");
      recipe->pushRotation(".4 * cos(t)", "1", "0", "0");
        auto mirror = recipe->addElement("mirror", "ConcaveMirror");
        mirror->set("radius", "0.25");
        printf("Mirror created with index %d\n", mirror->s_index);
      recipe->pop();
    recipe->pop();
  recipe->pop();
  /////////////////////////////// SCRIPT END /////////////////////////////////

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
