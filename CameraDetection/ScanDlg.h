#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <Utility/Manage/Window.h>
#include <Utility/Manage/VarPtr.hpp>
#include "ui_ScanDlg.h"

#ifndef MES_NETWORK_COMM
	//自身标题
enum SelfTitle
{
	//TvsA56ScanCode.INVO.R&D
	ST_SD_HD_ECU_DVR_LINE,

	//CameraLineA.INVO.R&D
	ST_SD_CAM_AUTO_LINE,

	//HDCameraAUTO.INVO.SC
	ST_HD_CAM_AUTO_LINE
};

//查询类型
enum QueryType
{
	//硬件测试
	QT_HARDWARE_TEST,

	//老化测试
	QT_AGING_TEST,

	//功能测试
	QT_FUNCTION_TEST,

	//摄像头测试
	QT_CAMERA_TEST
};

//查询结果
enum QueryResult
{
	//测试OK
	QR_OK,

	//测试NG
	QR_NG,

	//上站NG
	QR_PRE_NG,

	//上站未作
	QR_PRE_NONE,

	//本站NG
	QR_CUR_NG,

	//本站OK
	QR_CUR_OK
};
#endif

//扫码对话框
class ScanDlg : public QWidget
{
	Q_OBJECT
private:
	/*
	* @brief 构造
	* @param[in] parent 父对象
	*/
	ScanDlg(QWidget* parent = Q_NULLPTR);

	/*
	* @brief 析构
	*/
	~ScanDlg();

	/*
	* @brief 拷贝构造删除
	*/
	ScanDlg(const ScanDlg&) = delete;

	/*
	* @brief 赋值构造删除
	*/
	ScanDlg& operator=(const ScanDlg&) = delete;
public:
	/*
	* @brief,扫描参数
	*/
	struct Parameters
	{
		Parameters() :
			dataContent("NULL"),
			dataLength("0"),
			showText("请扫条码"),
			judgeData(false),
			queryData(false),
			interceptRepeat(false),
			autoClearDataList(true),
#ifdef MES_NETWORK_COMM
			onMessage(nullptr),
			onConnection(nullptr)

#else
			selfTitle(ST_HD_CAM_AUTO_LINE),
			queryType(QT_HARDWARE_TEST),
			enumTitle("数采客户端.for.RT")
#endif
		{

		}

		Utility::VarPtr<QString> dataContent;
		Utility::VarPtr<QString> dataLength;
		Utility::VarPtr<bool> judgeData;
		Utility::VarPtr<bool> queryData;
		Utility::VarPtr<bool> interceptRepeat;
		Utility::VarPtr<bool> autoClearDataList;
		QString showText;
#ifdef MES_NETWORK_COMM
		std::function<void(bool connected)> onConnection;
		std::function<void(const QString& msg)> onMessage;
		QString pluginSavePath;
#else
		SelfTitle selfTitle;
		QueryType queryType;
		QString enumTitle;
#endif
	};

	/*
	* @brief 设置参数
	* @param[in] params 参数
	* @return bool
	*/
	static bool initialize(const Parameters& params);

	/*
	* @brief 获取实例
	* @return ScanDlg*
	*/
	static ScanDlg* getInstance();

	/*
	* @brief 删除实例
	* @return void
	*/
	static void deleteInstance();

	/*
	* @brief 设置扫描对话框窗口
	* @param[in] show 是否显示
	* @param[out] data 扫描内容
	* @param[in] text 显示文本
	* @return bool
	*/
	static bool setWindow(bool show, QString* data = nullptr, const QString& text = QString());

	/*
	* @brief 数据是否重复
	* @param[in] data 数据
	* @return bool
	*/
	static bool dataIsRepeat(const QString& data);

	/*
	* @brief 获取数据列表
	* @return QStringList
	*/
	static QStringList getDataList();

	/*
	* @brief 获取数据
	* @return QString
	*/
	static QString getData();

	/*
	* @brief 清空数据列表
	* @return void
	*/
	static void clearDataList();

	/*
	* @brief 是否显示
	* @return bool
	*/
	[[nodiscard]] static bool isShow();

	/*
	* @brief 是否循环
	* @return bool
	*/
	[[nodiscard]] static bool isLoop();
protected:
#ifndef MES_NETWORK_COMM
	/*
	* @brief,设置工站类型
	* @param1,类型
	* @return,void
	*/
	void setQueryType(QueryType type);

	/*
	* @brief,设置自身标题
	* @param1,标题
	* @return,void
	*/
	void setSelfTitle(const QString& title);

	/*
	* @brief,设置枚举标题
	* @param1,标题
	* @return,void
	*/
	void setEnumTitle(const QString& title);
#endif

	/*
	* @brief 设置显示文本
	* @param[in] text 文本
	* @return void
	* @notice 此接口只能在主线程调用,如果在子线程将会导致程序崩溃
	*/
	void setShowText(const QString& text);

	/*
	* @brief 设置条码判断
	* @param[in] context 判断信息
	* @return void
	*/
	void setDataContext(const QString& context);

	/*
	* @brief 设置条码判断
	* @param[in] context 判断信息
	* @return void
	*/
	void setDataContext(const QString* context);

	/*
	* @brief 设置条码长度
	* @param[in] length 条码长度
	* @return void
	*/
	void setDataLength(const QString& length);

