#include "client/gui/main_window.hpp"

int main(int argc, char* argv[])
{
  QApplication * app = new QApplication(argc, argv);

  MainWindow * main_window = new MainWindow();

  main_window->show();

  return app->exec();
}