#include "client/gui/server_widget.hpp"

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget* parent = nullptr, 
             bool show_splash_screen = true);

  void createMenu();

private slots:
  void helpRequest();

private:
  ServerWidget* server_;

  // container log widget
  QPlainTextEdit* container_log;

  // app central widget
  QWidget* central;

  // main app layout's
  QVBoxLayout* main_layout;
};