#include "LogicControl.h"

LogicControl::LogicControl(QObject* parent)
	: QObject(parent)
{
}

LogicControl::~LogicControl()
{
	m_quit = true;
	m_future.wait();
}

QString LogicControl::getLastError() const
{
	return m_lastError;
}

bool LogicControl::initialize()
{
	bool result = false;
	do
	{
		connect(&m_countdown, &utility::Countdown::trigger, this, &LogicControl::trigger);
		m_ft2 = cv::freetype::createFreeType2();
		wchar_t windir[256] = { 0 };
		RUN_BREAK(!GetWindowsDirectory(windir, 256), "获取系统目录失败");
		m_ft2->loadFontData(std::format("{}\\Fonts\\simhei.ttf", WC_TO_C_STR(windir)), 0);
		RUN_BREAK(!UTIL_JSON, "JSON分配内存失败");
		RUN_BREAK(!UTIL_JSON->initialize(), "JSON初始化失败," + UTIL_JSON->getLastError());;
		m_baseInfo = UTIL_JSON->getBaseInfo();
		m_testInfo = UTIL_JSON->getTestInfo();
		m_enableInfo = UTIL_JSON->getEnableInfo();
		m_userInfo = UTIL_JSON->getUserInfo();
		m_bindInfo = UTIL_JSON->getBindInfo();
		m_channelInfo = UTIL_JSON->getChannelInfo();
		m_countInfo = UTIL_JSON->getCountInfo();
		m_crashDetectionInfo = UTIL_JSON->getCrashDetectionInfo();

		if (m_testInfo->timePrecision == utility::TP_MINUTE) {
			updateText(Q_SPRINTF("时间 %02d:00", m_testInfo->testTime));
		}
		else {
			int min = 0, sec = 0;
			utility::secondCountdown(m_testInfo->testTime, 0, min, sec);
			updateText(Q_SPRINTF("时间 %02d:%02d", min, sec));
		}

		m_countdown.addTask(&m_testInfo->testTime, &m_testInfo->timePrecision);
		m_countdown.addTask(&m_testInfo->cycleInterval, &m_testInfo->timePrecision);

		utility::ScanWidget::Parameters params;
		params.dataHeader = (const QString*)&m_baseInfo->barcodeHeader;
		params.dataLength = (const QString*)&m_baseInfo->barcodeLength;
		params.isJudgeData = (const bool*)&m_enableInfo->judgeCode;
		params.isQueryMes = (const bool*)&m_enableInfo->queryWorkstation;
		params.autoClearDataList = false;
		params.interceptRepeat = true;
		utility::ScanWidget::initialize(params);

		m_rlg = rlg::autoReleaseNew(UTIL_JSON->getDeviceType(), UTIL_JSON->getDeserializerType(), m_baseInfo->systemClock.toDouble());
		m_rlg->setGrabInterval(&m_testInfo->captureInterval);
		m_rlg->enableScaleFirstProcessLater(true);
		m_rlg->setImageScaleSize(cv::Size(720, 480));
		m_rlg->setGrabTimeout(&UTIL_JSON->getTestInfo()->grabTimeout);
		m_rlg->enableSaveErrorFrameLog(m_enableInfo->saveErrorFrameLog);

		connect(m_rlg.get(), &rlg::Base::updateImage, this, &LogicControl::updateImage);
		connect(m_rlg.get(), &rlg::Base::lastError, this, &LogicControl::lastError);
		m_rlg->setImageProc([&](int channel, cv::Mat& mat) { TRY_CATCH(putText(mat, channel);); });

		m_future.add(std::async(std::bind(&LogicControl::onDrawChannel, this)));
		m_future.add(std::async(std::bind(&LogicControl::onScanBarcode, this)));
		m_future.add(std::async(std::bind(&LogicControl::onWriteTestLog, this)));

		result = true;
	} while (false);
	return result;
}

