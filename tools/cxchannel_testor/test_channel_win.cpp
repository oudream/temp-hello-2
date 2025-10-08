#include "test_channel_win.h"
#include "ui_test_channel_win.h"


#include <ccxx/cxqtutil.h>

#include <ccxx/cxchannel_udp.h>
#include <ccxx/cxchannel_tcpclient.h>
#include <ccxx/cxchannel_tcpserver.h>
#include <ccxx/cxchannel_serial.h>
#include <ccxx/cxapplication.h>
#include <ccxx/cxcrc.h>
#include <ccxx/cxuuid.h>


using namespace std;


TestChannelWin * f_win = NULL;
QEvent f_event = QEvent(QEvent::User);


TestQEvent f_testEvent;



// -------------------------------------------------------------------------------------------------------


CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
         if (block.isVisible() && bottom >= event->rect().top()) {
             QString number = QString::number(blockNumber + 1);
             painter.setPen(Qt::black);
             painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                              Qt::AlignRight, number);
         }

         block = block.next();
         top = bottom;
         bottom = top + qRound(blockBoundingRect(block).height());
         ++blockNumber;
     }
 }


// -------------------------------------------------------------------------------------------------------


TestChannelWin::TestChannelWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestChannelWin)
{
    f_win = this;
    //
    _channel = NULL;
    CxInterinfoOut::addObserver(this);
    //
    ui->setupUi(this);
    //
    _statusChannelConnected = new QLabel("NULL",this);
    _statusChannelConnected->setMidLineWidth(100);
    _statusChannelSend = new QLabel("NULL", this);
    _statusChannelSend->setMidLineWidth(100);
    _statusChannelRecv = new QLabel("NULL", this);
    _statusChannelRecv->setMidLineWidth(100);
    ui->statusbar->addWidget(_statusChannelConnected);
    ui->statusbar->addWidget(_statusChannelSend);
    ui->statusbar->addWidget(_statusChannelRecv);
    _progressBar = new QProgressBar(this);
    ui->statusbar->addPermanentWidget(_progressBar, 1);
    _progressBar->setTextVisible(true);
    _progressBar->setValue(0);
    //
    QFont font2;
    font2.setPointSize(9);
    delete ui->infoOutEd;;
    ui->infoOutEd = new CodeEditor(ui->groupBox_5);
    ui->infoOutEd->setObjectName(QString::fromUtf8("infoOutEd"));
    ui->infoOutEd->setFont(font2);
    ui->horizontalLayout_9->addWidget(ui->infoOutEd);
    //
    _infoOutMaxLine = ui->clearEd->value();
    _infoOutInterval = ui->infoOutIntervalEd->value();
    //
    ui->connectIp1Ed->addItem(CxQString::gbkToQString("127.0.0.1"));
    ui->connectIp1Ed->addItem(CxQString::gbkToQString("0.0.0.0"));
    std::vector<std::string> sLocalIps = CxNetwork::getLocalIps();
    for (size_t i = 0; i < sLocalIps.size(); ++i)
    {
        string sIp = sLocalIps.at(i);
        if (CxString::isValidIp(sIp))
        {
            ui->connectIp1Ed->addItem(CxQString::gbkToQString(sIp));
        }
    }
    //
    vector<string> sPortNames = CxChannelSerial::getPortNames();
    for (size_t i = 0; i < sPortNames.size(); ++i)
    {
        ui->PortNameEd->addItem(CxQString::gbkToQString(sPortNames.at(i)));
    }
    //
    ui->DataBitsEd->setCurrentIndex(3);
    ui->BaudRateEd->setCurrentIndex(6);
}

TestChannelWin::~TestChannelWin()
{
    CxInterinfoOut::removeObserver(this);
    _timerThread.waitExit();
    if (_channel) delete _channel;
    delete ui;
}

void TestChannelWin::testPostEvent()
{

}

