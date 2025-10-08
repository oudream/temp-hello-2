#include "qt_helper.h"

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QComboBox>
#include <QStringDecoder>

#include <ccxx/cxcontainer.h>

using namespace std;

const QChar HexQChar16[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

QStringDecoder f_gbk("GB18030");
QStringDecoder f_utf8("UTF-8");

///************ ToString begin
QString QStringHelper::gbkToQString(const std::string &s)
{
    return QString::fromLocal8Bit(s.data(), s.size());
}

QStringList QStringHelper::gbkToQStrings(const std::vector<std::string> &ss)
{
    QStringList r;
    for (size_t i = 0; i < ss.size(); ++i)
    {
        const std::string &s = ss.at(i);
        r.append(QString::fromLocal8Bit(s.data(), s.size()));
    }
    return r;
}

QStringList QStringHelper::gbkToQStrings(const std::string *oSs, int iCount)
{
    QStringList r;
    for (size_t i = 0; i < iCount; ++i)
    {
        const std::string &s = oSs[i];
        r.append(QString::fromLocal8Bit(s.data(), s.size()));
    }
    return r;
}

std::vector<std::string> QStringHelper::gbkToStdStrings(const QStringList &ss)
{
    std::vector<std::string> r;
    for (size_t i = 0; i < ss.count(); ++i)
    {
        const QString &s = ss[i];
        r.push_back(gbkToStdString(s));
    }
    return r;
}

std::string QStringHelper::gbkToStdString(const QString &s)
{
    QByteArray data = s.toLocal8Bit();
    return std::string(data.data(), data.size());
}

QString QStringHelper::ToString(const int &i)
{
    return QString::number(i);
}

QString QStringHelper::ToString(const unsigned int &i)
{
    return QString::number(i);
}

QString QStringHelper::ToString(const int &i, int iZeroCount)
{
    QString s = QString::number(i);
    QString v;
    for (int i = 0; i < iZeroCount - s.length(); ++i)
    {
        v += "0";
    }
    return v + s;
}


QString QStringHelper::ToString(const bool &b)
{
    return QString::number(b);
}


QString QStringHelper::ToString(const double &d)
{
    return QString::number(d);
}


QString QStringHelper::ToString(const QString &s)
{
    return s;
}

QString QStringHelper::ToString(const std::string &s)
{
    return QString::fromLocal8Bit(s.data(), s.size());
}

QString QStringHelper::ToString(const qlonglong &dt)
{
    return DateTimeToString(dt);
}

QString QStringHelper::ToString(const QHash<QString, QString> &keyValues)
{
    QStringList sResults;
    QHash<QString, QString>::const_iterator it = keyValues.constBegin();
    while (it != keyValues.constEnd())
    {
        sResults.append(QString("%1:%2").arg(it.key()).arg(it.value()));
        ++it;
    }
    return sResults.join("|");
}

QString QStringHelper::ToString(const QStringList &keyValues)
{
    return keyValues.join("|");
}

QString QStringHelper::ToString(const char *pData, int iLength)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 6))
    QString s = QString::fromLocal8Bit(pData, iLength);
#else
    QString s = QString::fromLocal8Bit(pData, iLength);
#endif
    return s;
}

QString QStringHelper::ToString(const char *pData)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 6))
    QString s = QString::fromLocal8Bit(pData, strlen(pData));
#else
    QString s = QString::fromLocal8Bit(pData, strlen(pData));
#endif
    return s;
}

QString QStringHelper::ToString(const cx::uchar *pData, int iLength)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 6))
    QString s = QString::fromLocal8Bit((char *) pData, iLength);
#else
    QString s = QString::fromLocal8Bit((char*)pData, iLength);
#endif
    return s;
}

///************ ToString end


///************ FromString begin
int QStringHelper::FromString(const QString &s, const int &iDefault, bool *ok)
{
    if (s.length() <= 0)
        return iDefault;
    bool b;
    int v = s.toInt(&b, 10);
    if (ok) *ok = b;
    if (b)
        return v;
    else
        return iDefault;
}


bool QStringHelper::FromString(const QString &s, const bool &bDefault, bool *ok)
{
    if (s.length() <= 0)
        return bDefault;
    bool b;
    int v = s.toInt(&b, 10);
    if (ok) *ok = b;
    if (b)
        return (bool) v;
    else
        return bDefault;
}


float QStringHelper::FromString(const QString &s, const float &fDefault, bool *ok)
{
    if (s.length() <= 0)
        return fDefault;
    bool b;
    float v = s.toFloat(&b);
    if (ok) *ok = b;
    if (b)
        return v;
    else
        return fDefault;
}