bool LogicControl::openDevice()
{
	bool result = false, success = true;
	do
	{
		if (!m_rlg->getDeviceCount()) {
			addListItem("无可用采集卡设备");
			break;
		}

		if (!ChannelInfo::zeroEnable(m_rlg.get(), m_channelInfo)) {
			addListItem("启用通道总数必须大于0");
			break;
		}

		auto plot = UTIL_JSON->getPlotInfo();
		for (int i = 0; i < m_rlg->getChannelTotal(); ++i) {
			auto filepath = QString("%1/%2.ini").arg(UTIL_JSON->getFilePath()).arg(plot->plotFile[i]);
			QUI_LOG->info("设置通道:%d,配置文件:%s", i, filepath.toStdString().c_str());
			if (!m_rlg->setPreviewFile(i, filepath)) {
				addListItem(m_rlg->getLastError());
				success = false;
				break;
			}
		}

		if (!success) {
			break;
		}

		auto enable = QUI_INI->value("设备绑定校验/启用").toInt();
		if (enable) {
			auto checkNg = false;
			for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
				if (!m_bindInfo[i].enable) {
					continue;
				}

				auto sn = QUI_INI->value(Q_SPRINTF("设备绑定校验/设备%d序列号", i + 1)).toString();
				if (sn != m_bindInfo[i].sn) {
					addListItem(Q_SPRINTF("设备%d序列号校验失败", i + 1));
					checkNg = true;
				}
			}

			if (checkNg) {
				break;
			}
		}

		auto count = 0;
		for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
			if (m_bindInfo[i].enable) {
				if (!m_rlg->open(i, m_bindInfo[i].sn)) {
					addListItem(Q_SPRINTF("打开设备%d失败,请重新绑定", i + 1));
				}
				else {
					addListItem(Q_SPRINTF("打开设备%d成功", i + 1));
				}
				count++;
			}
		}

		if (!count) {
			addListItem("无可用设备,请先绑定并启用设备");
			break;
		}

		m_connect = true;
		m_scan = true;
		result = true;
	} while (false);
	return result;
}

bool LogicControl::closeDevice()
{
	enableButton(ButtonType::BT_PREV, false);
	utility::ScanWidget::setWindow(false);
	for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
		if (m_bindInfo[i].enable) {
			m_rlg->close(i);
			addListItem(Q_SPRINTF("关闭设备%d成功", i + 1));
		}
	}
	setTestStatus(TestStatus::TS_NO);
	m_connect = false;
	m_scan = false;
	return true;
}

void LogicControl::startPreview(bool cycle)
{
	m_preview = true;
	std::thread(&LogicControl::onStartPreview, this, cycle).detach();
}

void LogicControl::stopPreview(bool cycle)
{
	m_preview = false;
	std::thread(&LogicControl::onStopPreview, this, cycle).detach();
}

void LogicControl::startTest()
{
	for (int i = 0; i < m_rlg->getChannelTotal(); ++i) {
		if (m_channelInfo[i].enable) {
			m_channelInfo[i].status = RunStatus::RS_START;
		}
	}

	enableButton(ButtonType::BT_PREV, false);
	setTestStatus(TestStatus::TS_TS);

	m_countdown.start(0);
	m_start = true;
	m_cycleCount = 1;

	addListItem("检测开始");
	addListItem(Q_SPRINTF("循环次数%d次,循环间隔%d分钟", m_testInfo->cycleCount, m_testInfo->cycleInterval));
	addListItem(Q_SPRINTF("第1/%d轮检测开始", m_testInfo->cycleCount));
}

void LogicControl::stopTest(bool* isStop)
{
	if (m_waitStabilizing) {
		qui::MsgBox::warning("警告", "当前状态无法结束检测,请稍后再试!", 10);
		if (isStop) {
			*isStop = false;
		}
		return;
	}

	if (isStop) {
		*isStop = true;
	}

	if (m_countdown.isActive(0) || m_countdown.isActive(1)) {
		if (!qui::MsgBox::question("询问", "当前正在检测中,是否要结束检测?", false, 10)) {
			if (isStop) {
				*isStop = false;
			}
			return;
		}
	}

	for (int i = 0; i < m_rlg->getChannelTotal(); ++i) {
		if (!m_countdown.isActive(0) && !m_countdown.isActive(1)) {
			if (m_channelInfo[i].enable) {
				m_channelInfo[i].status = RunStatus::RS_STOP;
			}
		}
		else {
			if (m_channelInfo[i].enable) {
				m_channelInfo[i].status = RunStatus::RS_ABORT;
				m_channelInfo[i].result = false;
			}
		}
	}

	m_countdown.stop(0);
	m_countdown.stop(1);

	m_start = false;
	m_cycleCount = 1;

	enableButton(ButtonType::BT_PREV, true);
	setTestStatus(ChannelInfo::allPass(m_rlg.get(), m_channelInfo) ? TestStatus::TS_OK : TestStatus::TS_NG);

	addListItem("检测结束");

	if (ChannelInfo::allStatusEqual(m_rlg.get(), m_channelInfo, RunStatus::RS_STOP)) {
		clickButton(ButtonType::BT_TEST);
		clickButton(ButtonType::BT_PREV);
	}
	else {
		clickButton(ButtonType::BT_PREV);
	}

	stopPreview();
	saveLog();
	m_scan = true;
}

