#include "client/gui/boxes/server_credental_box.hpp"

ServerCredentalBox::ServerCredentalBox(QWidget* parent)
: QDialog(parent)
{
  lbl_display_ = new QLabel("Enter server credental");
  lbl_display_->setFrameStyle(QFrame::Box | QFrame::Raised);
  lbl_display_->setLineWidth(2);
  lbl_display_->setFixedHeight(50);

  lbl_server_ip_ = new QLabel("&Server ip:");
  le_server_ip_ = new QLineEdit();
  lbl_server_ip_->setBuddy(le_server_ip_);

  lbl_port_ = new QLabel("&Server port:");
  le_port_ = new QLineEdit();
  lbl_port_->setBuddy(le_port_);

  button_box_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  common_layout_ = new QVBoxLayout(this);
  common_layout_->addWidget(lbl_display_);
  common_layout_->addWidget(lbl_server_ip_);
  common_layout_->addWidget(le_server_ip_);
  common_layout_->addWidget(lbl_port_);
  common_layout_->addWidget(le_port_);
  common_layout_->addWidget(button_box_);

  connect(button_box_, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(button_box_, &QDialogButtonBox::accepted, this, &ServerCredentalBox::saveCredLocal);
  connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

std::pair<std::string, std::string> ServerCredentalBox::getIpPort()
{
  return std::make_pair(std::string(ip_.toUtf8().constData()), std::string(port_.toUtf8().constData()));
}

void ServerCredentalBox::saveCredLocal() {
  ip_ = le_server_ip_->text();
  port_ = le_port_->text();
}