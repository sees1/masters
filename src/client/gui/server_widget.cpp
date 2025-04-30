#include "client/gui/server_widget.hpp"

ServerWidget::ServerWidget(QPlainTextEdit* cont_log,
                           QWidget* parent)
: QWidget(parent),
  server_counter_(0),
  container_log(cont_log)
{
  first_run_container_fire_ = true;

  client_creator_ = std::make_shared<RoboClientCreator>();

  context_td_ = std::thread([this] { client_creator_->getContext()->run(); });

  main_srv_layout_ = new QVBoxLayout(this);

  wgt_stack = new QStackedWidget(this);

  srv_name_list_wgt = new QListWidget();

  glue_container_name_wgt_ = new QWidget();

  glue_container_name_layout = new QVBoxLayout(glue_container_name_wgt_);

  container_build_table_wgt = new QTableWidget();
  container_name_table_wgt = new QTableWidget();
  container_run_table_wgt = new QTableWidget();

  container_build_table_wgt->setSelectionBehavior(QAbstractItemView::SelectRows);
  container_name_table_wgt->setSelectionBehavior(QAbstractItemView::SelectRows);
  container_run_table_wgt->setSelectionBehavior(QAbstractItemView::SelectRows);

  container_build_table_wgt->setSelectionMode(QAbstractItemView::SingleSelection);
  container_name_table_wgt->setSelectionMode(QAbstractItemView::SingleSelection);
  container_run_table_wgt->setSelectionMode(QAbstractItemView::SingleSelection);

  container_build_table_wgt->setContextMenuPolicy(Qt::CustomContextMenu);
  container_name_table_wgt->setContextMenuPolicy(Qt::CustomContextMenu);
  container_run_table_wgt->setContextMenuPolicy(Qt::CustomContextMenu);

  back_button_ = new QPushButton("X");

  glue_container_name_layout->addWidget(back_button_);
  glue_container_name_layout->addWidget(container_build_table_wgt);
  glue_container_name_layout->addWidget(container_name_table_wgt);
  glue_container_name_layout->addWidget(container_run_table_wgt);

  wgt_stack->addWidget(srv_name_list_wgt);
  wgt_stack->addWidget(glue_container_name_wgt_);

  main_srv_layout_->addWidget(wgt_stack);

  connect(srv_name_list_wgt, &QListWidget::itemClicked, this, &ServerWidget::onServerClicked);
  connect(back_button_, &QPushButton::clicked, this, &ServerWidget::onServerBackClicked);
  connect(container_build_table_wgt, &QTableWidget::customContextMenuRequested, this, &ServerWidget::showContextBuild);
  connect(container_name_table_wgt, &QTableWidget::customContextMenuRequested, this, &ServerWidget::showContextName);
  connect(container_run_table_wgt, &QTableWidget::customContextMenuRequested, this, &ServerWidget::showContextRun);
}

ServerWidget::~ServerWidget()
{
  client_creator_->getWorkGuard()->reset();
  
  if (context_td_.joinable())
    context_td_.join();
}

void ServerWidget::createServer()
{
  ServerCredentalBox* server_cred_ = new ServerCredentalBox();

  if (server_cred_->exec() == QDialog::Accepted)
  {
    wgt_stack->setCurrentIndex(0);

    QString server_name("Server");

    auto [ip, port] = server_cred_->getIpPort();
    server_name += QString::number(server_counter_);

    clients_[server_name] = client_creator_->createRoboClient(ip, port);

    if(!clients_[server_name]->createDDSInstance(server_counter_))
    {
      auto&& erase_it = clients_.find(server_name);
      clients_.erase(erase_it);
    }
    else
    {
      QListWidgetItem* server_item_ = new QListWidgetItem(server_name);
      server_item_->setIcon(QPixmap(":/images/Android.png"));

      srv_name_list_wgt->addItem(server_item_);

      server_counter_++;
    }
  }
}