void LogicControl::setLastError(const QString& error)
{
	m_lastError = error;
}

void LogicControl::onDrawChannel()
{
	while (!m_quit) {
		for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
			if (m_rlg->isOpen(i)) {
				for (int j = 0; j < m_rlg->getChannelCount(); ++j) {
					int id = m_rlg->getChannelId(i, j);
					if (!m_rlg->isGrabbing(id)) {
						cv::Mat mat(m_rlg->getImageScaleSize(), CV_8UC3, cv::Scalar::all(0));
						putText(mat, id, false);
						m_rlg->updateImage(id, ImageProcess::cvMatToQPixmap(mat));
						Sleep(10);
					}
				}
			}
			else {
				for (int j = 0; j < m_rlg->getChannelCount(); ++j) {
					int id = m_rlg->getChannelId(i, j);
					cv::Mat mat(m_rlg->getImageScaleSize(), CV_8UC3, cv::Scalar::all(0));
					m_rlg->updateImage(id, ImageProcess::cvMatToQPixmap(mat));
					Sleep(10);
				}
			}
		}
		Sleep(100);
	}
}

void LogicControl::onScanBarcode()
{
	while (!m_quit) {
		if (m_connect && m_scan) {
			QThread::msleep(100);
			auto abort = false, clear = true;
			for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
				if (!m_bindInfo[i].enable) {
					continue;
				}

				for (int j = 0; j < m_rlg->getChannelCount(); ++j) {
					auto index = m_rlg->getChannelId(i, j);
					m_detail[index].sn = std::string();
					if (!m_channelInfo[index].enable) {
						continue;
					}

					auto func = [&](QString& err) {
						if (index >= MAX_CHANNEL_COUNT) {
							err = QString("最大仅支持%1个通道").arg(MAX_CHANNEL_COUNT);
							return false;
						}

						if (m_countInfo->channelSum[index] >= m_countInfo->upperlimit) {
							err = QString("通道%1,线束插拔次数上限").arg(index + 1);
							return false;
						}
						return true;
					};

					QString sn;
					if (!utility::ScanWidget::setWindow(true, &sn, Q_SPRINTF("请扫通道%d条码", index + 1), func)) {
						abort = true;
						break;
					}
					m_detail[index].sn = sn.toStdString();

					auto number = UTIL_JSON->getCountInfoValue(q_sprintf("通道%d", index + 1)).toInt() + 1;
					UTIL_JSON->setCountInfoValue(q_sprintf("通道%d", index + 1), QString::number(number));

					if (clear) {
						clearListItem();
						clear = false;
					}
					addListItem(Q_SPRINTF("通道%02d条码:", index + 1) + utility::ScanWidget::getData());
				}

				if (abort) {
					utility::ScanWidget::clearDataList();
					break;
				}
			}

			UTIL_JSON->initialize(true);

			if (!abort) {
				m_scan = false;
				utility::ScanWidget::setWindow(false);
				enableButton(ButtonType::BT_PREV, true);
				addListItem("扫码完毕,请点击预览按钮继续");
			}
		}
		Sleep(100);
	}
}

void LogicControl::onWriteTestLog()
{
	while (!m_quit) {
		if (m_connect) {
			for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
				if (!m_bindInfo[i].enable) {
					continue;
				}

				for (int j = 0; j < m_rlg->getChannelCount(); ++j) {
					auto index = m_rlg->getChannelId(i, j);
					if (m_channelInfo[index].enable && 
						m_channelInfo[index].status != RunStatus::RS_END) {
						writeTestLogFile(index);
					}
				}
			}
		}
		Sleep(1000);
	}
}