void TestChannelWin::on_sendBn_clicked()
{
    GM_INVALID_RETURE(_channel);

    string sInfo = CxQString::gbkToStdString(ui->infoInEd->toPlainText());
    vector<uchar> data = CxString::fromHexstring(sInfo);

    string sIp = CxQString::gbkToStdString(ui->remoteIpEd->text());
    string sPort = CxQString::gbkToStdString(ui->remotePortEd->text());
    if (CxString::isValidIp(sIp) && CxString::isValidPort(CxString::toInt32(sPort)) && ui->channelTypeCb->currentIndex() == 3 && ui->channelTypeCb->currentIndex() == 4)
    {
        ((CxChannelUdp *)_channel)->writeTo((const char *)data.data(), data.size(), sIp, CxString::toInt32(sPort));
    }
    else
    {
        _channel->sendBytes(data);
    }
}

void TestChannelWin::outInfo(const QString &sInfo)
{
    static int iCount = 0;
    if (iCount > _infoOutMaxLine)
    {
        ui->infoOutEd->clear();
        iCount = 0;
    }
    iCount++;
    ui->infoOutEd->appendPlainText(sInfo);
}

void TestChannelWin::outInfo(const string &sInfo)
{
    outInfo(CxQString::gbkToQString(sInfo));
}

void TestChannelWin::interinfo_out_simple(int iLevel, int iGroup, const std::string& sMessage, const std::string& sTitle, int iTag)
{
    outInfo(sMessage);
}

void TestChannelWin::timer_timeOut(const CxTimer *oTimer)
{
    if (_channel)
    {
        _statusChannelRecv->setText(QString::number(_channel->receivedByteCount()));
    }
}

TestQEvent::TestQEvent(QObject *parent)
  : QObject(parent)
{
    
}

TestQEvent::~TestQEvent()
{
}

bool TestQEvent::event(QEvent *)
{
    f_win->outInfo(CxTime::currentSystemTimeString());
    return true;
}


volatile bool _canRun = true;

QEvent f_receivedDataEvent(QEvent::User);

void TimerThread::waitExit()
{
    _canRun = false;
    join();
}

void TimerThread::run()
{
    while (_canRun)
    {
        CxThread::sleep(10);
        QApplication::postEvent(&f_testEvent, &f_receivedDataEvent);
    }
}

