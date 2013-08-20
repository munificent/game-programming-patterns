void processInput() {}
void update() {}
void update(double elapsed) {}
void render() {}
void render(double elapsed) {}

double getCurrentTime() { return 0; }
void sleep(double time) {}

int FPS = 60;
int MS_PER_FRAME = 1000 / FPS;
int MS_PER_TICK = 1000 / FPS;

namespace Repl
{
  char* readCommand() { return NULL; }
  void handleCommand(char* command) {}

  void loop()
  {
    //^1
    while (true)
    {
      char* command = readCommand();
      handleCommand(command);
    }
    //^1
  }
}

namespace EventLoop
{
  class Event {};
  Event* waitForEvent() { return NULL; }
  bool dispatchEvent(Event* event) { return false; }

  void loop()
  {
    //^2
    while (true)
    {
      Event* event = waitForEvent();
      dispatchEvent(event);
    }
    //^2
  }
}

namespace FastAsPossible
{
  void runGame()
  {
    //^3
    while (true)
    {
      processInput();
      update();
      render();
    }
    //^3
  }
}

namespace FixedFramerate
{
  void runGame()
  {
    //^4
    while (true)
    {
      double start = getCurrentTime();
      processInput();
      update();
      render();

      sleep(start + MS_PER_FRAME - getCurrentTime());
    }
    //^4
  }
}


namespace FluidFramerate
{
  void runGame()
  {
    //^5
    double lastTime = getCurrentTime();
    while (true)
    {
      double current = getCurrentTime();
      double elapsed = current - lastTime;
      processInput();
      update(elapsed);
      render();
      lastTime = current;
    }
    //^5
  }
}

namespace FixedUpdateFramerate
{
  void runGame()
  {
    const double MS_PER_UPDATE = 8;

    //^6
    double previous = getCurrentTime();
    double lag = 0.0;
    while (true)
    {
      double current = getCurrentTime();
      double elapsed = current - previous;
      previous = current;
      lag += elapsed;

      processInput();

      while (lag >= MS_PER_UPDATE)
      {
        update();
        lag -= MS_PER_UPDATE;
      }

      render();
    }
    //^6
  }
}


namespace Interpolate
{
  void runGame()
  {
    const double MS_PER_UPDATE = 8;
    double lag = 0;

    //^7
    render(lag / MS_PER_UPDATE);
    //^7
  }
}