void LogicControl::onStartPreview(bool cycle)
{
	m_waitStabilizing = true;
	if (!cycle) {
		enableButton(ButtonType::BT_CONN, false);
		enableButton(ButtonType::BT_PREV, false);
	}

	auto plot = UTIL_JSON->getPlotInfo();
	if (plot->plotWay == 0) {
		addListItem("出图方式:顺序方式");
		for (int i = 0; i < m_rlg->getDeviceCount(); i++) {
			if (!m_bindInfo[i].enable) {
				continue;
			}

			for (int j = 0; j < m_rlg->getChannelCount(); j++) {
				auto id = m_rlg->getChannelId(i, j);
				if (!m_channelInfo[id].enable) {
					if (!(id % 4) && (m_channelInfo[id + 1].enable || m_channelInfo[id + 2].enable || m_channelInfo[id + 3].enable) &&
						m_rlg->getDeserializerType() == rlg::DESERIALIZER_TYPE_NS6603_C2_D8) {
						m_rlg->startGrab(id, rlg::Base::AsyncProc());
						Sleep(m_testInfo->plotInterval);
					}
					continue;
				}

				m_rlg->startGrab(id, [this](const rlg::Base::AsyncArgs& args) {
					//m_channelInfo[args.channel].status = RunStatus::RS_PREVIEW;
					addListItem(Q_SPRINTF("通道%02d启动预览%s,用时%.2fs", args.channel + 1, SU_FA(args.errorCode == 0), (float)args.elapsedTime / 1000));
				});

				if (!cycle) {
					m_channelInfo[id].result = true;
					m_channelInfo[id].count = 0;
					m_channelInfo[id].cycleCount = 0;
				}
				else {
					if (m_enableInfo->ngStatusReset) {
						m_channelInfo[id].result = true;
						m_channelInfo[id].count = 0;
					}
				}
				++m_channelInfo[id].cycleCount;
				m_channelInfo[id].status = RunStatus::RS_PREVIEW;
				Sleep(m_testInfo->plotInterval);
			}
		}
	}
	else {
		//先出奇数通道
		addListItem("出图方式:奇偶方式");
		for (int i = 0; i < m_rlg->getDeviceCount(); i++) {
			if (!m_bindInfo[i].enable) {
				continue;
			}

			for (int j = 0; j < m_rlg->getChannelCount(); j++) {
				auto id = m_rlg->getChannelId(i, j);
				if (!m_channelInfo[id].enable || (id + 1) % 2 == 0) {
					continue;
				}

				m_rlg->startGrab(id, [this](const rlg::Base::AsyncArgs& args) {
					//m_channelInfo[args.channel].status = RunStatus::RS_PREVIEW;
					addListItem(Q_SPRINTF("通道%02d启动预览%s,用时%.2fs", args.channel + 1, SU_FA(args.errorCode == 0), (float)args.elapsedTime / 1000));
				});

				if (!cycle) {
					m_channelInfo[id].result = true;
					m_channelInfo[id].count = 0;
					m_channelInfo[id].cycleCount = 0;
				}
				else {
					if (m_enableInfo->ngStatusReset) {
						m_channelInfo[id].result = true;
						m_channelInfo[id].count = 0;
					}
				}
				++m_channelInfo[id].cycleCount;
				m_channelInfo[id].status = RunStatus::RS_PREVIEW;
			}
		}

		Sleep(m_testInfo->plotInterval);
		
		//再出偶数通道
		for (int i = 0; i < m_rlg->getDeviceCount(); i++) {
			if (!m_bindInfo[i].enable) {
				continue;
			}

			for (int j = 0; j < m_rlg->getChannelCount(); j++) {
				auto id = m_rlg->getChannelId(i, j);
				if (!m_channelInfo[id].enable || (id + 1) % 2 != 0) {
					continue;
				}

				m_rlg->startGrab(id, [this](const rlg::Base::AsyncArgs& args) {
					//m_channelInfo[args.channel].status = RunStatus::RS_PREVIEW;
					addListItem(Q_SPRINTF("通道%02d启动预览%s,用时%.2fs", args.channel + 1, SU_FA(args.errorCode == 0), (float)args.elapsedTime / 1000));
				});

				if (!cycle) {
					m_channelInfo[id].result = true;
					m_channelInfo[id].count = 0;
					m_channelInfo[id].cycleCount = 0;
				}
				else {
					if (m_enableInfo->ngStatusReset) {
						m_channelInfo[id].result = true;
						m_channelInfo[id].count = 0;
					}
				}
				++m_channelInfo[id].cycleCount;
				m_channelInfo[id].status = RunStatus::RS_PREVIEW;
			}
		}
	}

	openTestLogFile(cycle);
	addListItem(Q_SPRINTF("启动预览线程同步%s", SU_FA(m_rlg->waitForStartSync())));
	addListItem("等待设备稳定中...");
	Sleep(m_testInfo->startDelay);
	if (!cycle) {
		addListItem("预览完毕,请点击检测按钮继续");
		enableButton(ButtonType::BT_PREV, true);
		enableButton(ButtonType::BT_TEST, true);
	}
	else {
		//等待启动延时结束,将其赋值为START状态
		for (int i = 0; i < m_rlg->getChannelTotal(); ++i) {
			if (m_channelInfo[i].enable) {
				m_channelInfo[i].status = RunStatus::RS_START;
			}
		}
		addListItem(Q_SPRINTF("第%d/%d轮检测开始", ++m_cycleCount, m_testInfo->cycleCount));
		m_countdown.start(0);
		m_start = true;
	}
	m_waitStabilizing = false;
}

