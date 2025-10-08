#ifndef CHARGE_TESTOR_WIN_H
#define CHARGE_TESTOR_WIN_H

#include <QMainWindow>
#include <QTimer>
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QLineEdit>

#include <ccxx/cxthread.h>
#include <ccxx/cxchannel.h>

#include "charging_agreement.h"
#include "dlt645/dlt645.h"


namespace Ui {
class ChargeTestorWin;
}


// -------------------------------------------------------------------------------------------------------


class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};


// -------------------------------------------------------------------------------------------------------


class TestQEvent : public QObject
{

    Q_OBJECT

public:
    explicit TestQEvent(QObject *parent=0);

    ~TestQEvent();

    bool event(QEvent *);

};


class TimerThread : public CxJoinableThread
{
public:
    void waitExit();

protected:
    void run();

};


// -------------------------------------------------------------------------------------------------------


class ChargeTestorWin : public QMainWindow , public CxIChannelSubject , CxInterinfoOut_I , ICxTimerNotify, public ChargeTestorInter
{

    Q_OBJECT

public:
    explicit ChargeTestorWin(QWidget *parent = 0);
    ~ChargeTestorWin();

    void testPostEvent();

    void outInfo(const QString& sInfo);

    void outInfo(const std::string& sInfo);

protected:
    bool channel_canChangeConnect(const CxChannelBase *oChannel, bool bOldConnect, bool bNewConnect);

    void channel_connectChanged(const CxChannelBase *oChannel);

    void channel_beforeDelete(const CxChannelBase *oChannel);

    void channel_receivedData(const uchar *pData, int iLength, void *oSource);

    void channel_sentData(const char *pData, int iLength, void *oTarget);

    void interinfo_out_simple(int iLevel, int iGroup, const std::string& sMessage, const std::string& sTitle, int iTag);

    CxInterinfo::PlatformEnum platformValue() { return CxInterinfo::Platform_Desktop; }

    void timer_timeOut(const CxTimer* oTimer);

protected:
    int check_addr(const QString &in_src);
    void getLineEdit(QLineEdit *lineEdit, double &out_data);

private slots:
    void on_connectBn_clicked();

    void on_closeBn_clicked();

    void on_sendBn_clicked();

    void on_isSendQueueCb_stateChanged(int arg1);

    void on_testBn_clicked();

    void on_clearBn_clicked();

    void on_clearEd_valueChanged(int arg1);

    void on_infoOutIntervalEd_valueChanged(int arg1);

    void on_sendIntervalEd_valueChanged(int arg1);

    void on_crcEd_currentIndexChanged(int index);

    void on_crcBn_clicked();

    void on_clearSendBn_clicked();

    void on_analogChargingButton_clicked();

    void on_addButton_clicked();

    void on_modifyButton_clicked();

    bool eventFilter(QObject *obj, QEvent *event);

    void on_deleteButton_clicked();

    void on_emptyButton_clicked();

    void on_batchAddButton_clicked();

    void on_analogMeterButton_clicked();

public:
    int DoSendData(const char* pData, int iLength);
    void DoShowWindowImpl(const char* pStr);
    int AddrCheck(const char* addr, ProtocolType pro_type);
    int GetData(Dlt645DataType data_type, double &out_data);

private:
    Ui::ChargeTestorWin *ui;
    TimerThread _timerThread;
    //
    CxChannelBase *_channel;
    CxTimer _sendTimer;
    // serial setting
    std::string _portName;
    int _baudRateEnum;
    int _flowControl;
    int _parity;
    int _stopBits;
    int _characterSize;
    //
    int _infoOutMaxLine;
    int _infoOutInterval;
    //
    QLabel *_statusChannelConnected;
    QLabel *_statusChannelSend;
    QLabel *_statusChannelRecv;
    QProgressBar *_progressBar;
    std::vector<std::string> oldPortNames;

    ChargingAgree m_pChargingAgree;
    IDlt645 *dlt645_;
};

#endif // CHARGE_TESTOR_WIN_H