double QStringHelper::FromString(const QString &s, const double &dDefault, bool *ok)
{
    if (s.length() <= 0)
        return dDefault;
    bool b;
    double v = s.toDouble(&b);
    if (ok) *ok = b;
    if (b)
        return v;
    else
        return dDefault;
}


QString QStringHelper::FromString(const QString &s, const QString &sDefault, bool *ok)
{
    if (ok) *ok = true;
    return s;
}

qlonglong QStringHelper::FromString(const QString &s, const qlonglong &dtDefault, bool *ok)
{
    if (s.length() <= 0)
    {
        return dtDefault;
    }
    qlonglong dt = DateTimeFromString(s);
    if (ok)
    {
        *ok = dt == 0;
    }
    return dt;
}

QHash<QString, QString>
QStringHelper::FromString(const QString &s, const QHash<QString, QString> &hashDefault, bool *ok)
{
    if (s.length() <= 0)
        return hashDefault;
    QHash<QString, QString> hashResult;
    QStringList ss = s.split("|");
            foreach (QString s2, ss)
        {
            QString sKey = FirstToken(s2, ':');
            hashResult[sKey] = s2;
        }
    if (hashResult.count() > 0)
        return hashResult;
    else
        return hashDefault;
}


QStringList QStringHelper::FromString(const QString &s, const QStringList &hashDefault, bool *ok)
{
    if (s.length() <= 0)
        return hashDefault;
    return s.split("|");
}

QString QStringHelper::DateTimeToString(const qlonglong &ms)
{
    QDateTime _g_datetime;

    _g_datetime.setMSecsSinceEpoch(ms);

    QDate date = _g_datetime.date();

    QTime t = _g_datetime.time();

    int y, m, d;
    date.getDate(&y, &m, &d);

    QString s = DateTimeToString(y, m, d, t.hour(), t.minute(), t.second());

    return s;
}

QString QStringHelper::DateTimeToString(int y, int m, int d, int h, int mi, int ms)
{
    QString dts("2012/12/12 12:12:12");

    if (y > -1 && y < 10000 && m > 0 && m < 13 && d > 0 && d < 32 && h > -1 && h < 24 && mi > -1 && mi < 60 &&
        ms > -1 && ms < 100)
    {
        QChar *data = dts.data();

        int hc, lc;

        int yy = y % 100;

        hc = yy / 10;
        lc = yy % 10;
        data[2] = HexQChar16[hc];
        data[3] = HexQChar16[lc];

        hc = m / 10;
        lc = m % 10;
        data[5] = HexQChar16[hc];
        data[6] = HexQChar16[lc];

        hc = m / 10;
        lc = m % 10;
        data[5] = HexQChar16[hc];
        data[6] = HexQChar16[lc];

        hc = d / 10;
        lc = d % 10;
        data[8] = HexQChar16[hc];
        data[9] = HexQChar16[lc];

        hc = h / 10;
        lc = h % 10;
        data[11] = HexQChar16[hc];
        data[12] = HexQChar16[lc];

        hc = mi / 10;
        lc = mi % 10;
        data[14] = HexQChar16[hc];
        data[15] = HexQChar16[lc];

        hc = ms / 10;
        lc = ms % 10;
        data[17] = HexQChar16[hc];
        data[18] = HexQChar16[lc];
    }
    else
    {
//        dts = QString("err:%1-%2-%3 %4:%5:%6").arg(y).arg(m).arg(d).arg(h).arg(mi).arg(ms);
    }

    return dts;
}

qlonglong QStringHelper::DateTimeFromString(const QString &str)
{
//    QString dts("2012/12/12 12:12:12");
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 6))
    QByteArray data = str.toLocal8Bit();
#else
    QByteArray data = str.toAscii();
#endif
    if (data.size() == 19)
    {

        int y, m, d, h, mi, s;

        const char zero = '0';

        y = (data[2] - zero) * 10 + (data[3] - zero);
        m = (data[5] - zero) * 10 + (data[6] - zero);
        d = (data[8] - zero) * 10 + (data[9] - zero);
        h = (data[11] - zero) * 10 + (data[12] - zero);
        mi = (data[14] - zero) * 10 + (data[15] - zero);
        s = (data[17] - zero) * 10 + (data[18] - zero);

        QDate date(y, m, d);
        QTime time(h, mi, s);
        return QDateTime(date, time).toMSecsSinceEpoch();
    }
    else
    {
        return 0;
    }
}

QString QStringHelper::FirstToken(QString &s, const QChar &sep)
{
    QString sResult;
    int iIndex = s.indexOf(sep);
    if (iIndex > -1)
    {
        sResult = s.left(iIndex);
        s = s.right(s.size() - iIndex - 1);
    }
    return sResult;
}

