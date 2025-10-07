#include "ScanDlg.h"

ScanDlg* ScanDlg::m_self = nullptr;

bool ScanDlg::m_initialize = false;

#ifdef MES_NETWORK_COMM
void* ScanDlg::m_plugin = nullptr;
#endif

ScanDlg::ScanDlg(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	installEventFilter(this);
	QFont font;
	font.setBold(true);
	font.setPixelSize(20);
	m_label.setParent(ui.titleLabel);
	m_label.setText("-");
	m_label.setFont(font);
	m_label.setAlignment(Qt::AlignCenter);
	m_label.setFixedSize(QSize(25, 25));
	m_label.setStyleSheet("color:rgb(0,0,0);background-color:transparent;");
	m_label.move(size().width() - m_label.size().width(), 0);
	m_label.installEventFilter(this);
	connect(ui.codeLine, &QLineEdit::returnPressed, this, &ScanDlg::returnPressedSlot);
	connect(this, &ScanDlg::showWindowSignal, this, &ScanDlg::showWindowSlot, Qt::BlockingQueuedConnection);
	connect(this, &ScanDlg::hideWindowSignal, this, &ScanDlg::hideWindowSlot);
	connect(this, &ScanDlg::onlyHideWindowSignal, this, &ScanDlg::onlyHideWindowSlot);
	connect(this, &ScanDlg::setTitleSignal, this, &ScanDlg::setTitleSlot);
}

ScanDlg::~ScanDlg()
{

}

extern DWORD g_gui_tid;

bool ScanDlg::initialize(const Parameters& params)
{
	if (m_initialize)
	{
		return true;
	}

	Q_ASSERT_X(g_gui_tid == GetCurrentThreadId(), __FUNCTION__, "必须在GUI线程进行初始化");

	if (params.dataContent.isNull())
		getInstance()->setDataContext(params.dataContent.var());
	else
		getInstance()->setDataContext(params.dataContent.ptr());

	if (params.dataLength.isNull())
		getInstance()->setDataLength(params.dataLength.var());
	else
		getInstance()->setDataLength(params.dataLength.ptr());

	if (params.judgeData.isNull())
		getInstance()->enableJudgeData(params.judgeData.var());
	else
		getInstance()->enableJudgeData(params.judgeData.ptr());

	if (params.queryData.isNull())
		getInstance()->enableQueryData(params.queryData.var());
	else
		getInstance()->enableQueryData(params.queryData.ptr());

	if (params.interceptRepeat.isNull())
		getInstance()->enableAutoInterceptRepeatData(params.interceptRepeat.var());
	else
		getInstance()->enableAutoInterceptRepeatData(params.interceptRepeat.ptr());

	if (params.autoClearDataList.isNull())
		getInstance()->enableAutoClearDataList(params.autoClearDataList.var());
	else
		getInstance()->enableAutoClearDataList(params.autoClearDataList.ptr());

	getInstance()->setShowText(params.showText);

#ifndef MES_NETWORK_COMM
	QString title = "HDCameraAUTO.INVO.SC";
	switch (params.selfTitle)
	{
	case ST_SD_HD_ECU_DVR_LINE:
		title = "TvsA56ScanCode.INVO.R&D";
		break;
	case ST_SD_CAM_AUTO_LINE:
		title = "CameraLineA.INVO.R&D";
		break;
	case ST_HD_CAM_AUTO_LINE:
		title = "HDCameraAUTO.INVO.SC";
		break;
	default:
		break;
	}

	getInstance()->setSelfTitle(title);

	getInstance()->setEnumTitle(params.enumTitle);

	getInstance()->setQueryType(params.queryType);
	m_initialize = true;
#else
	getInstance()->onConnection = params.onConnection;

	getInstance()->onMessage = params.onMessage;

	m_plugin = invo_mes_plugin_new();
	if (!params.pluginSavePath.isEmpty())
	{
		QByteArray&& byte = params.pluginSavePath.toLocal8Bit();
		invo_mes_plugin_set_ini_save_path(m_plugin, byte.data());
	}
	invo_mes_plugin_initialize(m_plugin);
	invo_mes_plugin_connection_cb(m_plugin, [](bool connected, void* user) {
		ScanDlg* dlg = static_cast<ScanDlg*>(user);
		if (dlg->onConnection)
		{
			dlg->onConnection(connected);
		}
	}, getInstance());

	invo_mes_plugin_recv_message_cb(m_plugin, [](const invo_mes_plugin_message_t* msg, void* user) {
		ScanDlg* dlg = static_cast<ScanDlg*>(user);
		if (msg->response_code == query_data_message_response)
		{
			if (msg->error_code == message_response_ok)
			{
				dlg->m_dataList.append(dlg->m_data);
				dlg->onlyHideWindowSignal();
				dlg->m_loop.quit();
			}
			else
			{
				dlg->setTitleSignal(QString::fromLocal8Bit(msg->error_string));
			}
		}

		if (dlg->onMessage)
		{
			if (msg->response_code == upload_data_message_response)
			{
				dlg->onMessage(QString::fromLocal8Bit(msg->error_string));
			}
		}
	}, getInstance());

	m_initialize = invo_mes_plugin_start(m_plugin);
#endif
	return m_initialize;
}

