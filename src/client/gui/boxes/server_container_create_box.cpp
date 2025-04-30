#include "client/gui/boxes/server_container_create_box.hpp"

ServerContainerCreateBox::ServerContainerCreateBox(QWidget* parent)
: QDialog(parent)
{
  lbl_display_ = new QLabel("Enter container settings");
  lbl_display_->setFrameStyle(QFrame::Box | QFrame::Raised);
  lbl_display_->setLineWidth(2);
  lbl_display_->setFixedHeight(50);

  lbl_conatiner_name_ = new QLabel("&Container name:");
  le_container_name_ = new QLineEdit();
  lbl_conatiner_name_->setBuddy(le_container_name_);

  lbl_launch_dir_name_ = new QLabel("&Container launch directory:");
  le_launch_dir_name_ = new QLineEdit();
  lbl_launch_dir_name_->setBuddy(le_launch_dir_name_);

  lbl_container_username_ = new QLabel("&Container username:");
  le_container_username_ = new QLineEdit();
  lbl_container_username_->setBuddy(le_container_username_);

  lbl_cont_launch_ = new QLabel("&Container launch:");
  le_cont_launch_ = new QLineEdit();
  lbl_cont_launch_->setBuddy(le_cont_launch_);

  lbl_command_args_ = new QLabel("&Command arguments:");
  le_command_args_ = new QLineEdit();
  lbl_command_args_->setBuddy(le_command_args_);

  lbl_cont_tty_ = new QLabel("&Tty:");

  button_layout_wgt_ = new QWidget();
  button_layout_ = new QHBoxLayout(button_layout_wgt_);
  enable_tty_ = new QRadioButton("Enable");
  disable_tty_ = new QRadioButton("Disable");

  button_layout_->addWidget(enable_tty_);
  button_layout_->addWidget(disable_tty_);

  enable_tty_->setChecked(true);

  butt_group_ = new QButtonGroup();
  butt_group_->addButton(enable_tty_, 1);
  butt_group_->addButton(disable_tty_, 0);
  butt_group_->setExclusive(true);

  connect(butt_group_, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ServerContainerCreateBox::onTtyChoose);

  button_box_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  common_layout_ = new QVBoxLayout(this);
  common_layout_->addWidget(lbl_display_);
  common_layout_->addWidget(lbl_conatiner_name_);
  common_layout_->addWidget(le_container_name_);
  common_layout_->addWidget(lbl_launch_dir_name_);
  common_layout_->addWidget(le_launch_dir_name_);
  common_layout_->addWidget(lbl_container_username_);
  common_layout_->addWidget(le_container_username_);
  common_layout_->addWidget(lbl_cont_launch_);
  common_layout_->addWidget(le_cont_launch_);
  common_layout_->addWidget(lbl_command_args_);
  common_layout_->addWidget(le_command_args_);
  common_layout_->addWidget(lbl_cont_tty_);
  common_layout_->addWidget(button_layout_wgt_);
  common_layout_->addWidget(button_box_);

  connect(button_box_, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(button_box_, &QDialogButtonBox::accepted, this, &ServerContainerCreateBox::saveSettingsLocal);
  connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

MContainerSetting ServerContainerCreateBox::getSettings()
{
  return {std::string(container_username_.toUtf8().constData()),
          std::string(name_.toUtf8().constData()),
          std::string(launch_dir_.toUtf8().constData()),
          std::string(launch_.toUtf8().constData()),
          std::string(arguments_.toUtf8().constData()),
          tty_};
}

void ServerContainerCreateBox::saveSettingsLocal() {
  name_ = le_container_name_->text();
  launch_ = le_cont_launch_->text(); 
  container_username_ = le_container_username_->text();
  launch_dir_ = le_launch_dir_name_->text();
  arguments_ = le_command_args_->text();
}

void ServerContainerCreateBox::onTtyChoose(int id) {
  tty_ = (id == 1);
}