QString QStringHelper::RightToken(const QString &s, const QChar &sep)
{
    QString sResult;
    int iIndex = s.indexOf(sep);
    if (iIndex > -1)
    {
        sResult = s.right(s.size() - iIndex - 1);
    }
    return sResult;
}

QString QStringHelper::ToHexString(const char *pData, int iLength)
{
    QByteArray hex(iLength * 3, Qt::Uninitialized);
    char *hexData = hex.data();
    const cx::uchar *data = (const cx::uchar *) pData;
    int i = 0;
    for (; i < iLength; ++i)
    {
        int j = (data[i] >> 4) & 0xf;
        if (j <= 9)
            hexData[i * 3] = (j + '0');
        else
            hexData[i * 3] = (j + 'a' - 10);
        j = data[i] & 0xf;
        if (j <= 9)
            hexData[i * 3 + 1] = (j + '0');
        else
            hexData[i * 3 + 1] = (j + 'a' - 10);
        hexData[i * 3 + 2] = 0x20;
    }
    return hex;
}

QString QStringHelper::ToHexString(const cx::uchar *pData, int iLength)
{
    QByteArray hex(iLength * 3, Qt::Uninitialized);
    char *hexData = hex.data();
    const cx::uchar *data = (const cx::uchar *) pData;
    int i = 0;
    for (; i < iLength; ++i)
    {
        int j = (data[i] >> 4) & 0xf;
        if (j <= 9)
            hexData[i * 3] = (j + '0');
        else
            hexData[i * 3] = (j + 'a' - 10);
        j = data[i] & 0xf;
        if (j <= 9)
            hexData[i * 3 + 1] = (j + '0');
        else
            hexData[i * 3 + 1] = (j + 'a' - 10);
        hexData[i * 3 + 2] = 0x20;
    }
    return hex;
}

QString QStringHelper::ToHexString(const QByteArray &data)
{
    return ToHexString(data.data(), data.size());
}

QString QStringHelper::ToHexString(const char &v)
{
    QString s = "0x00";
    QChar *data = s.data();
    int j = (v >> 4) & 0xf;
    data[2] = HexQChar16[j];
    j = v & 0xf;
    data[3] = HexQChar16[j];
    return s;
}

QString QStringHelper::ToHexString(const cx::uchar &v)
{
    QString s = "0x00";
    QChar *data = s.data();
    int j = (v >> 4) & 0xf;
    data[2] = HexQChar16[j];
    j = v & 0xf;
    data[3] = HexQChar16[j];
    return s;
}

QString QStringHelper::ToHexString(const short &v)
{
    QString s = "0x0000";
    QChar *data = s.data();
    int j = (v >> 12) & 0xf;
    data[2] = HexQChar16[j];
    j = (v >> 8) & 0xf;
    data[3] = HexQChar16[j];
    j = (v >> 4) & 0xf;
    data[4] = HexQChar16[j];
    j = v & 0xf;
    data[5] = HexQChar16[j];
    return s;
}

QString QStringHelper::ToHexString(const cx::ushort &v)
{
    QString s = "0x0000";
    QChar *data = s.data();
    int j = (v >> 12) & 0xf;
    data[2] = HexQChar16[j];
    j = (v >> 8) & 0xf;
    data[3] = HexQChar16[j];
    j = (v >> 4) & 0xf;
    data[4] = HexQChar16[j];
    j = v & 0xf;
    data[5] = HexQChar16[j];
    return s;
}

QString QStringHelper::ToHexString(const int &v)
{
    QString s = "0x00000000";
    QChar *data = s.data();
    int j = (v >> 28) & 0xf;
    data[2] = HexQChar16[j];
    j = (v >> 24) & 0xf;
    data[3] = HexQChar16[j];
    j = (v >> 20) & 0xf;
    data[4] = HexQChar16[j];
    j = (v >> 16) & 0xf;
    data[5] = HexQChar16[j];
    j = (v >> 12) & 0xf;
    data[6] = HexQChar16[j];
    j = (v >> 8) & 0xf;
    data[7] = HexQChar16[j];
    j = (v >> 4) & 0xf;
    data[8] = HexQChar16[j];
    j = v & 0xf;
    data[9] = HexQChar16[j];
    return s;
}

QString QStringHelper::ToHexString(const cx::uint &v)
{
    QString s = "0x00000000";
    QChar *data = s.data();
    int j = (v >> 28) & 0xf;
    data[2] = HexQChar16[j];
    j = (v >> 24) & 0xf;
    data[3] = HexQChar16[j];
    j = (v >> 20) & 0xf;
    data[4] = HexQChar16[j];
    j = (v >> 16) & 0xf;
    data[5] = HexQChar16[j];
    j = (v >> 12) & 0xf;
    data[6] = HexQChar16[j];
    j = (v >> 8) & 0xf;
    data[7] = HexQChar16[j];
    j = (v >> 4) & 0xf;
    data[8] = HexQChar16[j];
    j = v & 0xf;
    data[9] = HexQChar16[j];
    return s;
}

