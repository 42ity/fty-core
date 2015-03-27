#include <tnt/tntnet.h>

int main(int argc, char* argv[])
{
  try
  {
    tnt::Tntnet app;
    app.listen(8000);
    app.mapUrl("^/", "bios_web");
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}
