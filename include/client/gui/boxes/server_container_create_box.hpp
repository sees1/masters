#include <QtWidgets>

#include <sstream>
#include <string>

#include "client/mcontainer_setting.hpp"

class ServerContainerCreateBox : public QDialog {
 Q_OBJECT
public:
  ServerContainerCreateBox(QWidget* parent = nullptr);

  MContainerSetting getSettings();

private slots:
  void saveSettingsLocal();

  void onTtyChoose(int id);

private:
  QString name_;
  QString launch_;
  QString container_username_;
  QString launch_dir_;
  QString arguments_;
  bool tty_;


  QButtonGroup* butt_group_;
  QRadioButton* enable_tty_;
  QRadioButton* disable_tty_;
  QWidget* button_layout_wgt_;
  QHBoxLayout* button_layout_;

  QLabel* lbl_display_;
  QLabel* lbl_conatiner_name_;
  QLabel* lbl_launch_dir_name_;
  QLabel* lbl_container_username_;
  QLabel* lbl_cont_launch_;
  QLabel* lbl_command_args_;
  QLabel* lbl_cont_tty_;
  
  QLineEdit* le_container_name_;
  QLineEdit* le_launch_dir_name_;
  QLineEdit* le_container_username_;
  QLineEdit* le_command_args_;
  QLineEdit* le_cont_launch_;

  QDialogButtonBox* button_box_;

  QVBoxLayout* common_layout_;
};