void ServerWidget::onServerClicked(QListWidgetItem* item)
{
  QString server_name = item->text();

  last_server_name_ = server_name;

  std::shared_ptr<RoboClient> c = clients_[server_name];

  // Получение данных о контейнерах (пример)
  // QList<ContainerInfo> containers = getContainersForServer(serverName);

  std::pair<std::shared_ptr<std::map<std::string, std::string>>,
            std::shared_ptr<docker_cpp::ImageList>> temp_pair = c->getRoboContainersInfo();

  std::shared_ptr<std::map<std::string, std::string>> buildable_img_list = temp_pair.first;
  std::shared_ptr<docker_cpp::ImageList> img_list = temp_pair.second;

  // Очистка таблицы образов которые могут быть собраны
  container_build_table_wgt->clear();
  container_build_table_wgt->setRowCount(buildable_img_list->size());
  container_build_table_wgt->setColumnCount(3); // Пример: Tэг, Имя, Цель

  // Установка заголовков для таблицы контейнеров
  QStringList horizBuildHead = {"TAG", "IMAGE NAME", "TARGET"};
  container_build_table_wgt->setHorizontalHeaderLabels(horizBuildHead);

  int counter = 0;
  std::for_each(buildable_img_list->begin(), buildable_img_list->end(), 
    [this, &counter](auto&& pair)
    {
      size_t delim = pair.first.find(":");
      std::string name = pair.first.substr(0, delim);
      std::string tag = pair.first.substr(delim + 1);
      container_build_table_wgt->setItem(counter, 0, new QTableWidgetItem(QString(tag.c_str())));
      container_build_table_wgt->setItem(counter, 1, new QTableWidgetItem(QString(name.c_str())));
      container_build_table_wgt->setItem(counter, 2, new QTableWidgetItem(QString(pair.second.c_str())));
      counter++;
    }
  );

  // Очистка таблицы контейнеров
  container_name_table_wgt->clear();
  container_name_table_wgt->setRowCount(img_list->size());
  container_name_table_wgt->setColumnCount(4); // Пример: ID, Статус, Имя

  // Установка заголовков для таблицы контейнеров
  QStringList horizHead = {"REPOSITORY", "IMAGE ID", "CREATED", "SIZE"};
  container_name_table_wgt->setHorizontalHeaderLabels(horizHead);

  // Заполнение таблицы контейнеров
  for (size_t i = 0; i < img_list->size(); ++i) {
    std::string repos = (!(*img_list)[i].repoTags.empty()) ? (*img_list)[i].repoTags[0] : "<none>";
    container_name_table_wgt->setItem(i, 0, new QTableWidgetItem(QString(repos.c_str())));
    container_name_table_wgt->setItem(i, 1, new QTableWidgetItem(QString((*img_list)[i].id.c_str())));
    container_name_table_wgt->setItem(i, 2, new QTableWidgetItem(QString((*img_list)[i].created.c_str())));
    container_name_table_wgt->setItem(i, 3, new QTableWidgetItem(QString::number((*img_list)[i].size)));
  }

  std::shared_ptr<docker_cpp::ContainerList> list = c->getExecutedContainers();

  container_run_table_wgt->clear();
  container_run_table_wgt->setRowCount(list->size());
  container_run_table_wgt->setColumnCount(7);

  // Установка заголовков для таблицы запущенных контейнеров
  QStringList horizRunHead = {"CONTAINER ID", "IMAGE", "COMMAND", "CREATED", "STATUS", "PORTS", "NAMES"};
  container_run_table_wgt->setHorizontalHeaderLabels(horizRunHead);

          // Заполнение таблицы запущенных контейнеров
  for (size_t i = 0; i < list->size(); ++i)
  {
    container_run_table_wgt->setItem(i, 0, new QTableWidgetItem(QString((*list)[i].id.c_str())));
    container_run_table_wgt->setItem(i, 1, new QTableWidgetItem(QString((*list)[i].image.c_str())));
    container_run_table_wgt->setItem(i, 2, new QTableWidgetItem(QString((*list)[i].command.c_str())));
    container_run_table_wgt->setItem(i, 3, new QTableWidgetItem(QString((*list)[i].created.c_str())));
    container_run_table_wgt->setItem(i, 4, new QTableWidgetItem(QString((*list)[i].status.c_str())));
    std::string ports;
    if (!(*list)[i].ports.empty())
    {
      for (size_t j = 0; j < (*list)[i].ports.size(); ++j)
      {
        ports += (*list)[i].ports[j].ip.empty() ? "" : (*list)[i].ports[j].ip + " " + 
                  std::to_string(static_cast<int>((*list)[i].ports[j].privatePort))
                  + ":" + 
                  std::to_string(static_cast<int>((*list)[i].ports[j].publicPort)) + "\n";
      }
    }
    container_run_table_wgt->setItem(i, 5, new QTableWidgetItem(QString(ports.c_str())));
    std::string names = (*list)[i].names.empty() ? "" : (*list)[i].names[0];
    container_run_table_wgt->setItem(i, 6, new QTableWidgetItem(QString(names.c_str())));
  }

  wgt_stack->setCurrentIndex(1);
}

