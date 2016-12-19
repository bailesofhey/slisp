#include "Controller.h"

int main(int argc, char **argv) {
  Controller controller(argc, argv);
  controller.Run();
  return controller.ExitCode();
}