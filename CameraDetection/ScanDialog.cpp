#include "ScanDialog.h"

ScanDialog::ScanDialog(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ScanDialog::~ScanDialog()
{

}

void ScanDialog::on_enterButton_clicked()
{
	do
	{
		if (!MainWindow::m_dlgExist) {
			auto dirpath = qApp->applicationDirPath() + "/config";
			utility::makeDir(dirpath);

			auto productCode = ui.lineEdit->text();
			auto typeName = QUI_INI->value(QString("产品代码/%1").arg(productCode)).toString();
			if (typeName.isEmpty()) {
				qui::MsgBox::warning("提示", QString("产品代码%1不存在!").arg(productCode));
				break;
			}

			dirpath = dirpath + "\\" + typeName;

			if (!utility::existDir(dirpath)) {
				qui::MsgBox::warning("提示", QString("产品代码所对应的机种%1不存在,请先添加!").arg(typeName));
				break;
			}

			UTIL_JSON->setFilePath(dirpath);
			UTIL_JSON->setTypeName(typeName);

			MainWindow* window = new MainWindow(typeName, productCode);
			window->maskCloseHint(true);
			if (!window->initialize()) {
				qui::MsgBox::warning("错误", window->getLastError());
				window->close();
				break;
			}
			window->maskCloseHint(false);
			qui::Widget::doModeless(window, QUI_MAIN_WINDOW_LAYOUT);

			auto qui = qui::Widget::findSelf(this);
			qui->hide();
			connect(window, &MainWindow::applicationExitSignal, this, [qui]() { qui->show(); });
		}

	} while (false);
	ui.lineEdit->clear();
	return;
}

void ScanDialog::on_exitButton_clicked()
{
	close();
}

void ScanDialog::on_lineEdit_returnPressed()
{
	on_enterButton_clicked();
}
