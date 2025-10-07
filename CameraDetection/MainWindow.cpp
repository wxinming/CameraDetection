#include "MainWindow.h"

bool MainWindow::m_dlgExist = false;

MainWindow::MainWindow(const QString& typeName, QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.mainToolBar->setHidden(true);
	REGISTER_META_TYPE(ButtonType);
	REGISTER_META_TYPE(TestStatus);
	utility::window::resize(this, 1.1);
	m_imageLabel.action.repreview = m_imageLabel.menu.addAction("重新预览");
	m_imageLabel.menu.addSeparator();
	m_imageLabel.action.imageMaximum = m_imageLabel.menu.addAction("图像最大化");
	m_imageLabel.menu.addSeparator();
	m_imageLabel.action.saveImage = m_imageLabel.menu.addAction("保存图像");
	connect(m_imageLabel.action.repreview, &QAction::triggered, this, &MainWindow::imageLabelRepreview);
	connect(m_imageLabel.action.imageMaximum, &QAction::triggered, this, &MainWindow::imageLabelImageMaximum);
	connect(m_imageLabel.action.saveImage, &QAction::triggered, this, &MainWindow::imageLabelSaveImage);
	ui.image_0->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.image_0, &QLabel::customContextMenuRequested, this, &MainWindow::imageLabelCustomContextMenuRequested);
	setWindowTitle(QString("摄像头高温检测[%1] 机种[%2] 编译时间[%3]").arg(utility::getVersion(), typeName, COMPILE_DATE_TIME()));
	QUI_LOG->info("软件启动");
}

MainWindow::MainWindow(const QString& typeName, const QString& productCode, QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.mainToolBar->setHidden(true);
	REGISTER_META_TYPE(ButtonType);
	REGISTER_META_TYPE(TestStatus);
	utility::window::resize(this, 1.1);
	m_imageLabel.action.repreview = m_imageLabel.menu.addAction("重新预览");
	m_imageLabel.menu.addSeparator();
	m_imageLabel.action.imageMaximum = m_imageLabel.menu.addAction("图像最大化");
	m_imageLabel.menu.addSeparator();
	m_imageLabel.action.saveImage = m_imageLabel.menu.addAction("保存图像");
	connect(m_imageLabel.action.repreview, &QAction::triggered, this, &MainWindow::imageLabelRepreview);
	connect(m_imageLabel.action.imageMaximum, &QAction::triggered, this, &MainWindow::imageLabelImageMaximum);
	connect(m_imageLabel.action.saveImage, &QAction::triggered, this, &MainWindow::imageLabelSaveImage);
	ui.image_0->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.image_0, &QLabel::customContextMenuRequested, this, &MainWindow::imageLabelCustomContextMenuRequested);
	setWindowTitle(QString("摄像头高温检测[%1] 机种[%2] 产品代码[%3] 编译时间[%4]").arg(utility::getVersion(), typeName, productCode, COMPILE_DATE_TIME()));
	QUI_LOG->info("软件启动");
}

MainWindow::~MainWindow()
{
	QUI_LOG->info("软件退出");
}

QString MainWindow::getLastError() const
{
	return m_lastError;
}

bool MainWindow::initialize()
{
	bool result = false;
	do
	{
		connect(ui.connectButton, &QPushButton::clicked, this, &MainWindow::connectButton);
		connect(ui.exitButton, &QPushButton::clicked, this, &MainWindow::exitButton);
		connect(ui.previewButton, &QPushButton::clicked, this, &MainWindow::previewButton);
		connect(ui.testButton, &QPushButton::clicked, this, &MainWindow::testButton);
		connect(&m_logic, &LogicControl::enableButton, this, &MainWindow::enableButton);
		connect(&m_logic, &LogicControl::clickButton, this, &MainWindow::clickButton);
		connect(&m_logic, &LogicControl::addListItem, this, &MainWindow::addListItem);
		connect(&m_logic, &LogicControl::clearListItem, this, &MainWindow::clearListItem);
		connect(&m_logic, &LogicControl::updateText, this, &MainWindow::updateText);
		connect(&m_logic, &LogicControl::updateImage, this, &MainWindow::updateImage);
		connect(&m_logic, &LogicControl::lastError, this, &MainWindow::lastError);
		connect(&m_logic, &LogicControl::setTestStatus, this, &MainWindow::setTestStatus);
		RUN_BREAK(!m_logic.initialize(), m_logic.getLastError());
		addImageWidget();
		result = true;
	} while (false);
	return result;
}