QString QStringHelper::ToHexString(const float &v)
{
    char fData[4];
    memcpy(fData, &v, 4);
    return ToHexString(fData, 4);
}

QString QStringHelper::ToHexString(const double &v)
{
    char fData[8];
    memcpy(fData, &v, 8);
    return ToHexString(fData, 8);
}

QByteArray QStringHelper::FromHexString(const QString &s)
{
    QString s2 = s;
    s2.remove(' ');
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 6))
    QByteArray hexEncoded = s2.toLocal8Bit();
#else
    QByteArray hexEncoded = s2.toAscii();
#endif
    QByteArray res((hexEncoded.size() + 1) / 2, Qt::Uninitialized);
    cx::uchar *result = (cx::uchar *) res.data() + res.size();

    bool odd_digit = true;
    for (int i = hexEncoded.size() - 1; i >= 0; --i)
    {
        int ch = hexEncoded.at(i);
        int tmp;
        if (ch >= '0' && ch <= '9')
            tmp = ch - '0';
        else if (ch >= 'a' && ch <= 'f')
            tmp = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F')
            tmp = ch - 'A' + 10;
        else
            continue;
        if (odd_digit)
        {
            --result;
            *result = tmp;
            odd_digit = false;
        }
        else
        {
            *result |= tmp << 4;
            odd_digit = true;
        }
    }

    res.remove(0, result - (const cx::uchar *) res.constData());
    return res;
}

bool QStringHelper::CompareName(const QString &name1, const QString &name2)
{
    QString s1 = name1.trimmed().toLower();
    QString s2 = name2.trimmed().toLower();
    return s1 == s2;
}


void QDialogHelper::ShowPrompt(const QString &sText, const QString &sTitle)
{
    QMessageBox::information(nullptr, sTitle, sText, QMessageBox::Ok);
}


void QDialogHelper::ShowWarning(const QString &sText)
{
    QMessageBox::warning(0, "warning", sText, QMessageBox::Ok);
}


void QDialogHelper::ShowError(const QString &sText)
{
    QMessageBox::warning(0, "error", sText, QMessageBox::Ok);
}


bool QDialogHelper::ShowQuery(const QString &sQuery, const QString &sTitle, const QString &sOk, const QString &sCancel)
{
    QMessageBox msgBox(QMessageBox::Warning, sTitle, sQuery, QMessageBox::NoButton);

    QPushButton *okButton     = msgBox.addButton(sOk, QMessageBox::AcceptRole);
    QPushButton *cancelButton = msgBox.addButton(sCancel, QMessageBox::RejectRole);

    msgBox.setDefaultButton(okButton);

    msgBox.exec();

    return (msgBox.clickedButton() == okButton);
}


int QDialogHelper::ShowQuery3(const QString &sQuery)
{
    QMessageBox::StandardButton ret = QMessageBox::warning(
            nullptr,
            "Dialog",
            sQuery,
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::No   // 默认选中
    );

    switch (ret)
    {
        case QMessageBox::Yes:
            return 1;
        case QMessageBox::No:
            return 0;
        case QMessageBox::Cancel:
        default:
            return -1;
    }
}


void QDialogHelper::ShowTerminate(const QString &sText)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("terminate");
    msgBox.setText(sText);
    msgBox.addButton("My God", QMessageBox::AcceptRole);  // 自定义按钮文字
    msgBox.exec();
}

//sameple : sExtension = *.png *.jpg *.bmp
bool QDialogHelper::DialogOpen(QString &sFilePath, const QString &sExtension)
{
    QString sExtension2 = sExtension;
    if (sExtension2.length() <= 0)
    {
        sExtension2 = "*.*";
    }
    sFilePath = QFileDialog::getOpenFileName(0, "Open file...", "", QString("Files (%1)").arg(sExtension2));
    return sFilePath.length();
}

//sameple : sExtension = *.png *.jpg *.bmp
bool QDialogHelper::DialogSave(QString &sFilePath, const QString &sDefaultPath, const QString &sExtension)
{
    QString sExtension2 = sExtension;
    if (sExtension2.length() <= 0)
    {
        sExtension2 = "*.*";
    }
    sFilePath = QFileDialog::getSaveFileName(0, "Save file...", sDefaultPath, QString("Files (%1)").arg(sExtension2));
    return sFilePath.length();
}

