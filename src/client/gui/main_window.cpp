#include "client/gui/main_window.hpp"

MainWindow::MainWindow(QWidget* parent,
                       bool show_splash_screen)
: QMainWindow(parent)
{
  central = new QWidget(this);

  container_log = new QPlainTextEdit();

  server_ = new ServerWidget(container_log);

  createMenu();

  container_log->setReadOnly(true);

  container_log->appendPlainText("New msgs from server");

  main_layout = new QVBoxLayout(central);
  main_layout->addWidget(server_);
  main_layout->addWidget(container_log);

  this->setCentralWidget(central);
  // auto endpoints = resolver.resolve(argv[2], argv[3]);
}

void MainWindow::createMenu()
{
  QAction* logo = new QAction(QIcon(":/images/Android.png"), QString(), this);
  logo->setEnabled(false);

  QAction* add_new_action = new QAction("&Add new", this);
  connect(add_new_action, &QAction::triggered, server_, &ServerWidget::createServer);

  // app logo
  menuBar()->addAction(logo);

  // server tool's menu
  menuBar()->addMenu("&Server")->addAction(add_new_action);
}

void MainWindow::helpRequest() {
  QMessageBox::information(this, "About app", "This is a Qt5 application GUI with a menu.");
}