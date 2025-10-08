#include "ChooseDialog.h"

ChooseDialog::ChooseDialog(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	if (!initialize()) {
		qui::MsgBox::warning("错误", getLastError());
	}
}

ChooseDialog::~ChooseDialog()
{

}

QString ChooseDialog::getLastError() const
{
	return m_lastError;
}

bool ChooseDialog::initialize()
{
	bool result = false;
	do 
	{
		m_dirName = utility::getCurrentDirectory() + "\\config";
		addDirctoryList();

		if (!utility::existDir(m_dirName)) {
			utility::makeDir(m_dirName);
		}

		m_recordName = m_dirName + "\\record.json";

		QJsonObject root, record;
		if (!utility::file::exist(m_recordName))
		{
			record.insert("机种", "");
			root.insert("记录", record);
			utility::file::writeJson(m_recordName, root);
		}

		utility::file::readJson(m_recordName, root);
		m_recordObj = root.value("记录").toObject();
		QString name = m_recordObj.value("机种").toString();
		if (!name.isEmpty())
		{
			ui.nameListCombo->setCurrentText(name);
		}

		utility::file::setHidden(m_recordName);
		result = true;
	} while (false);
	return result;
}

void ChooseDialog::setLastError(const QString& error)
{
	m_lastError = error;
}

void ChooseDialog::addDirctoryList()
{
	ui.nameListCombo->clear();

	m_dirList.clear();
	utility::getDirectoryList(m_dirName, m_dirList, false);

	for (auto& x : m_dirList)
	{
		ui.nameListCombo->addItem(QIcon(":/MainDlg/Resources/images/chrome.ico"), QFileInfo(x).baseName());
	}
}

bool ChooseDialog::checkInput()
{
	return false;
}

void ChooseDialog::on_addButton_clicked()
{
	do
	{
		const QString typeName = ui.destNameLine->text();
		if (typeName.length() > 12)
		{
			qui::MsgBox::warning("提示", "机种名长度不可超过12个字符");
			break;
		}

		if (typeName.isEmpty())
		{
			qui::MsgBox::warning("提示", "机种名不可为空");
			break;
		}

		m_pathName = m_dirName + "\\" + typeName;

		if (m_dirList.contains(m_pathName))
		{
			qui::MsgBox::warning("提示", "机种已存在,请勿重复添加");
			break;
		}

		if (!utility::existPath(m_pathName))
		{
			utility::makePath(m_pathName);
		}

		UTIL_JSON->setFilePath(m_pathName);

		if (!UTIL_JSON->initialize())
		{
			qui::MsgBox::warning("错误", "添加机种失败," + UTIL_JSON->getLastError());
			break;
		}

		utility::file::write(m_pathName + "\\sensor.ini", QString());

		addDirctoryList();

		qui::MsgBox::information("提示", "添加机种成功");
	} while (false);
	return;
}

void ChooseDialog::on_deleteButton_clicked()
{
	do
	{
		qui::MsgBox::information("提示", "此路不通,请联系管理员删除");
		return;

		if (!ui.nameListCombo->count())
		{
			qui::MsgBox::warning("提示", "机种为空,无法进行删除");
			break;
		}

		QString moduleName = ui.nameListCombo->currentText();
		QString fileName = QString("%1\\%2.json").arg(m_dirName, moduleName);
		auto result = qui::MsgBox::question("询问", QString("你确定要删除机种[%1]吗?").arg(moduleName));
		if (!result) {
			break;
		}
		utility::file::remove(fileName);
		addDirctoryList();
	} while (false);
	return;
}

void ChooseDialog::on_confirmButton_clicked()
{
	do 
	{
		if (!ui.nameListCombo->count())
		{
			qui::MsgBox::warning("提示", "机种为空,请先添加机种");
			break;
		}

		if (!MainWindow::m_dlgExist)
		{
			QString currentName = ui.nameListCombo->currentText();
			m_pathName = m_dirName + "\\" + currentName;

			m_recordObj.insert("机种", currentName);
			QJsonObject root;
			root.insert("记录", m_recordObj);
			utility::file::writeJson(m_recordName, root);

			UTIL_JSON->setFilePath(m_pathName);
			UTIL_JSON->setTypeName(currentName);

			MainWindow* window = new MainWindow(currentName);
			window->maskCloseHint(true);
			if (!window->initialize()) {
				qui::MsgBox::warning("错误", window->getLastError());
				window->close();
				break;
			}
			window->maskCloseHint(false);
			qui::Widget::doModeless(window, QUI_MAIN_WINDOW_LAYOUT);

			auto qui = qui::Widget::findSelf(this);
			if (qui) {
				qui->hide();
				connect(window, &MainWindow::applicationExitSignal, this, [qui]() { qui->show(); });
			}
		}
	} while (false);
	return;
}

void ChooseDialog::on_exitButton_clicked()
{
	this->close();
}