void ServerWidget::onServerBackClicked()
{
  wgt_stack->setCurrentIndex(0);
}

void ServerWidget::showContextBuild(const QPoint& pos)
{
  QPoint global_pose = container_build_table_wgt->viewport()->mapToGlobal(pos);
  QTableWidgetItem* item = container_build_table_wgt->itemAt(pos);

  if (!item)
    return;

  int item_row = item->row();

  int column_count = container_build_table_wgt->columnCount();

  int name_info_col = -1;
  int target_info_col = -1;
  for(int col = 0; col < column_count; ++col)
  {
    QTableWidgetItem* header_item = container_build_table_wgt->horizontalHeaderItem(col);
    if(header_item && header_item->text() == "IMAGE NAME")
    {
      name_info_col = col;
    }
    if(header_item && header_item->text() == "TARGET")
    {
      target_info_col = col;
    }
  }

  if (name_info_col == -1 || target_info_col == -1)
    return;
  
  QMenu context_menu;
  QAction* build_action = context_menu.addAction("Создать");

  QAction* selected_action = context_menu.exec(global_pose);
  if (selected_action == build_action)
  {
    std::shared_ptr<RoboClient> c = clients_[last_server_name_];

    ServerImageBuildBox* build_box = new ServerImageBuildBox();

    if ((build_box->exec() == QDialog::Accepted) && c->buildRoboImage(std::string(container_build_table_wgt->item(item_row, name_info_col)->text().toUtf8().constData()),
                                                                      std::string(container_build_table_wgt->item(item_row, target_info_col)->text().toUtf8().constData()),
                                                                      build_box->getUsername()))
    {
      ServerInfoBox* dial = new ServerInfoBox(std::string("Successfuly build image!"));
      dial->exec();

      std::pair<std::shared_ptr<std::map<std::string, std::string>>,
      std::shared_ptr<docker_cpp::ImageList>> temp_pair = c->getRoboContainersInfo();

      std::shared_ptr<docker_cpp::ImageList> img_list = temp_pair.second;

      // Очистка таблицы контейнеров
      container_name_table_wgt->clear();
      container_name_table_wgt->setRowCount(img_list->size());
      container_name_table_wgt->setColumnCount(4); // Пример: ID, Статус, Имя

      // Установка заголовков для таблицы контейнеров
      QStringList horizHead = {"REPOSITORY", "IMAGE ID", "CREATED", "SIZE"};
      container_name_table_wgt->setHorizontalHeaderLabels(horizHead);

      // Заполнение таблицы контейнеров
      for (size_t i = 0; i < img_list->size(); ++i) {
        std::string repos = (!(*img_list)[i].repoTags.empty()) ? (*img_list)[i].repoTags[0] : "<none>";
        container_name_table_wgt->setItem(i, 0, new QTableWidgetItem(QString(repos.c_str())));
        container_name_table_wgt->setItem(i, 1, new QTableWidgetItem(QString((*img_list)[i].id.c_str())));
        container_name_table_wgt->setItem(i, 2, new QTableWidgetItem(QString((*img_list)[i].created.c_str())));
        container_name_table_wgt->setItem(i, 3, new QTableWidgetItem(QString::number((*img_list)[i].size)));
      }
    }
    else
    {
      ServerInfoBox* error = new ServerInfoBox(std::string("Failure to build container"));
      error->exec();
    }
  }
}