void MainWindow::maskCloseHint(bool mask)
{
	m_maskCloseHint = mask;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!m_maskCloseHint) {
		if (m_start) {
			qui::MsgBox::warning("警告", "请先停止测试!");
		}

		if (m_connect) {
			qui::MsgBox::warning("警告", "请先断开连接!");
		}

		if (m_start || m_connect) {
			event->ignore();
			return;
		}
	}
	applicationExitSignal();
	event->accept();
	return;
}

void MainWindow::setLastError(const QString& error)
{
	m_lastError = error;
}

void MainWindow::addImageWidget()
{
	m_imageLabel.list.clear();
	int best = m_logic.m_rlg->getBestMatrix(UTIL_JSON->getBindInfoEnableCount());
	QGridLayout* gridLayout = new QGridLayout;
	int row = best, column = best;
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < column; ++j) {
			auto label = new QLabel(this);
			label->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(label, &QLabel::customContextMenuRequested, this, &MainWindow::imageLabelCustomContextMenuRequested);
			label->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);
			label->setObjectName(Q_SPRINTF("label_%d", i * column + j));
			label->setStyleSheet("background-color:rgb(0,0,0);color:rgb(255,255,255);");
			m_imageLabel.list.append(label);
			gridLayout->addWidget(label, i, j);
		}
	}

	auto layout = ui.groupBox->layout();
	if (layout) {
		for (int i = 0; i < layout->count(); ++i) {
			auto item = layout->itemAt(i);
			QLabelEx* label = dynamic_cast<QLabelEx*>(item->widget());
			if (label) {
				layout->removeItem(item);
				delete label;
				--i;
			}
		}
		delete layout;
	}

	ui.groupBox->setContentsMargins(0, 0, 0, 0);
	ui.groupBox->setLayout(gridLayout);
}


void MainWindow::on_actionViewLog_triggered()
{
	qui::LogViewerDialog::doModal("查看日志", QUI_LOG);
}

void MainWindow::on_actionExitApplication_triggered()
{
	close();
}

void MainWindow::on_actionParameterSetting_triggered()
{
	QString pwd;
	if (!qui::InputBox::normal("管理员认证", pwd, "请输入管理员密码", true)) {
		return;
	}

	auto info = UTIL_JSON->getUserInfo();
	if (pwd != info->adminPassword) {
		qui::MsgBox::critical("错误", "管理员密码错误!");
		return;
	}

	auto setting = new SettingDialog(m_logic.m_rlg);
	connect(setting, &SettingDialog::updateText, this, &MainWindow::updateText);
	qui::Widget::doModeless(setting, QUI_WIDGET_WINDOW_LAYOUT);
	setting->initialize();
}

void MainWindow::on_actionAbout_triggered()
{
	qui::AboutDialog::doModal();
}

void MainWindow::on_actionUpdateLog_triggered()
{
	qui::TextViewerDialog::doModal("更新日志", ":/MainDlg/change_logs.txt");
}

void MainWindow::on_actionCheckUpdate_triggered()
{
	qui::UpdateDialog::doModal();
}

void MainWindow::on_actionFeedbackAndSuggestion_triggered()
{
	qui::FeedbackDialog::doModal();
}

void MainWindow::setTestStatus(TestStatus status)
{
	switch (status)
	{
	case TestStatus::TS_NO:
		ui.resultLabel->setText("NO");
		ui.resultLabel->setStyleSheet("background-color:rgb(0,0,0);color:rgb(255,255,255);");
		break;
	case TestStatus::TS_WT:
		ui.resultLabel->setText("WT");
		ui.resultLabel->setStyleSheet("background-color:rgb(30,144,255);color:rgb(0,0,0);");
		break;
	case TestStatus::TS_TS:
		ui.resultLabel->setText("TS");
		ui.resultLabel->setStyleSheet("background-color:rgb(255,255,0);color:rgb(0,0,0);");
		break;
	case TestStatus::TS_OK:
		ui.resultLabel->setText("OK");
		ui.resultLabel->setStyleSheet("background-color:rgb(0,255,0);color:rgb(0,0,0);");
		break;
	case TestStatus::TS_NG:
		ui.resultLabel->setText("NG");
		ui.resultLabel->setStyleSheet("background-color:rgb(255,0,0);color:rgb(0,0,0);");
		break;
	default:break;
	}
	return;
}