QString QDialogHelper::DialogDir(const QString &sRootDir, const QString &sTitle)
{
    QString sCaption = sTitle.size() > 0 ? sTitle : "Select Directory";
    return QFileDialog::getExistingDirectory(0,
                                             sCaption, sRootDir);
}

bool QDialogHelper::DialogInput(const QString &sPrompt, QString &sVaule)
{
    bool ok;
    QString text = QInputDialog::getText(0, "Input",
                                         sPrompt, QLineEdit::Normal,
                                         sVaule, &ok);
    if (ok)
    {
        sVaule = text;
        return true;
    }
    else
        return false;
}

bool QDialogHelper::DialogInput2(const QString &sTitle, const QString &sValue1Title, const QString &sValue2Title,
                                 QString &sValue1, QString &sValue2, int iValue1EchoMode, int iValue2EchoMode)
{
    QDialog dialog;

    QLabel *value1Label = new QLabel(sValue1Title);
    QLabel *value2Label = new QLabel(sValue2Title);
    QLineEdit *value1Ed = new QLineEdit;
    value1Ed->setEchoMode((QLineEdit::EchoMode) iValue1EchoMode);
    QLineEdit *value2Ed = new QLineEdit;
    value2Ed->setEchoMode((QLineEdit::EchoMode) iValue2EchoMode);

    value1Ed->setText(sValue1);
    value2Ed->setText(sValue1);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(value1Label, 0, 0, 1, 1);
    gridLayout->addWidget(value1Ed, 0, 1, 1, 3);
    gridLayout->addWidget(value2Label, 1, 0, 1, 1);
    gridLayout->addWidget(value2Ed, 1, 1, 1, 3);

    QPushButton *okBtn = new QPushButton(QString::fromUtf8("OK"));
    QPushButton *cancelBtn = new QPushButton(QString::fromUtf8("Cancel"));
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(60);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    QVBoxLayout *dlgLayout = new QVBoxLayout;
    dlgLayout->setContentsMargins(40, 40, 40, 40);
    dlgLayout->addLayout(gridLayout);
    dlgLayout->addStretch(40);
    dlgLayout->addLayout(btnLayout);
    dialog.setLayout(dlgLayout);

    dialog.connect(okBtn, SIGNAL(clicked()), &dialog, SLOT(accept()));
    dialog.connect(cancelBtn, SIGNAL(clicked()), &dialog, SLOT(reject()));

    dialog.setWindowTitle(sTitle);
    dialog.resize(380, 220);
    if (dialog.exec() == QDialog::Accepted)
    {
        sValue1 = value1Ed->text();
        sValue2 = value2Ed->text();
        return true;
    }
    else
    {
        return false;
    }
}

bool QDialogHelper::DialogInput3(const QString &sTitle, const QString &sValue1Title, const QString &sValue2Title,
                                 const QString &sValue3Title, QString &sValue1, QString &sValue2, QString &sValue3,
                                 int iValue1EchoMode, int iValue2EchoMode, int iValue3EchoMode)
{
    QDialog dialog;

    QLabel *value1Label = new QLabel(sValue1Title);
    QLabel *value2Label = new QLabel(sValue2Title);
    QLabel *value3Label = new QLabel(sValue3Title);
    QLineEdit *value1Ed = new QLineEdit;
    value1Ed->setMaxLength(13);
    value1Ed->setEchoMode((QLineEdit::EchoMode) iValue1EchoMode);
    QLineEdit *value2Ed = new QLineEdit;
    value2Ed->setMaxLength(13);
    value2Ed->setEchoMode((QLineEdit::EchoMode) iValue2EchoMode);
    QLineEdit *value3Ed = new QLineEdit;
    value3Ed->setMaxLength(13);
    value3Ed->setEchoMode((QLineEdit::EchoMode) iValue3EchoMode);

    value1Ed->setText(sValue1);
    value2Ed->setText(sValue2);
    value3Ed->setText(sValue3);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(value1Label, 0, 0, 1, 1);
    gridLayout->addWidget(value1Ed, 0, 1, 1, 3);
    gridLayout->addWidget(value2Label, 1, 0, 1, 1);
    gridLayout->addWidget(value2Ed, 1, 1, 1, 3);
    gridLayout->addWidget(value3Label, 2, 0, 1, 1);
    gridLayout->addWidget(value3Ed, 2, 1, 1, 3);

    QPushButton *okBtn = new QPushButton(QPushButton::tr("OK"));
    QPushButton *cancelBtn = new QPushButton(QPushButton::tr("Cancel"));
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(60);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    QVBoxLayout *dlgLayout = new QVBoxLayout;
    dlgLayout->setContentsMargins(40, 40, 40, 40);
    dlgLayout->addLayout(gridLayout);
    dlgLayout->addStretch(40);
    dlgLayout->addLayout(btnLayout);
    dialog.setLayout(dlgLayout);

    dialog.connect(okBtn, SIGNAL(clicked()), &dialog, SLOT(accept()));
    dialog.connect(cancelBtn, SIGNAL(clicked()), &dialog, SLOT(reject()));

    dialog.setWindowTitle(sTitle);
    dialog.resize(380, 220);
    if (dialog.exec() == QDialog::Accepted)
    {
        sValue1 = value1Ed->text();
        sValue2 = value2Ed->text();
        sValue3 = value3Ed->text();
        return true;
    }
    else
    {
        return false;
    }
}

