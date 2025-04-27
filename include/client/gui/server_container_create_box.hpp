#include <QtWidgets>

#include <sstream>
#include <string>

#include "client/mcontainer_setting.hpp"

class ServerContainerCreateBox : public QDialog {
 Q_OBJECT
public:
  ServerContainerCreateBox(QWidget* parent = nullptr)
  : QDialog(parent)
  {
    lbl_display_ = new QLabel("Enter container settings");
    lbl_display_->setFrameStyle(QFrame::Box | QFrame::Raised);
    lbl_display_->setLineWidth(2);
    lbl_display_->setFixedHeight(50);

    lbl_conatiner_name_ = new QLabel("&Container name:");
    le_container_name_ = new QLineEdit();
    lbl_conatiner_name_->setBuddy(le_container_name_);

    lbl_cont_command_ = new QLabel("&Container command:");
    le_cont_command_ = new QLineEdit();
    lbl_cont_command_->setBuddy(le_cont_command_);

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
    common_layout_->addWidget(lbl_cont_command_);
    common_layout_->addWidget(le_cont_command_);
    common_layout_->addWidget(lbl_cont_tty_);
    common_layout_->addWidget(button_layout_wgt_);
    common_layout_->addWidget(button_box_);

    connect(button_box_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box_, &QDialogButtonBox::accepted, this, &ServerContainerCreateBox::saveSettingsLocal);
    connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::reject);
  }

  MContainerSetting getSettings()
  {
    std::vector<std::string> tokens;
    std::stringstream ss(std::string(command_.toUtf8().constData()));
    std::string token;
    while (std::getline(ss, token, ' ')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return {std::string(name_.toUtf8().constData()), tokens, tty_};
  }

private slots:
  void saveSettingsLocal() {
    name_ = le_container_name_->text();
    command_ = le_cont_command_->text(); 
  }

  void onTtyChoose(int id) {
    tty_ = (id == 1);
  }

private:
  QString name_;
  QString command_;
  bool tty_;

  QButtonGroup* butt_group_;
  QRadioButton *enable_tty_;
  QRadioButton *disable_tty_;
  QWidget* button_layout_wgt_;
  QHBoxLayout* button_layout_;

  QLabel* lbl_display_;
  QLabel* lbl_conatiner_name_;
  QLabel* lbl_cont_command_;
  QLabel* lbl_cont_tty_;
  
  QLineEdit* le_container_name_;
  QLineEdit* le_cont_command_;

  QDialogButtonBox* button_box_;

  QVBoxLayout* common_layout_;
};