void TestChannelWin::on_connectBn_clicked()
{
    if (_channel && _channel->connected())
    {
        _channel->close();
        delete _channel;
        _channel = NULL;
        outInfo(QString("disconnect success!!!"));
        return;
    }

    string sIp1 = CxQString::gbkToStdString(ui->connectIp1Ed->currentText());
    string sIp2 = CxQString::gbkToStdString(ui->connectIp2Ed->text());
    int iPort1 = ui->connectPort1Ed->value();
    int iPort2 = ui->connectPort2Ed->value();
    if (! CxString::isValidIp(sIp1) || ! CxString::isValidIp(sIp2)
            || ! CxString::isValidPort(iPort1)|| ! CxString::isValidPort(iPort2))
        return;
    //
    _portName = ui->PortNameEd->currentText().toStdString();
    _baudRateEnum = ui->BaudRateEd->currentIndex();
    _flowControl = ui->FlowEd->currentIndex();
    _parity = ui->ParityEd->currentIndex();
    _stopBits = ui->StopBitsEd->currentIndex();
    _characterSize = ui->DataBitsEd->currentIndex();
    //
    if (! _channel)
    {
        //udp
        //tcp client
        //tcp server
        //udp (multi road)
        //tcp (multi road)
        int iChannelType = ui->channelTypeCb->currentIndex();
        switch (iChannelType)
        {
        case 1:
        {
            CxChannelTcpclient * oTcpclient = new CxChannelTcpclient();
            _channel = oTcpclient;
            oTcpclient->setRemoteIp(sIp2);
            oTcpclient->setRemotePort(iPort2);
        }
            break;
        case 0:
        case 3:
        {
            CxChannelUdp * oUdp = new CxChannelUdp();
            _channel = oUdp;
            oUdp->setLocalIp(sIp1);
            oUdp->setLocalPort(iPort1);
            oUdp->setRemoteIp(sIp2);
            oUdp->setRemotePort(iPort2);
        }
            break;
        case 2:
        case 4:
        {
            CxChannelTcpserver * oTcpServer = new CxChannelTcpserver();
            _channel = oTcpServer;
            oTcpServer->setLocalIp(sIp1);
            oTcpServer->setLocalPort(iPort1);
        }
            break;
        case 5:
        {
            CxChannelSerial * oSerial = new CxChannelSerial();
            _channel = oSerial;
            oSerial->setPortName(_portName);
            oSerial->setBaudRate((CxChannelSerial::BaudRateEnum)_baudRateEnum);
            oSerial->setFlowControl((CxChannelSerial::Flow)_flowControl);
            oSerial->setParity((CxChannelSerial::Parity)_parity);
            oSerial->setStopBits((CxChannelSerial::StopBits)_stopBits);
            oSerial->setCharacterSize((CxChannelSerial::CharacterSize)_characterSize);
        }
            break;
        default:
            break;
        }
        _channel->addObserver(this);
        _channel->setAutoOpenInterval(3000);
        if (iChannelType == 3 || iChannelType == 4)
        {
            _channel->setIsMultiRoad(true);
        }
        _channel->open();
    }

    this->setEnabled(false);
    _statusChannelConnected->setText("disconnected");
    int iProgress = 0;
    _progressBar->setValue(iProgress);
    msepoch_t dtNow = CxTime::currentMsepoch();
    while (! _channel->connected())
    {
        iProgress++;
        if (CxTime::milliSecondDifferToNow(dtNow) > 30000 || iProgress>100)
        {
            break;
        }
        if (_progressBar->value() >= 100)
        {
            break;
        }
        _progressBar->setValue(iProgress);
        CxApplication::waiting(100);
    }

    if (!_channel->connected())
    {
        GM_DELETEANDNULL_OBJECT(_channel);
    }
    else
    {
        _statusChannelConnected->setText("connected");
    }
    _progressBar->setValue(0);
    this->setEnabled(true);
}

void TestChannelWin::on_closeBn_clicked()
{
    if (_channel && _channel->connected())
    {
        _channel->close();
        delete _channel;
        _channel = NULL;
    }
    this->close();
    CxApplication::exit();
}

void TestChannelWin::on_isSendQueueCb_stateChanged(int arg1)
{
    GM_INVALID_RETURE(_channel);

    _channel->setIsSendQueue(ui->isSendQueueCb->checkState() == Qt::Checked);
}

void TestChannelWin::on_testBn_clicked()
{
}

void TestChannelWin::on_clearBn_clicked()
{
    ui->infoOutEd->clear();
}

bool TestChannelWin::channel_canChangeConnect(const CxChannelBase *oChannel, bool bOldConnect, bool bNewConnect)
{
    return true;
}

void TestChannelWin::channel_connectChanged(const CxChannelBase *oChannel)
{
    _progressBar->setValue(100);
    cxInfo() << "Channel Connect " << (oChannel->isOpen() ? "Success" : "Fail");
    ui->connectBn->setText(oChannel->isOpen() ? "disconnect" : "connect");
    _statusChannelConnected->setText(oChannel->isOpen() ? "disconnected" : "connected");
    _statusChannelSend->setText(QString::number(_channel->sentByteCount()));
    _statusChannelRecv->setText(QString::number(_channel->receivedByteCount()));
}

void TestChannelWin::channel_beforeDelete(const CxChannelBase *oChannel)
{
}