bool QDialogHelper::DialogInput4(const QString &sTitle, const QString &sValue1Title, const QString &sValue2Title,
                                 const QString &sValue3Title, const QString &sValue4Title, QString &sValue1,
                                 QString &sValue2, QString &sValue3, QString &sValue4, int iValue1EchoMode,
                                 int iValue2EchoMode, int iValue3EchoMode, int iValue4EchoMode)
{
    QDialog dialog;

    QLabel *value1Label = new QLabel(sValue1Title);
    QLabel *value2Label = new QLabel(sValue2Title);
    QLabel *value3Label = new QLabel(sValue3Title);
    QLabel *value4Label = new QLabel(sValue4Title);
    QLineEdit *value1Ed = new QLineEdit;
    value1Ed->setMaxLength(13);
    value1Ed->setEchoMode((QLineEdit::EchoMode) iValue1EchoMode);
    QLineEdit *value2Ed = new QLineEdit;
    value2Ed->setMaxLength(13);
    value2Ed->setEchoMode((QLineEdit::EchoMode) iValue2EchoMode);
    QLineEdit *value3Ed = new QLineEdit;
    value3Ed->setMaxLength(13);
    value3Ed->setEchoMode((QLineEdit::EchoMode) iValue3EchoMode);
    QLineEdit *value4Ed = new QLineEdit;
    value4Ed->setMaxLength(13);
    value4Ed->setEchoMode((QLineEdit::EchoMode) iValue4EchoMode);

    value1Ed->setText(sValue1);
    value2Ed->setText(sValue2);
    value3Ed->setText(sValue3);
    value4Ed->setText(sValue4);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(value1Label, 0, 0, 1, 1);
    gridLayout->addWidget(value1Ed, 0, 1, 1, 3);
    gridLayout->addWidget(value2Label, 1, 0, 1, 1);
    gridLayout->addWidget(value2Ed, 1, 1, 1, 3);
    gridLayout->addWidget(value3Label, 2, 0, 1, 1);
    gridLayout->addWidget(value3Ed, 2, 1, 1, 3);
    gridLayout->addWidget(value4Label, 2, 0, 1, 1);
    gridLayout->addWidget(value4Ed, 2, 1, 1, 3);

    QPushButton *okBtn = new QPushButton(QPushButton::tr("OK"));
    QPushButton *cancelBtn = new QPushButton(QPushButton::tr("Cancel"));
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(60);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    QVBoxLayout *dlgLayout = new QVBoxLayout;
    dlgLayout->setContentsMargins(40, 40, 40, 40);
    dlgLayout->addLayout(gridLayout);
    dlgLayout->addStretch(40);
    dlgLayout->addLayout(btnLayout);
    dialog.setLayout(dlgLayout);

    dialog.connect(okBtn, SIGNAL(clicked()), &dialog, SLOT(accept()));
    dialog.connect(cancelBtn, SIGNAL(clicked()), &dialog, SLOT(reject()));

    dialog.setWindowTitle(sTitle);
    dialog.resize(380, 220);
    if (dialog.exec() == QDialog::Accepted)
    {
        sValue1 = value1Ed->text();
        sValue2 = value2Ed->text();
        sValue3 = value3Ed->text();
        return true;
    }
    else
    {
        return false;
    }
}

bool
QDialogHelper::DialogCombox(const QString &sTitle, const QString &sPrompt, const QStringList &sValues, QString &sValue)
{
    QDialog dialog;

    QLabel *valueLabel = new QLabel(sPrompt);
    QComboBox *valueCombo = new QComboBox();

    valueCombo->addItems(sValues);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(valueLabel, 0, 0, 1, 1);
    gridLayout->addWidget(valueCombo, 0, 1, 1, 3);

    QPushButton *okBtn = new QPushButton(QObject::tr("OK"));
    QPushButton *cancelBtn = new QPushButton(QObject::tr("Cancel"));
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(60);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    QVBoxLayout *dlgLayout = new QVBoxLayout;
    dlgLayout->setContentsMargins(40, 40, 40, 40);
    dlgLayout->addLayout(gridLayout);
    dlgLayout->addStretch(40);
    dlgLayout->addLayout(btnLayout);
    dialog.setLayout(dlgLayout);

    dialog.connect(okBtn, SIGNAL(clicked()), &dialog, SLOT(accept()));
    dialog.connect(cancelBtn, SIGNAL(clicked()), &dialog, SLOT(reject()));

    valueCombo->setCurrentIndex(sValues.indexOf(sValue));

    dialog.setWindowTitle(sTitle);
    dialog.resize(240, 140);
    if (dialog.exec() == QDialog::Accepted)
    {
        sValue = valueCombo->currentText();
        return true;
    }
    else
    {
        return false;
    }
}


