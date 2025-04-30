#include <QtWidgets>

#include <string>

class ServerInfoBox : public QDialog {
 Q_OBJECT
public:
  ServerInfoBox(std::string&& label, QWidget* parent = nullptr);

private:
  QLabel* lbl_display_;

  QDialogButtonBox* button_box_;

  QVBoxLayout* common_layout_;
};