void LogicControl::onStopPreview(bool cycle)
{
	m_waitStabilizing = true;
	if (!cycle) {
		enableButton(ButtonType::BT_PREV, false);
		enableButton(ButtonType::BT_TEST, false);
	}

	for (int i = 0; i < m_rlg->getDeviceCount(); i++) {
		if (!m_bindInfo[i].enable) {
			continue;
		}

		for (int j = 0; j < m_rlg->getChannelCount(); j++) {
			int id = m_rlg->getChannelId(i, j);
			if (!m_channelInfo[id].enable) {
				if (!(id % 4) && (m_channelInfo[id + 1].enable || m_channelInfo[id + 2].enable || m_channelInfo[id + 3].enable) &&
					m_rlg->getDeserializerType() == rlg::DESERIALIZER_TYPE_NS6603_C2_D8) {
					m_rlg->stopGrab(id, nullptr);
				}
				continue;
			}

			m_rlg->stopGrab(id, [this](const rlg::Base::AsyncArgs& args) {
				addListItem(Q_SPRINTF("通道%02d停止预览%s,用时%.2lfs", args.channel + 1, SU_FA(args.errorCode == 0), (float)args.elapsedTime / 1000));
			});

			if (!cycle) {
				if (m_channelInfo[id].status == RunStatus::RS_PREVIEW) {
					m_channelInfo[id].status = RunStatus::RS_BEGIN;
				}
			}
			else {
				m_channelInfo[id].status = RunStatus::RS_STOP;
			}
			m_channelInfo[id].current = 0;
			m_channelInfo[id].srfps = 0;
			m_channelInfo[id].sefps = 0;
			m_channelInfo[id].trfps = 0;
			m_channelInfo[id].tefps = 0;
			m_channelInfo[id].crashAcreage = 0;
		}
	}

	addListItem(Q_SPRINTF("停止预览线程同步%s", SU_FA(m_rlg->waitForStopSync())));
	if (!cycle) {
		enableButton(ButtonType::BT_CONN, true);
		if (ChannelInfo::allStatusEqual(m_rlg.get(), m_channelInfo, RunStatus::RS_BEGIN)) {
			m_scan = true;
		}
		else {
			for (int i = 0; i < m_rlg->getChannelTotal(); ++i) {
				if (m_channelInfo[i].enable) {
					m_channelInfo[i].status = RunStatus::RS_END;
				}
			}
		}
		closeTestLogFile(cycle);
	}
	else {
		addListItem(Q_SPRINTF("第%d/%d轮检测结束", m_cycleCount, m_testInfo->cycleCount));
		addListItem(Q_SPRINTF("等待%d分钟后,进行第%d轮检测", m_testInfo->cycleInterval, m_cycleCount + 1));
		m_countdown.start(1);
	}
	m_waitStabilizing = false;
}

