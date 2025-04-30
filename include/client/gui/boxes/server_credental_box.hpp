#include <QtWidgets>

#include <string>

class ServerCredentalBox : public QDialog {
 Q_OBJECT
public:
  ServerCredentalBox(QWidget* parent = nullptr);

  std::pair<std::string, std::string> getIpPort();

private slots:
  void saveCredLocal();

private:
  QString ip_;
  QString port_;

  QLabel* lbl_display_;
  QLabel* lbl_server_ip_;
  QLabel* lbl_port_;

  QLineEdit* le_server_ip_;
  QLineEdit* le_port_;

  QDialogButtonBox* button_box_;

  QVBoxLayout* common_layout_;
};