ScanDlg* ScanDlg::getInstance()
{
	if (!m_self)
	{
		m_self = new ScanDlg;
	}
	return m_self;
}

void ScanDlg::deleteInstance()
{
	if (m_self)
	{
#ifdef MES_NETWORK_COMM
		invo_mes_plugin_connection_cb(m_plugin, nullptr, nullptr);
		invo_mes_plugin_recv_message_cb(m_plugin, nullptr, nullptr);
		invo_mes_plugin_free(m_plugin);
#endif
		m_initialize = false;
		delete m_self;
		m_self = nullptr;
	}
}

bool ScanDlg::setWindow(bool show, QString* data, const QString& text)
{
	Q_ASSERT_X(m_initialize, __FUNCTION__, "未调用ScanDlg::initialize");
	bool result = false;
	if (show)
	{
		Q_ASSERT_X(g_gui_tid != GetCurrentThreadId(), __FUNCTION__, "必须在子线程中调用show");
		result = emit ScanDlg::getInstance()->showWindowSignal(data, text);
	}
	else
	{
		emit ScanDlg::getInstance()->hideWindowSignal();
		result = true;
	}
	return result;
}

bool ScanDlg::dataIsRepeat(const QString& data)
{
	return getInstance()->m_dataList.contains(data);
}

QStringList ScanDlg::getDataList()
{
	return getInstance()->m_dataList;
}

QString ScanDlg::getData()
{
	if (getInstance()->m_dataList.isEmpty())
		return getInstance()->m_data;
	return getInstance()->m_dataList.last();
}

void ScanDlg::clearDataList()
{
	getInstance()->m_dataList.clear();
}

bool ScanDlg::isShow()
{
	return !getInstance()->isHidden();
}

bool ScanDlg::isLoop()
{
	return getInstance()->m_loop.isRunning();
}

#ifndef MES_NETWORK_COMM
void ScanDlg::setQueryType(QueryType type)
{
	m_type = type;
}

void ScanDlg::setSelfTitle(const QString& title)
{
	setWindowTitle(title);
}

void ScanDlg::setEnumTitle(const QString& title)
{
	m_enumTitle = title;
}
#endif

void ScanDlg::setShowText(const QString& text)
{
	m_text = text;
}

void ScanDlg::setDataContext(const QString& content)
{
	m_content = content;
}

void ScanDlg::setDataContext(const QString* content)
{
	if (content)
		m_contentPtr = content;
	else
		m_contentPtr = &m_content;
}

void ScanDlg::setDataLength(const QString& length)
{
	m_length = length;
}

void ScanDlg::setDataLength(const QString* length)
{
	if (length)
		m_lengthPtr = length;
	else
		m_lengthPtr = &m_length;
}

void ScanDlg::enableJudgeData(bool enable)
{
	m_judgeData = enable;
}

void ScanDlg::enableJudgeData(const bool* enable)
{
	if (enable)
		m_judgeDataPtr = enable;
	else
		m_judgeDataPtr = &m_judgeData;
}

void ScanDlg::enableQueryData(bool enable)
{
	m_queryData = enable;
}

void ScanDlg::enableQueryData(const bool* enable)
{
	if (enable)
		m_queryDataPtr = enable;
	else
		m_queryDataPtr = &m_queryData;
}

void ScanDlg::enableAutoInterceptRepeatData(bool enable)
{
	m_intercept = enable;
}

void ScanDlg::enableAutoInterceptRepeatData(const bool* enable)
{
	if (enable)
		m_interceptPtr = enable;
	else
		m_interceptPtr = &m_intercept;
}

void ScanDlg::enableAutoClearDataList(bool enable)
{
	m_clear = enable;
}

void ScanDlg::returnPressedSlot()
{
	m_data = ui.codeLine->text();

	if (!judgeData() || !sendData())
	{
		ui.titleLabel->setText(getLastError());
		goto clear;
	}
clear:
	if (m_clear)
		clearDataList();
	ui.codeLine->clear();
}

void ScanDlg::setLastError(const QString& error)
{
	m_lastError = error;
}

QString ScanDlg::getLastError() const
{
	return m_lastError;
}