	/*
	* @brief 设置条码长度
	* @param[in] length 条码长度
	* @return void
	*/
	void setDataLength(const QString* length);

	/*
	* @brief 启用判断条码
	* @param[in] enable 是否跳过
	* @return void
	*/
	void enableJudgeData(bool enable);

	/*
	* @brief 启用判断条码
	* @param[in] enable 是否跳过
	* @return void
	*/
	void enableJudgeData(const bool* enable);

	/*
	* @brief 启用查询工站
	* @param[in] enable 是否跳过
	* @return void
	*/
	void enableQueryData(bool enable);

	/*
	* @brief 启用查询工站
	* @param[in] enable 是否跳过
	* @return void
	*/
	void enableQueryData(const bool* enable);

	/*
	* @brief 启用自动拦截重复数据
	* @param[in] enable 是否拦截
	* @return void
	*/
	void enableAutoInterceptRepeatData(bool enable = true);

	/*
	* @brief 启用自动拦截重复数据
	* @param[in] enable 是否拦截
	* @return void
	*/
	void enableAutoInterceptRepeatData(const bool* enable);

	/*
	* @brief 启用自动清空条码列表
	* @notice 如果设置了true,setAutoInterceptRepeatCode将失效
	* @param[in] clear 是否清空
	* @return void
	*/
	void enableAutoClearDataList(bool clear = true);

protected:
	/*
	* @brief 重写鼠标事件
	*/
	OVERRIDE_MOUSE_EVENT;

	/*
	* @brief 设置最终错误
	* @param[in] error 错误
	* @return void
	*/
	void setLastError(const QString& error);

	/*
	* @brief 获取最终错误
	* @return QString
	*/
	QString getLastError() const;

	/*
	* @brief 判断条码
	* @return void
	*/
	bool judgeData();

	/*
	* @brief 发送条码
	* @return void
	*/
	bool sendData();

	/*
	* @brief 重写事件过滤器
	* @param[in] obj 对象
	* @param[in] event 事件
	* @return bool
	*/
	virtual bool eventFilter(QObject* obj, QEvent* event);

	/*
	* @brief 本机事件
	* @param[in] eventType 事件类型
	* @param[in] message 消息
	* @param[in] result 结果
	* @return bool
	*/
	virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result);

signals:
	/*
	* @brief 显示窗口信号
	* @param[out] code 条码
	* @param[in] text 显示文本
	* @return bool
	*/
	bool showWindowSignal(QString* code, const QString& text);

	/*
	* @brief 隐藏窗口信号
	* @return void
	*/
	void hideWindowSignal();

	/*
	* @brief 只隐藏窗口信号
	* @return void
	*/
	void onlyHideWindowSignal();

	/*
	* @brief 设置标题信息
	* @param[in] title 标题
	* @return void
	*/
	void setTitleSignal(const QString& title);

private slots:
	/*
	* @brief 回车按下槽
	* @return void
	*/
	void returnPressedSlot();

	/*
	* @brief 显示窗口槽
	* @param[in] code 条码数据
	* @param[in] text 显示文本
	* @return bool
	* @notice 此接口必须在子线程中调用,否则会卡死主线程
	*/
	bool showWindowSlot(QString* code, const QString& text);

	/*
	* @brief 隐藏窗口槽
	* @return void
	* @notice 此接口可以在子线程调用,也可以在主线程中调用
	*/
	void hideWindowSlot();

	/*
	* @brief 只隐藏窗口槽
	* @return void
	*/
	void onlyHideWindowSlot();

	/*
	* @brief 设置标题槽
	* @param[in] title 标题
	* @return void
	*/
	void setTitleSlot(const QString& title);
private:
	//界面类
	Ui::ScanDlg ui;

	//枚举窗口句柄
	HWND m_hwnd = nullptr;

	//最小化控件
	QLabel m_label;

	//条码内容
	QString m_content = "NULL";
	const QString* m_contentPtr = &m_content;

	//条码长度
	QString m_length = "0";
	const QString* m_lengthPtr = &m_length;

	//判断条码
	bool m_judgeData = false;
	const bool* m_judgeDataPtr = &m_judgeData;

	//查询工站
	bool m_queryData = false;
	const bool* m_queryDataPtr = &m_queryData;

	//自身
	static ScanDlg* m_self;

#ifndef MES_NETWORK_COMM
	//工站类型
	QueryType m_type = QT_HARDWARE_TEST;

	//枚举标题
	QString m_enumTitle = "数采客户端.for.RT";
#else
	//网络通讯调用invo_mes_plugin.dll
	static void* m_plugin;

	//插件连接回调
	std::function<void(bool connected)> onConnection = nullptr;

	//插件接收消息回调(上传数据)
	std::function<void(const QString& msg)> onMessage = nullptr;
#endif

	//自动拦截
	bool m_intercept = true;
	const bool* m_interceptPtr = &m_intercept;

	//自动清空
	bool m_clear = false;

	//数据
	QString m_data;

	//显示文本
	QString m_text = "请扫条码";

	//事件循环
	QEventLoop m_loop;

	//终止
	bool m_abort = false;

	//条码列表
	QStringList m_dataList;

	//最终错误
	QString m_lastError = "未知错误";

	//初始化
	static bool m_initialize;
};