int QWidgetHelper::setItemValue16(QTableWidgetItem *oItem, const int &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value, 16));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QTableWidgetItem *oItem, const int &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QTableWidgetItem *oItem, const double &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toDouble() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value, 'g', 3));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QTableWidgetItem *oItem, const long long &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toLongLong() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QTableWidgetItem *oItem, const char *value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toByteArray() != value)
    {
        QByteArray pData(value);
        oItem->setData(Qt::UserRole, pData);
        oItem->setText(QStringHelper::ToString(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QTableWidgetItem *oItem, const std::string &value)
{
    return setItemValue(oItem, value.c_str());
}

int QWidgetHelper::setItemValue(QTableWidgetItem *oItem, const QString &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toString() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(value);
        return TRUE;
    }
    return FALSE;
}

void QWidgetHelper::setItemValueDateTime(QTableWidgetItem *oItem, const long long &value)
{
    if (!oItem) return;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toLongLong() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QStringHelper::ToString(value));
    }
}

int QWidgetHelper::getItemValue16(QTableWidgetItem *oItem, const int &value)
{
    if (oItem)
    {
        QVariant data = oItem->data(Qt::UserRole);
        if (!data.isValid())
        {
            return data.toInt();
        }
    }
    return value;
}

int QWidgetHelper::getItemValue(QTableWidgetItem *oItem, const int &value)
{
    if (oItem)
    {
        QVariant data = oItem->data(Qt::UserRole);
        if (!data.isValid())
        {
            return data.toInt();
        }
    }
    return value;
}

double QWidgetHelper::getItemValue(QTableWidgetItem *oItem, const double &value)
{
    if (oItem)
    {
        QVariant data = oItem->data(Qt::UserRole);
        if (!data.isValid())
        {
            return data.toDouble();
        }
    }
    return value;
}

long long QWidgetHelper::getItemValue(QTableWidgetItem *oItem, const long long &value)
{
    if (oItem)
    {
        QVariant data = oItem->data(Qt::UserRole);
        if (!data.isValid())
        {
            return data.toLongLong();
        }
    }
    return value;
}

std::string QWidgetHelper::getItemValue(QTableWidgetItem *oItem, const std::string &value)
{
    if (oItem)
    {
        QVariant data = oItem->data(Qt::UserRole);
        if (!data.isValid())
        {
            return data.toString().toStdString();
        }
    }
    return value;
}

QString QWidgetHelper::getItemValue(QTableWidgetItem *oItem, const QString &value)
{
    if (oItem)
    {
        QVariant data = oItem->data(Qt::UserRole);
        if (!data.isValid())
        {
            return data.toString();
        }
    }
    return value;
}

long long QWidgetHelper::getItemValueDateTime(QTableWidgetItem *oItem, const long long &value)
{
    if (oItem)
    {
        QVariant data = oItem->data(Qt::UserRole);
        if (!data.isValid())
        {
            return data.toLongLong();
        }
    }
    return value;
}


int QWidgetHelper::setItemValue16(QListWidgetItem *oItem, const int &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value, 16));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QListWidgetItem *oItem, const int &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QListWidgetItem *oItem, const double &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toDouble() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value, 'g', 3));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QListWidgetItem *oItem, const long long &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toLongLong() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QListWidgetItem *oItem, const char *value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toByteArray() != value)
    {
        QByteArray pData(value);
        oItem->setData(Qt::UserRole, pData);
        oItem->setText(QStringHelper::ToString(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QListWidgetItem *oItem, const std::string &value)
{
    return setItemValue(oItem, QStringHelper::gbkToQString(value));
}

int QWidgetHelper::setItemValue(QListWidgetItem *oItem, const QString &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toString() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(value);
        return TRUE;
    }
    return FALSE;
}

void QWidgetHelper::setItemValueDateTime(QListWidgetItem *oItem, const long long &value)
{
    if (!oItem) return;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toLongLong() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QStringHelper::ToString(value));
    }
}