void LogicControl::putText(const cv::Mat& mat, int channelId, bool ok)
{
	const int index = channelId;
	static const int fontFace = cv::FONT_HERSHEY_SIMPLEX;
	static const double fontScale = 2.0;
	const cv::Scalar c0 = m_channelInfo[index].enable ? CV_RGB(0, 255, 0) : CV_RGB(190, 190, 190);
	static const int thickness = 8;
	int step = 1;
	int baseLine0 = 0;
	auto s0 = S_SPRINTF("[%d]", index + 1);
	auto size0 = cv::getTextSize(s0, fontFace, fontScale, thickness, &baseLine0);
	baseLine0 /= 2;

	if (m_enableInfo->showChannel) {
		cv::putText(mat, s0, cv::Point(0, (size0.height + baseLine0) * step++), fontFace, fontScale, c0, thickness);
	}

	if (!m_channelInfo[index].enable) {
		return;
	}

	auto crashOk = true;
	if ((m_channelInfo[index].status == RunStatus::RS_PREVIEW ||
		m_channelInfo[index].status == RunStatus::RS_START) &&
		m_channelInfo[index].timeout(1000)) {
		auto startTime = GetTickCount64();
		m_rlg->getFps(&m_channelInfo[index].srfps, &m_channelInfo[index].sefps, channelId, 0);
		m_rlg->getTotalFrame(&m_channelInfo[index].trfps, &m_channelInfo[index].tefps, channelId, 1);
		m_rlg->getCurrent(&m_channelInfo[index].current, channelId);
		if (m_crashDetectionInfo->enableDetection) {
			crashOk = m_alg.checkImageCrash(mat,
				m_crashDetectionInfo->hsvLower,
				m_crashDetectionInfo->hsvUpper,
				m_crashDetectionInfo->maxArea,
				&m_channelInfo[index].crashAcreage);
		}
		m_channelInfo[index].elapsedTime = GetTickCount64() - startTime;
	}

	if (m_start) {
		if (m_channelInfo[index].result) {
			auto fpsOk = (m_channelInfo[index].srfps > m_testInfo->fps0 &&
				m_channelInfo[index].srfps < m_testInfo->fps1);

			auto currentOk = (m_channelInfo[index].current > m_testInfo->current0 &&
				m_channelInfo[index].current < m_testInfo->current1);

			if (!m_enableInfo->skipTest && (!fpsOk || !currentOk || !crashOk)) {
				if (++m_channelInfo[index].count >= m_testInfo->determineCount) {
					if (!fpsOk) {
						addListItem(Q_SPRINTF("通道%02d帧率异常,%05.2lfFPS", index + 1, m_channelInfo[index].srfps));
					}

					if (!currentOk) {
						addListItem(Q_SPRINTF("通道%02d电流异常,%05.2lfmA", index + 1, m_channelInfo[index].current));
					}

					if (!crashOk) {
						addListItem(Q_SPRINTF("通道%02d存在花屏,%dpx^2", index + 1, m_channelInfo[index].crashAcreage));
					}
					m_channelInfo[index].result = false;
					if (m_enableInfo->saveNgImage) {
						auto image = m_rlg->grabImage(index);
						auto dirpath = QString("%1\\image\\exception\\%2").arg(utility::getCurrentDirectory()).arg(FORMAT_DATE("%04d-%02d-%02d"));
						utility::makePath(dirpath);
						QString sn(m_detail[index].sn.c_str());
						utility::replaceSpecialChar(sn);
						auto utf8 = QString("%1\\%2_%3_%4.png").arg(dirpath).arg(index + 1).arg(sn).arg(FORMAT_TIME("%02d%02d%02d%03d"));
						auto ansi = std::string(utf8.toLocal8Bit().constData());
						auto success = cv::imwrite(ansi, image);
						QUI_LOG->qinfo("保存异常图像%1%2", utf8, SU_FA(success));
					}
				}
			}

			static const cv::Scalar c1 = CV_RGB(0, 255, 0);
			std::string s1 = S_SPRINTF("摄像头单帧:%05.2lf/%05.2lf/s", m_channelInfo[index].srfps, m_channelInfo[index].sefps);
			std::string s2 = S_SPRINTF("摄像头总帧:%05lu/%05lu/s", m_channelInfo[index].trfps, m_channelInfo[index].tefps);
			std::string s3 = S_SPRINTF("摄像头电流:%05.2lfmA", m_channelInfo[index].current);
			//std::string s4 = S_SPRINTF("系统线程:%04X", QThread::currentThreadId());
			std::string s5 = S_SPRINTF("检测耗时:%02d/%02dms", m_rlg->getGrabElapsedTime(channelId), m_channelInfo[index].elapsedTime);
			std::string s6 = S_SPRINTF("线束插拔:%d/%d次", m_countInfo->channelSum[index], m_countInfo->upperlimit);
			std::string s7;
			if (m_crashDetectionInfo->enableDetection) {
				s7 = S_SPRINTF("花屏面积:%dpx^2", m_channelInfo[index].crashAcreage);
			}

			static const int fontHeight = 35;
			if (m_enableInfo->showData) {
				const auto gap = 4;
				m_mutex.lock();
				m_ft2->putText(mat, s1, cv::Point(0, (size0.height + baseLine0 + gap) * step++), fontHeight, c1, -1, 8, 1);
				m_ft2->putText(mat, s2, cv::Point(0, (size0.height + baseLine0 + gap) * step++), fontHeight, c1, -1, 8, 1);
				m_ft2->putText(mat, s3, cv::Point(0, (size0.height + baseLine0 + gap) * step++), fontHeight, c1, -1, 8, 1);
				//m_ft2->putText(mat, s4, cv::Point(0, (size0.height + baseLine0 + gap) * step++), fontHeight, c1, -1, 8, 1);
				m_ft2->putText(mat, s5, cv::Point(0, (size0.height + baseLine0 + gap) * step++), fontHeight, c1, -1, 8, 1);
				m_ft2->putText(mat, s6, cv::Point(0, (size0.height + baseLine0 + gap) * step++), fontHeight, c1, -1, 8, 1);
				if (m_crashDetectionInfo->enableDetection) {
					m_ft2->putText(mat, s7, cv::Point(0, (size0.height + baseLine0 + gap) * step++), fontHeight, c1, -1, 8, 1);
				}
				m_mutex.unlock();
			}
		}
		else {
			static const double fontSize = 12;
			static const int thickness = 12;
			cv::Scalar color = m_channelInfo[index].result ? CV_RGB(0, 255, 0) : CV_RGB(255, 0, 0);
			int baseLine = 0;
			auto size = cv::getTextSize(OK_NG(m_channelInfo[index].result), fontFace, fontSize, thickness, &baseLine);
			cv::Point point = cv::Point(mat.cols / 2 - size.width / 2, mat.rows / 2 + size.height / 2);
			cv::putText(mat, OK_NG(m_channelInfo[index].result), point, fontFace, fontSize, color, thickness);
		}
	}
	else {
		if (m_channelInfo[index].status == RunStatus::RS_END) {
			static const double fontSize = 12;
			static const int thickness = 12;
			cv::Scalar color = m_channelInfo[index].result ? CV_RGB(0, 255, 0) : CV_RGB(255, 0, 0);
			int baseLine = 0;
			auto size = cv::getTextSize(OK_NG(m_channelInfo[index].result), fontFace, fontSize, thickness, &baseLine);
			cv::Point point = cv::Point(mat.cols / 2 - size.width / 2, mat.rows / 2 + size.height / 2);
			cv::putText(mat, OK_NG(m_channelInfo[index].result), point, fontFace, fontSize, color, thickness);
		}
	}
}