void ServerWidget::showContextName(const QPoint& pos)
{
  QPoint global_pose = container_name_table_wgt->viewport()->mapToGlobal(pos);
  QTableWidgetItem* item = container_name_table_wgt->itemAt(pos);

  if (!item)
    return;

  int item_row = item->row();

  int column_count = container_name_table_wgt->columnCount();

  int exe_del_info_col = -1;

  for(int col = 0; col < column_count; ++col)
  {
    QTableWidgetItem* header_item = container_name_table_wgt->horizontalHeaderItem(col);
    if(header_item && header_item->text() == "IMAGE ID")
    {
      exe_del_info_col = col;
      break;
    }
  }

  if (exe_del_info_col == -1)
    return;
  
  QMenu context_menu;
  QAction* exe_action = context_menu.addAction("Запустить");
  QAction* delete_action = context_menu.addAction("Удалить");

  QAction* selected_action = context_menu.exec(global_pose);
  if (selected_action == exe_action)
  {
    std::shared_ptr<RoboClient> c = clients_[last_server_name_];

    ServerContainerCreateBox* cont_name_box = new ServerContainerCreateBox();

    if ((cont_name_box->exec() == QDialog::Accepted) && c->startContainer(std::string(container_name_table_wgt->item(item_row, exe_del_info_col)->text().toUtf8().constData()), cont_name_box->getSettings()))
    {
      std::cout << "Success start container" << std::endl;
      std::shared_ptr<docker_cpp::ContainerList> list = c->getExecutedContainers();

      container_run_table_wgt->clear();
      container_run_table_wgt->setRowCount(list->size());
      container_run_table_wgt->setColumnCount(7);

      // Установка заголовков для таблицы запущенных контейнеров
      QStringList horizRunHead = {"CONTAINER ID", "IMAGE", "COMMAND", "CREATED", "STATUS", "PORTS", "NAMES"};
      container_run_table_wgt->setHorizontalHeaderLabels(horizRunHead);

              // Заполнение таблицы запущенных контейнеров
      for (size_t i = 0; i < list->size(); ++i) {
        container_run_table_wgt->setItem(i, 0, new QTableWidgetItem(QString((*list)[i].id.c_str())));
        container_run_table_wgt->setItem(i, 1, new QTableWidgetItem(QString((*list)[i].image.c_str())));
        container_run_table_wgt->setItem(i, 2, new QTableWidgetItem(QString((*list)[i].command.c_str())));
        container_run_table_wgt->setItem(i, 3, new QTableWidgetItem(QString((*list)[i].created.c_str())));
        container_run_table_wgt->setItem(i, 4, new QTableWidgetItem(QString((*list)[i].status.c_str())));
        std::string ports;
        if (!(*list)[i].ports.empty())
        {
          for (size_t j = 0; j < (*list)[i].ports.size(); ++j)
          {
            ports += (*list)[i].ports[j].ip.empty() ? "" : (*list)[i].ports[j].ip + " " + 
                      std::to_string(static_cast<int>((*list)[i].ports[j].privatePort))
                      + ":" + 
                      std::to_string(static_cast<int>((*list)[i].ports[j].publicPort)) + "\n";
          }
        }
        container_run_table_wgt->setItem(i, 5, new QTableWidgetItem(QString(ports.c_str())));
        std::string names = (*list)[i].names.empty() ? "" : (*list)[i].names[0];
        container_run_table_wgt->setItem(i, 6, new QTableWidgetItem(QString(names.c_str())));
      }
    }
    else
    {
      ServerInfoBox* error = new ServerInfoBox(std::string("Failure to start container!"));
      error->exec();
    }
  }
  else if (selected_action == delete_action)
  {
    std::shared_ptr<RoboClient> c = clients_[last_server_name_];
    if(c->deleteRoboImage(std::string(container_name_table_wgt->item(item_row, exe_del_info_col)->text().toUtf8().constData())))
    {
      ServerInfoBox* del_info = new ServerInfoBox(std::string("Image succesfuly deleted!"));
      del_info->exec();

      std::pair<std::shared_ptr<std::map<std::string, std::string>>,
      std::shared_ptr<docker_cpp::ImageList>> temp_pair = c->getRoboContainersInfo();

      std::shared_ptr<docker_cpp::ImageList> img_list = temp_pair.second;

      // Очистка таблицы контейнеров
      container_name_table_wgt->clear();
      container_name_table_wgt->setRowCount(img_list->size());
      container_name_table_wgt->setColumnCount(4); // Пример: ID, Статус, Имя

      // Установка заголовков для таблицы контейнеров
      QStringList horizHead = {"REPOSITORY", "IMAGE ID", "CREATED", "SIZE"};
      container_name_table_wgt->setHorizontalHeaderLabels(horizHead);

      // Заполнение таблицы контейнеров
      for (size_t i = 0; i < img_list->size(); ++i) {
        std::string repos = (!(*img_list)[i].repoTags.empty()) ? (*img_list)[i].repoTags[0] : "<none>";
        container_name_table_wgt->setItem(i, 0, new QTableWidgetItem(QString(repos.c_str())));
        container_name_table_wgt->setItem(i, 1, new QTableWidgetItem(QString((*img_list)[i].id.c_str())));
        container_name_table_wgt->setItem(i, 2, new QTableWidgetItem(QString((*img_list)[i].created.c_str())));
        container_name_table_wgt->setItem(i, 3, new QTableWidgetItem(QString::number((*img_list)[i].size)));
      }
    }
    else
    {
      ServerInfoBox* del_info = new ServerInfoBox(std::string("Failure to delete image!"));
      del_info->exec();
    }
  }
}

