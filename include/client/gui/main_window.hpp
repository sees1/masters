// #include "client.hpp"
#include "client/gui/server_widget.hpp"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr, bool show_splash_screen = true)
  : QMainWindow(parent)
    // context_(std::make_shared<boost::asio::io_context>()),
    // resolver_(context_),
  {
    central = new QWidget(this);

    server_ = new ServerWidget();

    createMenu();

    container_stat_table = new QTableWidget(1, 3);

    container_stat_table->setHorizontalHeaderLabels({tr("Running"), tr("Pause"), tr("Stop")});

    container_stat_table->setItem(0, 0, new QTableWidgetItem(QString::number(1)));
    container_stat_table->setItem(0, 1, new QTableWidgetItem(QString::number(2)));
    container_stat_table->setItem(0, 2, new QTableWidgetItem(QString::number(3)));

    container_log = new QPlainTextEdit();
    container_log->setReadOnly(true);

    container_log->appendPlainText("New msgs from server");

    health_check_log = new QPlainTextEdit();
    health_check_log->setReadOnly(true);

    health_check_log->appendPlainText("New msgs from server");

    stat_layout = new QVBoxLayout();
    stat_layout->addWidget(container_stat_table);

    main_layout = new QGridLayout(central);
    main_layout->addWidget(server_, 0, 0);
    main_layout->addLayout(stat_layout, 0, 1);
    main_layout->addWidget(container_log, 1, 0);
    main_layout->addWidget(health_check_log, 1, 1);

    this->setCentralWidget(central);
    // auto endpoints = resolver.resolve(argv[2], argv[3]);
  }

  void createMenu()
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

  // void addNewServer(std::string dns_name, std::string port)
  // {
  //   auto endpoints = resolver.resolve(dns_name.c_str(), port.c_str());

  //   clients.push_back(Client());
  //   clients.back().connect(context_, endpoints);
  // }

private slots:
  void helpRequest() {
    QMessageBox::information(this, "About app", "This is a Qt5 application GUI with a menu.");
  }

private:
  ServerWidget* server_;

  // container stat widget
  QTableWidget* container_stat_table;

  // container log widget
  QPlainTextEdit* container_log;

  // health check widget
  QPlainTextEdit* health_check_log;

  // app central widget
  QWidget* central;

  // main app layout's
  QVBoxLayout* stat_layout;
  QGridLayout* main_layout;
  // std::shared_ptr<boost::asio::io_context> context_;
  // tcp::resolver resolver_;

  // std::vector<Client> clients_;
};