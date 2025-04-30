#include "client/gui/boxes/server_info_box.hpp"

ServerInfoBox::ServerInfoBox(std::string&& label, QWidget* parent)
: QDialog(parent)
{
  lbl_display_ = new QLabel(label.c_str());
  lbl_display_->setFrameStyle(QFrame::Box | QFrame::Raised);
  lbl_display_->setLineWidth(2);
  lbl_display_->setFixedHeight(50);

  button_box_ = new QDialogButtonBox(QDialogButtonBox::Ok);

  common_layout_ = new QVBoxLayout(this);
  common_layout_->addWidget(lbl_display_);
  common_layout_->addWidget(button_box_);

  connect(button_box_, &QDialogButtonBox::accepted, this, &QDialog::accept);
}