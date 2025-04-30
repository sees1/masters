#include <QtWidgets>

#include <sstream>
#include <string>

class ServerImageBuildBox : public QDialog {
 Q_OBJECT
public:
  ServerImageBuildBox(QWidget* parent = nullptr);

  std::string getUsername();

private slots:
  void saveSettingsLocal();

private:
  QString username_;

  QLabel* lbl_display_;
  QLabel* lbl_conatiner_name_;
  
  QLineEdit* le_container_name_;

  QDialogButtonBox* button_box_;

  QVBoxLayout* common_layout_;
};