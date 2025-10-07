#pragma once
#pragma execution_character_set("utf-8")

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "SettingDialog.h"
#include "QLabelEx.h"
#include "LogicControl.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(const QString& typeName, QWidget* parent = Q_NULLPTR);
	MainWindow(const QString& typeName, const QString& productCode, QWidget* parent = Q_NULLPTR);
    ~MainWindow();
    QString getLastError() const;
    bool initialize();
	void maskCloseHint(bool mask);

public:
    static bool m_dlgExist;

protected:
    void closeEvent(QCloseEvent* event) override;
    void setLastError(const QString& error);
	void addImageWidget();

private:
	Ui::MainWindowClass ui;
	QString m_lastError = "未知错误";
    bool m_connect = false;
    bool m_preview = false;
    bool m_start = false;
    bool m_maskCloseHint = false;
    LogicControl m_logic;
    struct {
		QList<QLabel*> list;
		QMenu menu;
        struct {
			QAction* repreview;
			QAction* imageMaximum;
			QAction* saveImage;
        } action;
		int changePageChannelId = -1;
		int selectChannel = -1;
		bool maximumSwitch = false;
    } m_imageLabel;

public slots:
    void on_actionViewLog_triggered();
    void on_actionExitApplication_triggered();
    void on_actionParameterSetting_triggered();
    void on_actionAbout_triggered();
    void on_actionUpdateLog_triggered();
	void on_actionCheckUpdate_triggered();
    void on_actionFeedbackAndSuggestion_triggered();

public slots:
    void setTestStatus(TestStatus status);
    void connectButton();
    void exitButton();
    void previewButton();
    void testButton();
    void updateText(const QString& text);
    void updateImage(int id, const QPixmap& pixmap);
	void lastError(int id, int code, const QString& error);
	void enableButton(ButtonType type, bool enable);
    void clickButton(ButtonType type);
	void addListItem(const QString& item);
    void clearListItem();
	void imageLabelCustomContextMenuRequested(const QPoint& point);
    void imageLabelRepreview();
    void imageLabelImageMaximum();
    void imageLabelSaveImage();

signals:
    void applicationExitSignal();
};