void TestChannelWin::channel_receivedData(const uchar *pData, int iLength, void *oSource)
{
    static msepoch_t dtNow = CxTime::currentMsepoch();
    if (_infoOutInterval > 0)
    {
        if (CxTime::milliSecondDifferToNow(dtNow) > _infoOutInterval)
        {
            outInfo(CxString::toHexstring(pData, iLength));
            _statusChannelRecv->setText(QString::number(_channel->receivedByteCount()));
            dtNow = CxTime::currentMsepoch();
        }
    }
    else
    {
        outInfo(CxString::toHexstring(pData, iLength));
        _statusChannelRecv->setText(QString::number(_channel->receivedByteCount()));
    }
}

void TestChannelWin::channel_sentData(const char *pData, int iLength, void *oTarget)
{
    _statusChannelSend->setText(QString::number(_channel->sentByteCount()));
}

void TestChannelWin::on_clearEd_valueChanged(int arg1)
{
    _infoOutMaxLine = ui->clearEd->value();
}

void TestChannelWin::on_infoOutIntervalEd_valueChanged(int arg1)
{
    _infoOutInterval = ui->infoOutIntervalEd->value();
}

void TestChannelWin::on_sendIntervalEd_valueChanged(int arg1)
{
    int v = ui->sendIntervalEd->value();
    if (v > 0)
    {
        _sendTimer.stop();
        _sendTimer.init(this, v);
        _sendTimer.start();
    }
    else
    {
        _sendTimer.stop();
    }
}

void TestChannelWin::on_crcEd_currentIndexChanged(int index)
{
    int iIndex = ui->crcEd->currentIndex();
    // uuid
    if (iIndex == 8)
    {
        string r = CxUuid::createGuidString();
        ui->infoInEd->appendPlainText("");
        ui->infoInEd->appendPlainText(CxQString::gbkToQString(r));
        return;
    }

    string sInfo = CxQString::gbkToStdString(ui->infoInEd->toPlainText());
    vector<uchar> data = CxString::fromHexstring(sInfo);
    if (sInfo.size() <= 0)
    {
        return;
    }

    string sHex;
    switch (iIndex)
    {
        // to hex
        case 1:
        {
            sHex = CxString::toHexstring(sInfo.c_str(), sInfo.size());
            ui->infoInEd->setPlainText(CxQString::gbkToQString(sHex));
            return;
        }
            break;
        // to ascii
        case 2:
        {
            vector<uchar> r = CxString::fromHexstring(sInfo);
            if (r.size() > 0)
            {
                ui->infoInEd->setPlainText(CxQString::gbkToQString(string((char *)&r.front())));
            }
            return;
        }
            break;
        // sum8
        case 3:
        {
            uchar r = CxCrc::calcsum8(&data.front(), data.size());
            sHex = CxString::toHexstring(r);
        }
            break;
        // sum16
        case 4:
        {
            ushort r = CxCrc::calcsum16(&data.front(), data.size());
            sHex = CxString::toHexstring(r);
        }
            break;
        // crc8
        case 5:
        {
            uchar r = CxCrc::calcCRC8(&data.front(), data.size());
            sHex = CxString::toHexstring(r);
        }
            break;
        // crc16
        case 6:
        {
            ushort r = CxCrc::calcModbus(&data.front(), data.size());
            sHex = CxString::toHexstring(r);
        }
            break;
        // md5
        case 7:
        {
            string r = CxCrc::md5HexCode(sInfo);
            ui->infoInEd->appendPlainText("");
            ui->infoInEd->appendPlainText(CxQString::gbkToQString(r));
            return;
        }
            break;
        default:
            break;
    }
    ui->infoInEd->setPlainText(CxQString::gbkToQString(sInfo + " " + CxString::insertSplit(sHex, ' ', 2)));
}

void TestChannelWin::on_crcBn_clicked()
{
    on_crcEd_currentIndexChanged(ui->crcEd->currentIndex());
}

void TestChannelWin::on_clearSendBn_clicked()
{
    ui->infoInEd->clear();
}
