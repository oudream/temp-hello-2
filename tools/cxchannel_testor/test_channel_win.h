#ifndef TEST_CHANNEL_WIN_H
#define TEST_CHANNEL_WIN_H

#include <QMainWindow>
#include <QTimer>
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>

#include <ccxx/cxthread.h>
#include <ccxx/cxchannel.h>


namespace Ui {
class TestChannelWin;
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


class TestChannelWin : public QMainWindow , public CxIChannelSubject , CxInterinfoOut_I , ICxTimerNotify
{

    Q_OBJECT

public:
    explicit TestChannelWin(QWidget *parent = 0);
    ~TestChannelWin();

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

private:
    Ui::TestChannelWin *ui;
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

};

#endif // TEST_CHANNEL_WIN_H
