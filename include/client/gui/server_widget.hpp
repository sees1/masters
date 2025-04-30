#include <QMainWindow>
#include <QSplashScreen>

#include <QtGui/QPixmap>
#include <QMessageBox>

#include <QtWidgets>

#include <map>

#include "client/gui/boxes/server_credental_box.hpp"
#include "client/gui/boxes/server_container_create_box.hpp"
#include "client/gui/boxes/server_info_box.hpp"
#include "client/gui/boxes/server_image_build_box.hpp"

#include "client/client.hpp"

class ServerWidget : public QWidget
{
  Q_OBJECT

public:
  ServerWidget(QPlainTextEdit* cont_log,
               QWidget* parent = nullptr);

  ~ServerWidget();

public slots:
  void createServer();
  void onServerClicked(QListWidgetItem* item);
  void onServerBackClicked();

  // Context Menu action's
  void showContextBuild(const QPoint& pos);
  void showContextName(const QPoint& pos);
  void showContextRun(const QPoint& pos);

private:
  int server_counter_;
  QString last_server_name_;

  bool first_run_container_fire_;

  std::thread context_td_;

  std::shared_ptr<RoboClientCreator> client_creator_;
  std::map<QString, std::shared_ptr<RoboClient>> clients_;

  // container log widget
  QPlainTextEdit* container_log;

  // back to server list button
  QPushButton* back_button_;

  // container's info on server
  QWidget* glue_container_name_wgt_;
  QTableWidget* container_build_table_wgt;
  QTableWidget* container_name_table_wgt;
  QTableWidget* container_run_table_wgt;
  QVBoxLayout* glue_container_name_layout;

  // server layout
  QVBoxLayout* main_srv_layout_;

  // list of server's added in map
  QWidget* glue_srv_name_wgt;
  QVBoxLayout* glue_srv_name_layout;
  QListWidget* srv_name_list_wgt;

  // swap widget
  QStackedWidget* wgt_stack;
};