bool LogicControl::saveLog()
{
	bool save = true, ok = ChannelInfo::allPass(m_rlg.get(), m_channelInfo);
	if (m_enableInfo->ngSaveRecordHint && !ok) {
		save = qui::MsgBox::question("询问", "检测NG,是否要保存测试记录?");
	}

	QString dirPath = QString("%1\\record\\%2\\%3").arg(utility::getCurrentDirectory(), OK_NG(ok), utility::getCurrentDate(true));
	if (!utility::existPath(dirPath)) {
		utility::makePath(dirPath);
	}

	QString filePath = QString("%1\\%2.csv").arg(dirPath, utility::getCurrentTimeEx(true));
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly)) {
		return false;
	}
	QTextStream stream(&file);
	QString key = "条形码,检测结果,通道号,帧率,电流";
	if (m_crashDetectionInfo->enableDetection) {
		key.append(",花屏轮廓");
	}
	stream << key << endl;

	int disable = 0;
	auto codeList = utility::ScanWidget::getDataList();
	for (int i = 0; i < m_rlg->getChannelTotal(); i++) {
		if (!m_channelInfo[i].enable) {
			++disable;
			continue;
		}

		if (!save && !m_channelInfo[i].result) {
			continue;
		}

		auto offset = i - disable;
		stream << codeList[offset] << ","
			<< OK_NG(m_channelInfo[i].result) << ","
			<< (i + 1) << ","
			<< Q_SPRINTF("%05.2f", m_channelInfo[i].srfps) << ","
			<< Q_SPRINTF("%05.2f", m_channelInfo[i].current);

		if (m_crashDetectionInfo->enableDetection) {
			stream << "," << m_channelInfo[i].crashAcreage;
		}
		stream << endl;
	}
	utility::ScanWidget::clearDataList();
	file.close();
	return true;
}

void LogicControl::openTestLogFile(bool cycle)
{
	if (!cycle && m_enableInfo->saveTestLog) {
		SYSTEMTIME st;
		GetLocalTime(&st);
		for (auto& x : m_detail) {
			x.second.mutex->lock();
			x.second.dirpath = std::format("test/{:02}-{:02}-{:02}", st.wYear, st.wMonth, st.wDay);
			utility::makePath(x.second.dirpath.c_str());
			x.second.filename = std::format("{}/{:02}.log", x.second.dirpath, x.first + 1);
			auto needWriteTitle = false;
			if (!QFileInfo(x.second.filename.c_str()).exists()) {
				needWriteTitle = true;
			}
			x.second.fileptr = fopen(x.second.filename.c_str(), "a");
			if (x.second.fileptr && needWriteTitle) {
				std::string str = "时间,序列号,通道,结果,测试时间,循环次数,循环总数,循环间隔,运行状态,电流,单帧正确帧,单帧错误帧,正确帧总帧,错误帧总帧,花屏面积\n";
				fprintf(x.second.fileptr, utility::utf8ToAnsi(str).c_str());
				fflush(x.second.fileptr);
			}
			x.second.mutex->unlock();
		}
	}
}