int QWidgetHelper::setItemValue16(QStandardItem *oItem, const int &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(value, Qt::UserRole);
        oItem->setText(QString::number(value, 16));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QStandardItem *oItem, const int &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(value, Qt::UserRole);
        oItem->setText(QString::number(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QStandardItem *oItem, const double &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toDouble() != value)
    {
        oItem->setData(value, Qt::UserRole);
        oItem->setText(QString::number(value, 'g', 3));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QStandardItem *oItem, const long long &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toLongLong() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QString::number(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QStandardItem *oItem, const char *value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toByteArray() != value)
    {
        QByteArray pData(value);
        oItem->setData(pData, Qt::UserRole);
        oItem->setText(QStringHelper::ToString(value));
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValue(QStandardItem *oItem, const std::string &value)
{
    return setItemValue(oItem, QStringHelper::gbkToQString(value));
}

int QWidgetHelper::setItemValue(QStandardItem *oItem, const QString &value)
{
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toString() != value)
    {
        oItem->setData(value, Qt::UserRole);
        oItem->setText(value);
        return TRUE;
    }
    return FALSE;
}

int QWidgetHelper::setItemValueEditRole(QStandardItem *oItem, const QString &value)
{
    if (!oItem) return FALSE;
    QVariant data = oItem->data(Qt::EditRole);
    if (!data.isValid() || data.toString() != value)
    {
        oItem->setData(value, Qt::EditRole);
//        oItem->setText(value);
        return TRUE;
    }
    return FALSE;
}

void QWidgetHelper::setItemValueDateTime(QStandardItem *oItem, const long long &value)
{
    if (!oItem) return;
    QVariant data = oItem->data(Qt::UserRole);
    if (!data.isValid() || data.toLongLong() != value)
    {
        oItem->setData(Qt::UserRole, value);
        oItem->setText(QStringHelper::ToString(value));
        return;
    }
    return;
}

std::vector<int> QWidgetHelper::getSelectRows(QTableWidget *oGrid, QString *pRows)
{
    std::vector<int> r;
    if (!oGrid) return r;
    QItemSelectionModel *selections = oGrid->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();

    int iRow;
            foreach (QModelIndex index, selected)
        {
            iRow = index.row();
            if (!CxContainer::contain(r, iRow))
            {
                if (pRows)
                {
                    *pRows += QString::fromUtf8(" %1 ,").arg(iRow);
                }
                r.push_back(iRow);
            }
        }

    CxContainer::sort(r, false);

    return r;
}

std::vector<int> QWidgetHelper::deleteSelectRows(QTableWidget *oGrid)
{
    QString sRows;
    vector<int> iRows = getSelectRows(oGrid, &sRows);

    if (iRows.size() > 0)
    {
        QString sMsg = QString::fromUtf8("? Do you sure delete row: \n  %1").arg(sRows);
        if (QDialogHelper::ShowQuery(sMsg))
        {
            for (size_t i = 0; i < iRows.size(); ++i)
            {
                int iRow = iRows.at(i);
                oGrid->removeRow(iRow);
            }
        }
        else
        {
            iRows.clear();
        }
    }
    return iRows;
}

void QWidgetHelper::setQSS(QWidget *widget, const QString &fp)
{
    QFile file(fp);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    widget->setStyleSheet(styleSheet);
}

void QWidgetHelper::setQSS(QWidget *widget, const string &fp)
{
    QFile file(QStringHelper::gbkToQString(fp));
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    widget->setStyleSheet(styleSheet);
}


void QWidgetHelper::setItemValueUserEx(QTableWidgetItem *oItem, const int id, const int value)
{
    if (!oItem || id < 1) return;
    QVariant data = oItem->data(Qt::UserRole + id);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(Qt::UserRole + id, value);
    }
}

int QWidgetHelper::getItemValueUserEx(QTableWidgetItem *oItem, const int id, const int value)
{
    if (oItem && id > 0)
    {
        QVariant data = oItem->data(Qt::UserRole + id);
        if (!data.isValid())
        {
            return data.toInt();
        }
    }
    return value;
}

void QWidgetHelper::setItemValueUserEx(QListWidgetItem *oItem, const int id, const int value)
{
    if (!oItem || id < 1) return;
    QVariant data = oItem->data(Qt::UserRole + id);
    if (!data.isValid() || data.toInt() != value)
    {
        oItem->setData(Qt::UserRole + id, value);
    }
}

int QWidgetHelper::getItemValueUserEx(QListWidgetItem *oItem, const int id, const int value)
{
    if (oItem && id > 0)
    {
        QVariant data = oItem->data(Qt::UserRole + id);
        if (!data.isValid())
        {
            return data.toInt();
        }
    }
    return value;
}