void ServerWidget::showContextRun(const QPoint& pos)
{
  QPoint global_pose = container_run_table_wgt->viewport()->mapToGlobal(pos);
  QTableWidgetItem* item = container_run_table_wgt->itemAt(pos);

  if (!item)
    return;

  int item_row = item->row();

  int column_count = container_run_table_wgt->columnCount();

  int stop_info_col = -1;

  for(int col = 0; col < column_count; ++col)
  {
    QTableWidgetItem* header_item = container_run_table_wgt->horizontalHeaderItem(col);
    if(header_item && header_item->text() == "CONTAINER ID")
    {
      stop_info_col = col;
      break;
    }
  }

  if (stop_info_col == -1)
    return;
  
  QMenu context_menu;
  QAction* stop_action = context_menu.addAction("Остановить");
  QAction* get_log_action = context_menu.addAction("Выгрузить логи");

  QAction* selected_action = context_menu.exec(global_pose);
  if (selected_action == stop_action)
  {
    std::shared_ptr<RoboClient> c = clients_[last_server_name_];

    if (c->stopContainer(std::string(container_run_table_wgt->item(item_row, stop_info_col)->text().toUtf8().constData())))
    {
      ServerInfoBox* stop_info = new ServerInfoBox(std::string("Container succesfuly stoped!"));
      stop_info->exec();

      std::shared_ptr<docker_cpp::ContainerList> list = c->getExecutedContainers();

      container_run_table_wgt->clear();
      container_run_table_wgt->setRowCount(list->size());
      container_run_table_wgt->setColumnCount(7);

      // Установка заголовков для таблицы запущенных контейнеров
      QStringList horizRunHead = {"CONTAINER ID", "IMAGE", "COMMAND", "CREATED", "STATUS", "PORTS", "NAMES"};
      container_run_table_wgt->setHorizontalHeaderLabels(horizRunHead);

              // Заполнение таблицы запущенных контейнеров
      for (size_t i = 0; i < list->size(); ++i) {
        container_run_table_wgt->setItem(i, 0, new QTableWidgetItem(QString((*list)[i].id.c_str())));
        container_run_table_wgt->setItem(i, 1, new QTableWidgetItem(QString((*list)[i].image.c_str())));
        container_run_table_wgt->setItem(i, 2, new QTableWidgetItem(QString((*list)[i].command.c_str())));
        container_run_table_wgt->setItem(i, 3, new QTableWidgetItem(QString((*list)[i].created.c_str())));
        container_run_table_wgt->setItem(i, 4, new QTableWidgetItem(QString((*list)[i].status.c_str())));
        std::string ports;
        if (!(*list)[i].ports.empty())
        {
          for (size_t j = 0; j < (*list)[i].ports.size(); ++j)
          {
            ports += (*list)[i].ports[j].ip.empty() ? "" : (*list)[i].ports[j].ip + " " + 
                      std::to_string(static_cast<int>((*list)[i].ports[j].privatePort))
                      + ":" + 
                      std::to_string(static_cast<int>((*list)[i].ports[j].publicPort)) + "\n";
          }
        }
        container_run_table_wgt->setItem(i, 5, new QTableWidgetItem(QString(ports.c_str())));
        std::string names = (*list)[i].names.empty() ? "" : (*list)[i].names[0];
        container_run_table_wgt->setItem(i, 6, new QTableWidgetItem(QString(names.c_str())));
      }
    }
    else
    {
      ServerInfoBox* stop_info = new ServerInfoBox(std::string("Failure to stop container!"));
      stop_info->exec();
    }
  }
  else if (selected_action == get_log_action)
  {
    std::shared_ptr<RoboClient> c = clients_[last_server_name_];

    std::string result_log;

    if (c->getRoboLog(std::string(container_run_table_wgt->item(item_row, stop_info_col)->text().toUtf8().constData()), result_log))
    {
      ServerInfoBox* log_info = new ServerInfoBox(std::string("Log info succesfuly taken!"));
      log_info->exec();

      if (!result_log.empty())
        container_log->setPlainText(result_log.c_str());
    }
    // else
    // {

    // }
  }
}