void MainWindow::connectButton()
{
	auto task = [this]()->int { return m_connect ? m_logic.closeDevice() : m_logic.openDevice(); };
	if (qui::LoadingDialog::doModal(task)) {
		ui.exitButton->setEnabled(m_connect);
		m_connect = !m_connect;
		ui.connectButton->setText(m_connect ? "断开设备" : "连接设备");
	}
}

void MainWindow::exitButton()
{
	this->close();
}

void MainWindow::previewButton()
{
	m_preview ? m_logic.stopPreview() : m_logic.startPreview();
	ui.previewButton->setText(m_preview ? "开始预览" : "停止预览");
	m_preview = !m_preview;
}

void MainWindow::testButton()
{
	bool isStop = true;
	m_start ? m_logic.stopTest(&isStop) : m_logic.startTest();
	if (isStop) {
		ui.testButton->setText(m_start ? "开始检测" : "结束检测");
		m_start = !m_start;
	}
}

void MainWindow::updateText(const QString& text)
{
	ui.countdownLabel->setText(text);
}

void MainWindow::updateImage(int id, const QPixmap& pixmap)
{
	if (id >= m_imageLabel.list.size()) {
		return;
	}

	if (ui.stackedWidget->currentIndex() == 0) {
		auto size = m_imageLabel.list[id]->size();
		auto pix = pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		m_imageLabel.list[id]->setPixmap(pix);
	}
	else {
		if (m_imageLabel.changePageChannelId == id) {
			auto size = ui.image_0->size();
			auto pix = pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			ui.image_0->setPixmap(pix);
		}
	}
}

void MainWindow::lastError(int id, int code, const QString& error)
{
	auto dt = utility::getCurrentDateTime();
	QString log = Q_SPRINTF("错误代码:0x%x,错误信息:%s", code, error.toStdString().c_str());
	log.insert(0, QTime::currentTime().toString() + "\n");
	m_imageLabel.list[id]->setToolTip(log);
}

void MainWindow::enableButton(ButtonType type, bool enable)
{
	switch (type)
	{
	case ButtonType::BT_EXIT:
		ui.exitButton->setEnabled(enable);
		break;
	case ButtonType::BT_CONN:
		ui.connectButton->setEnabled(enable);
		break;
	case ButtonType::BT_PREV:
		ui.previewButton->setEnabled(enable);
		break;
	case ButtonType::BT_TEST:
		ui.testButton->setEnabled(enable);
		break;
	default:break;

	}
	return;
}

void MainWindow::clickButton(ButtonType type)
{
	if (type == ButtonType::BT_PREV) {
		ui.previewButton->setText(m_preview ? "开始预览" : "停止预览");
		m_preview = !m_preview;
	}
	else if (type == ButtonType::BT_TEST) {
		ui.testButton->setText(m_start ? "开始检测" : "停止检测");
		m_start = !m_start;
	}
	return;
}

void MainWindow::addListItem(const QString& item)
{
	QUI_LOG->qinfo(item);
	ui.listWidget->addItem(utility::getCurrentTime() + QString(" ") + item);
	ui.listWidget->setCurrentRow(ui.listWidget->count() - 1);
}

void MainWindow::clearListItem()
{
	ui.listWidget->clear();
}

void MainWindow::imageLabelCustomContextMenuRequested(const QPoint& point)
{
	auto label = dynamic_cast<QLabel*>(sender());
	if (label != ui.image_0) {
		auto objectName = label->objectName();
		auto channel = objectName.split("_")[1].toInt();
		m_imageLabel.selectChannel = channel;
	}
	m_imageLabel.menu.exec(QCursor::pos());
}

void MainWindow::imageLabelRepreview()
{
	m_logic.imageLabelRepreview(m_imageLabel.selectChannel);
}

void MainWindow::imageLabelImageMaximum()
{
	m_imageLabel.maximumSwitch = !m_imageLabel.maximumSwitch;
	m_imageLabel.action.imageMaximum->setText(m_imageLabel.maximumSwitch ? "图像最小化" : "图像最大化");
	m_imageLabel.changePageChannelId = m_imageLabel.maximumSwitch ? m_imageLabel.selectChannel : -1;
	ui.stackedWidget->setCurrentIndex(static_cast<int>(m_imageLabel.maximumSwitch));
}

void MainWindow::imageLabelSaveImage()
{
	m_logic.imageLabelSaveImage(m_imageLabel.selectChannel);
}