bool ScanDlg::judgeData()
{
	bool result = false;
	do
	{
		if (*m_contentPtr == "NULL" || !m_lengthPtr->toInt() || !*m_judgeDataPtr)
		{
			result = true;
			break;
		}
		else
		{
			if (!m_clear && *m_interceptPtr && m_dataList.contains(m_data))
			{
				int index = 0;
				for (int i = 0; i < m_dataList.size(); i++)
				{
					if (m_dataList[i] == m_data)
					{
						index = i + 1;
						break;
					}
				}
				setLastError(QString().sprintf("条码%d与条码%d内容重复", m_dataList.size() + 1, index));
				break;
			}

			if (m_data.length() != m_lengthPtr->toInt() || m_data.left(m_contentPtr->length()) != *m_contentPtr)
			{
				setLastError("条码格式错误");
				break;
			}
		}
		result = true;
	} while (false);
	return result;
}

bool ScanDlg::sendData()
{
	bool result = false;
	do
	{
		if (!*m_queryDataPtr)
		{
			if (m_data.isEmpty())
			{
				m_dataList.append(QString().sprintf("未知条码-%d", m_dataList.size()));
			}
			else
			{
				m_dataList.append(m_data);
			}
			hide();
			m_loop.quit();
			result = true;
			break;
		}

#ifdef MES_NETWORK_COMM
		QByteArray&& byte = m_data.toLocal8Bit();
		if (invo_mes_plugin_send_message(m_plugin, byte.data(), byte.length()) < 0)
		{
			setLastError("未与MES客户端连接");
			break;
		}
#else
		EnumWindows([](HWND hwnd, LPARAM param)->int {
			wchar_t name[MAX_PATH] = { 0 };
			GetWindowTextW(hwnd, name, MAX_PATH);
			auto dlg = reinterpret_cast<ScanDlg*>(param);
			auto title = dlg->m_enumTitle.toStdWString();
			if (!wcsncmp(name, title.c_str(), title.length()))
			{
				dlg->m_hwnd = hwnd;
				return false;
			}
			return true;
			}, reinterpret_cast<LPARAM>(this));

		if (!m_hwnd)
		{
			setLastError("发送到采集端失败");
			break;
		}

		COPYDATASTRUCT cds = { 0 };
		cds.dwData = m_type;
		cds.cbData = m_data.length() + 1;
		QByteArray byteArray = m_data.toLatin1();
		char* byte = byteArray.data();
		cds.lpData = byte;
		HWND sender = reinterpret_cast<HWND>(effectiveWinId());
		::SendMessage(m_hwnd, WM_COPYDATA, reinterpret_cast<WPARAM>(sender), reinterpret_cast<LPARAM>(&cds));
#endif
		result = true;
	} while (false);
	return result;
}

bool ScanDlg::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == &m_label)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			auto mouseEvent = reinterpret_cast<QMouseEvent*>(event);
			if (mouseEvent->button() == Qt::LeftButton)
			{
				showMinimized();
				return true;
			}
		}
	}

	if (event->type() == QEvent::KeyPress)
	{
		if (reinterpret_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
		{
			return true;
		}
	}
	return QObject::eventFilter(obj, event);
}

bool ScanDlg::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
#ifndef MES_NETWORK_COMM
	MSG* msg = static_cast<MSG*>(message);
	if (msg->message == WM_COPYDATA)
	{
		auto cds = reinterpret_cast<COPYDATASTRUCT*>(msg->lParam);
		if (cds->dwData == QR_OK)
		{
			m_dataList.append(m_data);
			hide();
			m_loop.quit();
			//if (onHide)
			//{
			//	onHide();
			//}
		}
		else
		{
			if (cds->dwData == QR_CUR_NG)
			{
				ui.titleLabel->setText("本站NG");
			}
			else if (cds->dwData == QR_CUR_OK)
			{
				ui.titleLabel->setText("本站OK");
			}
			else if (cds->dwData == QR_PRE_NG)
			{
				ui.titleLabel->setText("上站NG");
			}
			else if (cds->dwData == QR_PRE_NONE)
			{
				ui.titleLabel->setText("上站未做");
			}
			else if (cds->dwData == QR_NG)
			{
				ui.titleLabel->setText("NG,已废弃的故障码");
			}
			else
			{
				ui.titleLabel->setText("NG,未知的原因");
			}
		}
		return true;
	}
#endif
	return QWidget::nativeEvent(eventType, message, result);
}

bool ScanDlg::showWindowSlot(QString* code, const QString& text)
{
	ui.titleLabel->setText(!text.isEmpty() ? text : m_text);

	ui.codeLine->clear();

	if (!isActiveWindow())
	{
		activateWindow();
	}

	if (isMinimized())
	{
		showNormal();
	}
	else
	{
		show();
	}
	m_loop.exec();
	if (m_abort)
	{
		m_abort = false;
		return false;
	}

	if (code)
	{
		*code = m_data;
	}
	return true;
}

void ScanDlg::hideWindowSlot()
{
	if (m_loop.isRunning())
	{
		m_abort = true;
	}
	hide();
	m_loop.quit();
}

void ScanDlg::onlyHideWindowSlot()
{
	hide();
}

void ScanDlg::setTitleSlot(const QString& title)
{
	ui.titleLabel->setText(title);
}