void LogicControl::writeTestLogFile(int channel, bool threadSafe)
{
	if (m_enableInfo->saveTestLog) {
		if (threadSafe) {
			m_detail[channel].mutex->lock();
		}

		if (m_detail[channel].fileptr) {
			std::string status;
			switch (m_channelInfo[channel].status)
			{
			case RunStatus::RS_BEGIN:
				status = "BEGIN";
				break;
			case RunStatus::RS_PREVIEW:
				status = "PREVIEW";
				break;
			case RunStatus::RS_START:
				status = "START";
				break;
			case RunStatus::RS_STOP:
				status = "STOP";
				break;
			case RunStatus::RS_ABORT:
				status = "ABORT";
				break;
			case RunStatus::RS_END:
				status = "END";
				break;
			default:
				break;
			}

			fprintf(m_detail[channel].fileptr,
				"%s,%s,%d,%s,%d,%d,%d,%d,%s,%.2lf,%.2lf,%.2lf,%lu,%lu,%d\n",
				FORMAT_TIME("%02d:%02d:%02d.%03d"),
				m_detail[channel].sn.c_str(),
				channel + 1,
				OK_NG(m_channelInfo[channel].result),
				m_testInfo->testTime,
				m_channelInfo[channel].cycleCount,
				m_testInfo->cycleCount,
				m_testInfo->cycleInterval,
				status.c_str(),
				m_channelInfo[channel].current,
				m_channelInfo[channel].srfps,
				m_channelInfo[channel].sefps,
				m_channelInfo[channel].trfps,
				m_channelInfo[channel].tefps,
				m_channelInfo[channel].crashAcreage);
			fflush(m_detail[channel].fileptr);
		}

		if (threadSafe) {
			m_detail[channel].mutex->unlock();
		}
	}
}

void LogicControl::closeTestLogFile(bool cycle)
{
	if (!cycle && m_enableInfo->saveTestLog) {
		for (auto& x : m_detail) {
			x.second.mutex->lock();
			if (x.second.fileptr) {
				writeTestLogFile(x.first, false);
				fclose(x.second.fileptr);
				x.second.fileptr = nullptr;
			}
			x.second.mutex->unlock();
		}
	}
}

void LogicControl::trigger(int id, bool fin, int min, int sec)
{
	updateText(Q_SPRINTF("%s %02d:%02d", id ? "等待" : "时间", min, sec));
	if (id == 0) {
		if (fin) {
			if (m_cycleCount >= m_testInfo->cycleCount) {
				stopTest();
			}
			else {
				m_start = false;
				stopPreview(true);
				setTestStatus(TestStatus::TS_WT);
			}
		}
	}
	else {
		if (fin) {
			startPreview(true);
			setTestStatus(TestStatus::TS_TS);
		}
	}
}

void LogicControl::imageLabelRepreview(int channelId)
{
	static bool lock = false;
	if (lock) {
		return;
	}

	lock = true;
	if (!m_rlg->isOpen(m_rlg->getDeviceId(channelId))) {
		lock = false;
		qui::MsgBox::warning("警告", "请先连接设备!");
		return;
	}

	if (m_channelInfo[channelId].status != RunStatus::RS_PREVIEW) {
		lock = false;
		qui::MsgBox::warning("警告", "未处于预览状态,无法重新预览!");
		return;
	}

	if (!m_rlg->isGrabbing(channelId)) {
		m_rlg->startGrab(channelId, [&](const rlg::Base::AsyncArgs& args) {
			addListItem(Q_SPRINTF("通道%02d启动预览%s,用时%.2fs", args.channel + 1, SU_FA(args.errorCode == 0), (float)args.elapsedTime / 1000));
			lock = false;
		});
	}
	else {
		m_rlg->stopGrab(channelId, [&](const rlg::Base::AsyncArgs& args) {
			addListItem(Q_SPRINTF("通道%02d停止预览%s,用时%.2fs", args.channel + 1, SU_FA(args.errorCode == 0), (float)args.elapsedTime / 1000));
			lock = false;
		});
	}
}

void LogicControl::imageLabelSaveImage(int id)
{
	auto image = m_rlg->grabImage(id);
	auto dirpath = QString("%1\\image\\save\\%2").arg(utility::getCurrentDirectory()).arg(FORMAT_DATE("%04d-%02d-%02d"));
	utility::makePath(dirpath);
	auto utf8 = QString("%1\\%2_%3_%4.bmp").arg(dirpath).arg(id + 1).arg(m_detail[id].sn.c_str()).arg(FORMAT_TIME("%02d%02d%02d%03d"));
	auto ansi = std::string(utf8.toLocal8Bit().constData());
	if (!image.empty()) {
		auto success = cv::imwrite(ansi, image);
		if (success) {
			qui::MsgBox::information("提示", "保存图像成功!");
		}
		else {
			qui::MsgBox::critical("错误", "保存图像失败!");
		}
	}
}
