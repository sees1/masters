#include "client/gui/boxes/server_image_build_box.hpp"

ServerImageBuildBox::ServerImageBuildBox(QWidget* parent)
: QDialog(parent)
{
  lbl_display_ = new QLabel("Enter container username");
  lbl_display_->setFrameStyle(QFrame::Box | QFrame::Raised);
  lbl_display_->setLineWidth(2);
  lbl_display_->setFixedHeight(50);

  lbl_conatiner_name_ = new QLabel("&Username:");
  le_container_name_ = new QLineEdit();
  lbl_conatiner_name_->setBuddy(le_container_name_);

  button_box_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  common_layout_ = new QVBoxLayout(this);
  common_layout_->addWidget(lbl_display_);
  common_layout_->addWidget(lbl_conatiner_name_);
  common_layout_->addWidget(le_container_name_);
  common_layout_->addWidget(button_box_);

  connect(button_box_, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(button_box_, &QDialogButtonBox::accepted, this, &ServerImageBuildBox::saveSettingsLocal);
  connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

std::string ServerImageBuildBox::getUsername()
{
  return std::string(username_.toUtf8().constData());
}

void ServerImageBuildBox::saveSettingsLocal() {
  username_ = le_container_name_->text();
}