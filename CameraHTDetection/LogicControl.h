#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include "SettingDialog.h"
#include "QLabelEx.h"
#include "DetectionAlgorithm.h"

class LogicControl : public QObject {
	Q_OBJECT
public:
	/*
	* @brief 构造
	* @param[in] parent 父对象
	*/
	LogicControl(QObject* parent = Q_NULLPTR);

	/*
	* @brief 析构
	* @return void
	*/
	~LogicControl();

	/*
	* @brief 获取最终错误
	* @return QString
	*/
	QString getLastError() const;

	/*
	* @brief 初始化
	* @return bool
	*/
	bool initialize();

	/*
	* @brief 打开设备
	* @return bool
	*/
	bool openDevice();

	/*
	* @brief 关闭设备
	* @return bool
	*/
	bool closeDevice();

	/*
	* @brief 开始预览
	* @param[in] cycle 循环测试中
	* @return void
	*/
	void startPreview(bool cycle = false);

	/*
	* @brief 停止预览
	* @param[in] cycle 循环测试中
	* @return void
	*/
	void stopPreview(bool cycle = false);

	/*
	* @brief 开始测试
	* @return void
	*/
	void startTest();

	/*
	* @brief 停止测试
	* @param[out] isStop 是否停止
	* @return void
	*/
	void stopTest(bool* isStop = nullptr);

	//采集卡类库
	std::shared_ptr<rlg::Base> m_rlg;

protected:
	/*
	* @brief 设置最终错误
	* @param[in] error 最终错误
	* @return void
	*/
	void setLastError(const QString& error);

	/*
	* @brief 画通道
	* @return void
	*/
	void onDrawChannel();

	/*
	* @brief 扫码
	* @return void
	*/
	void onScanBarcode();

	/*
	* @brief 写测试日志
	* @return void
	*/
	void onWriteTestLog();

	/*
	* @brief 开始预览
	* @param[in] cycle 是否在循环测试中
	* @return void
	*/
	void onStartPreview(bool cycle);

	/*
	* @brief 停止预览
	* @param[in] cycle 是否在循环测试中
	* @return void
	*/
	void onStopPreview(bool cycle);

	/*
	* @brief 输出文本
	* @param[in] mat cv::Mat
	* @param[in] channel 通道编号
	* @param[in] ok 是否为正常出画
	* @return void
	*/
	void putText(const cv::Mat& mat, int channel, bool ok = true);

	/*
	* @brief 保存日志
	* @return bool
	*/
	bool saveLog();

	/*
	* @brief 打开测试日志文件
	* @param[in] cycle 是否循环测试中
	* @return void
	*/
	void openTestLogFile(bool cycle);

	/*
	* @brief 写入测试日志文件
	* @param[in] channel 通道
	* @param[in] threadSafe 线程安全
	* @return void
	*/
	void writeTestLogFile(int channel, bool threadSafe = true);

	/*
	* @brief 关闭测试日志文件
	* @param[in] cycle 是否循环测试中
	* @return void
	*/
	void closeTestLogFile(bool cycle);
private:
	//最终错误
	QString m_lastError = "未知错误";

	//图像控件列表
	QList<QLabelEx*> m_imagelList;

	//是否退出
	bool m_quit = false;

	//是否连接
	bool m_connect = false;

	//是否开始
	bool m_start = false;

	//是否扫描
	bool m_scan = false;

	//是否预览
	bool m_preview = false;

	//基本信息
	BaseInfo* m_baseInfo = nullptr;

	//测试信息
	TestInfo* m_testInfo = nullptr;

	//启用信息
	EnableInfo* m_enableInfo = nullptr;

	//用户信息
	UserInfo* m_userInfo = nullptr;

	//绑定信息
	BindInfo* m_bindInfo = nullptr;

	//通道信息
	ChannelInfo* m_channelInfo = nullptr;

	//次数信息
	CountInfo* m_countInfo = nullptr;

	//花屏检测信息
	CrashDetectionInfo* m_crashDetectionInfo = nullptr;

	//循环次数
	int m_cycleCount = 0;

	//输出中文类
	cv::Ptr<cv::freetype::FreeType2> m_ft2;

	//输出中文锁
	std::mutex m_mutex;

	//倒计时
	utility::Countdown m_countdown;

	//异步线程同步锁
	utility::Future<std::future<void>> m_future;

	//检测算法
	DetectionAlgorithm m_alg;

	struct Detail {
		Detail() {
			mutex = new std::mutex;
		}

		~Detail() {
			delete mutex;
		}
		std::string dirpath;
		std::string filename;
		std::string sn;
		std::mutex* mutex = nullptr;
		FILE* fileptr = nullptr;
	};

	//序列号
	std::map<int, Detail> m_detail;

	//等待稳定中
	bool m_waitStabilizing = false;

public slots:
	void trigger(int id, bool fin, int min, int sec);
	void imageLabelRepreview(int id);
	void imageLabelSaveImage(int id);

signals:
	void updateImage(int id, const QPixmap& pixmap);
	void updateText(const QString& text);
	void lastError(int id, int code, const QString& error);
	void enableButton(ButtonType type, bool enable);
	void clickButton(ButtonType type);
	void addListItem(const QString& item);
	void clearListItem();
	void setTestStatus(TestStatus status);
};
