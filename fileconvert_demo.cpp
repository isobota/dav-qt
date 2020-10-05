#include "fileconvert_demo.h"
#include "ui_fileconvert_demo.h"
#include <QFileDialog>
#include <QMessageBox>
#include <unistd.h>
#include <sstream>
#include <cstring>

#define LANG_CS(a)              a
#define PORT                    0
#define SOURCEBUF_SIZE          (500 * 1024)

FileConvert_demo::FileConvert_demo(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FileConvert_demo),
	m_convertThread(NULL),
	m_isConverting(false),
	m_nChangeCount(0),
	m_bSegment(false),
	m_nSegmentSize(0)
{
	m_nConvertType = AVI_CONVERT;
	m_csSourceFile = "";
	m_csTargetFile = "";

    ui->setupUi(this);
    ui->ConvertProgress->setRange(0, 500);
    ui->ConvertProgress->setValue(0);
	QStringList strList;
    strList << QString::fromUtf8("Not Segmented") << QString::fromUtf8("Segmented");
	ui->comboBox_Seg->addItems(strList);

    connect(this, SIGNAL(startConvertFailed()), this, SLOT(onStartConvertFailed()), Qt::QueuedConnection);
    connect(this, SIGNAL(openFileFailed()), this, SLOT(onOpenFileFailed()), Qt::QueuedConnection);
    connect(this, SIGNAL(convertCompleted()), this, SLOT(onConvertCompleted()), Qt::QueuedConnection);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(changeUIState(int)), this, SLOT(onChangeUIState(int)), Qt::QueuedConnection);
	connect(ui->comboBox_Seg, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboxSegmentChange(int)));

    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
}

FileConvert_demo::~FileConvert_demo()
{
    delete ui;
}

void FileConvert_demo::onStartConvertFailed()
{
    QMessageBox::warning(this, "FileConvert_demo", LANG_CS("Convert Failed!"));
}

void FileConvert_demo::onOpenFileFailed()
{
	QMessageBox::warning(this, "FileConvert_demo", LANG_CS("OpenFile Failed!"));
}

void FileConvert_demo::onConvertCompleted()
{
    QMessageBox::warning(this, "FileConvert_demo", LANG_CS("Convert Complete!"));
}

void FileConvert_demo::onValueChanged(int value)
{
    ui->ConvertProgress->setValue(value);
}

void FileConvert_demo::on_StartConvert_clicked()
{
    if (NULL != m_convertThread)
    {
        m_isConverting = false;

        if (m_convertThread->isRunning())
        {
            m_convertThread->wait();
        }

        delete m_convertThread;
        m_convertThread = NULL;
    }

    m_nChangeCount = 0;

    m_convertThread = new FileConvertThread(this, this);
    m_convertThread->start();

    m_isConverting = true;
}

void FileConvert_demo::run()
{
    QFile srcFile(ui->SourcePathEdit->text());
    if (!srcFile.open(QFile::ReadOnly))
    {
        emit openFileFailed();
        return;
    }

    if (!startConvert())
    {
        srcFile.close();
        return;
    }

    emit changeUIState(CONVERT);

    qint64 fileLen = srcFile.size();
    qint64 readPos = 0;
    const int READ_LEN = 8 * 1024;
    unsigned char readBuf[READ_LEN] = {0};

    while (m_isConverting)
    {
        qint64 readLen = srcFile.read((char*)readBuf, READ_LEN);
        if (readLen <= 0)
        {
            break;
        }

        while (!PLAY_InputData(PORT, readBuf, readLen) && m_isConverting)
        {
			usleep(10);
        }

        readPos += readLen;

        if (m_isConverting)
        {
            double proPercent = ((double)readPos / (double)fileLen) * 500;
            emit valueChanged(proPercent);
        }
    }

    if (m_isConverting)
    {
        emit convertCompleted();
        emit changeUIState(CANCEL);
    }

    while ((PLAY_GetBufferValue(PORT, BUF_VIDEO_RENDER) + PLAY_GetSourceBufferRemain(PORT)) > 0)
    {
        usleep(5);
    }

    stopConvert();
    srcFile.close();
}

int FileConvert_demo::convertType(int nDataType)
{
	int nConvertType = 0;
	switch (nDataType)
	{
	case AVI_CONVERT:
		nConvertType = DATA_RECORD_AVI;
		break;
	case ASF_CONVERT:
		nConvertType = DATA_RECORD_ASF;
		break;
	case MP4_CONVERT:
		nConvertType = DATA_RECORD_MP4;
		break;
	case PS_CONVERT:
		nConvertType = DATA_RECORD_PS;
		break;
	case TS_CONVERT:
		nConvertType = DATA_RECORD_TS;
		break;
	case DAV_CONVERT:
		nConvertType = DATA_RECORD_ORIGINAL;
		break;
	}
	return nConvertType;
}

void CALLMETHOD SegmentRecordDataCBFun(LONG nPort, char* pSegmentFileName, unsigned int nFileSize, void* pUserData)
{
	std::stringstream outputStringStream;
	char fileName[1024] = "";
	strncpy(fileName, (char*)pSegmentFileName, strlen(pSegmentFileName));
}

bool FileConvert_demo::startConvert()
{
    PLAY_SetStreamOpenMode(PORT, STREAME_FILE);
    PLAY_OpenStream(PORT, NULL, 0, SOURCEBUF_SIZE);
    PLAY_Play(PORT, NULL);

	if (m_bSegment && 0 != m_nSegmentSize)
	{
		PLAY_SetSegmentRecordData(PORT, m_nSegmentSize, SegmentRecordDataCBFun, NULL);
	}

    BOOL retValue = PLAY_StartDataRecordEx(PORT, (char*)ui->DestPathEdit->text().toStdString().c_str(), convertType(m_nConvertType),NULL,NULL);

    if (!retValue)
    {
        // Trigger failure event
        emit startConvertFailed();
        m_isConverting = false;
        PLAY_Stop(PORT);
        PLAY_CloseStream(PORT);

        return false;
    }

    return true;
}

void FileConvert_demo::stopConvert()
{
    PLAY_StopDataRecord(PORT);

    PLAY_Stop(PORT);
    PLAY_CloseStream(PORT);
}

void FileConvert_demo::on_StopConvert_clicked()
{
    m_isConverting = false;
    if (NULL != m_convertThread)
    {
        m_convertThread->wait();
    }

    m_nChangeCount = 0;

    onChangeUIState(CANCEL);
}

void FileConvert_demo::closeEvent(QCloseEvent *event)
{
    on_StopConvert_clicked();
}



void FileConvert_demo::on_BrowseSourcePath_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, LANG_CS("Select one file to open"), ui->SourcePathEdit->text(), "DAV (*.dav);;All Files(*.*)");
    if (fileName.isEmpty())
    {
        return;
    }
    m_csSourceFile = fileName;
    ui->SourcePathEdit->setText(fileName);

	m_csTargetFile = m_csSourceFile;

	QString sNameExt="";
	switch(m_nConvertType)
	{
	case AVI_CONVERT:
		sNameExt += ".avi";
		break;
	case ASF_CONVERT:
		sNameExt += ".asf";
		break;
	case MP4_CONVERT:
        sNameExt += ".mp4";
		break;
	case PS_CONVERT:
		sNameExt += ".ps";
		break;
	case TS_CONVERT:
		sNameExt += ".ts";
		break;
	case DAV_CONVERT:
		sNameExt += ".dav";
		break;
	default:
		break;
	}
    int pos = m_csTargetFile.lastIndexOf('.');
	if (pos > 0)
	{
		QString tempFileName = m_csTargetFile.mid(0,pos);
		m_csTargetFile = tempFileName + sNameExt;
	}
	ui->DestPathEdit->setText(m_csTargetFile);
}

void FileConvert_demo::on_BrowseDestPath_clicked()
{
    QString strFilter;
    QString strDefault = ui->DestPathEdit->text();
    strFilter.append("All files (*.*)");
    if (strDefault.isEmpty())
    {
        strDefault.append("/home/convert.avi");
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save file", strDefault, strFilter);
    if (!fileName.isEmpty())
    {
        m_csTargetFile = fileName;
		QString sNameExt="";
		switch(m_nConvertType)
		{
			case AVI_CONVERT:
				sNameExt += ".avi";
				break;
			case ASF_CONVERT:
				sNameExt += ".asf";
				break;
			case MP4_CONVERT:
                sNameExt += ".mp4";
				break;
			case PS_CONVERT:
				sNameExt += ".ps";
				break;
			case TS_CONVERT:
				sNameExt += ".ts";
				break;
			default:
				break;
		}
        int pos = m_csTargetFile.lastIndexOf('.');
		if (pos > 0)
		{
			QString tempFileName = m_csTargetFile.mid(0,pos);
			m_csTargetFile = tempFileName + sNameExt;
		}
		ui->DestPathEdit->setText(m_csTargetFile);
    }
}

void FileConvert_demo::changeExtensionName(const QString& srcExtName, const QString& dstExtName)
{
    if (ui->DestPathEdit->text().indexOf(dstExtName) > 0)
    {
        return;
    }

    QString fileName = ui->DestPathEdit->text();
    int index = ui->DestPathEdit->text().lastIndexOf(srcExtName);
    if (index < 0)
    {
        index = ui->DestPathEdit->text().lastIndexOf(dstExtName);
    }

    if (index >= 0)
    {
        fileName = ui->DestPathEdit->text().mid(0, index);
        ui->DestPathEdit->setText(fileName + dstExtName);
    }
}

void FileConvert_demo::onChangeUIState(int state)
{
    bool isEnable = (state == CONVERT) ? false : true;

    ui->StopConvert->setEnabled(!isEnable);
    ui->StartConvert->setEnabled(isEnable);
    ui->DestPathEdit->setEnabled(isEnable);
    ui->SourcePathEdit->setEnabled(isEnable);
    ui->BrowseSourcePath->setEnabled(isEnable);
    ui->BrowseDestPath->setEnabled(isEnable);

    ui->comboBox_Seg->setEnabled(isEnable);  //Segment drop-down bar status
	isEnable = isEnable && m_bSegment;
	ui->radioBtnDAV->setEnabled(isEnable);
	ui->label_Seg_Size->setEnabled(isEnable);
	ui->lineEdit_Seg_Size->setEnabled(isEnable);
	ui->label_Seg_Units->setEnabled(isEnable);

    if (state == CANCEL)
    {
        emit valueChanged(0);
    }
}

void FileConvert_demo::on_radioBtnAVI_clicked()
{
	if (m_nConvertType == AVI_CONVERT)
		return;

	m_nConvertType = AVI_CONVERT;
	if (m_csTargetFile != "")
	{
		QString tempFileName = m_csTargetFile;
        int pos = m_csTargetFile.lastIndexOf('.');
		if (pos > 0)
		{
			tempFileName = m_csTargetFile.mid(0,pos);
		}
		m_csTargetFile = tempFileName + ".avi";
		ui->DestPathEdit->setText(m_csTargetFile);
	}
}

void FileConvert_demo::on_radioBtnASF_clicked()
{
	if (m_nConvertType == ASF_CONVERT)
		return;

	m_nConvertType = ASF_CONVERT;
	if (m_csTargetFile != "")
	{
		QString tempFileName = m_csTargetFile;
        int pos = m_csTargetFile.lastIndexOf('.');
		if (pos > 0)
		{
			tempFileName = m_csTargetFile.mid(0,pos);
		}
		m_csTargetFile = tempFileName + ".asf";
		ui->DestPathEdit->setText(m_csTargetFile);
	}
}

void FileConvert_demo::on_radioBtnMP4_clicked()
{
	if (m_nConvertType == MP4_CONVERT)
		return;

	m_nConvertType = MP4_CONVERT;
	if (m_csTargetFile != "")
	{
		QString tempFileName = m_csTargetFile;
        int pos = m_csTargetFile.lastIndexOf('.');
		if (pos > 0)
		{
			tempFileName = m_csTargetFile.mid(0,pos);
		}
		m_csTargetFile = tempFileName + ".mp4";
		ui->DestPathEdit->setText(m_csTargetFile);
	}
}

void FileConvert_demo::on_radioBtnTS_clicked()
{
	if (m_nConvertType == TS_CONVERT)
		return;

	m_nConvertType = TS_CONVERT;
	if (m_csTargetFile != "")
	{
		QString tempFileName = m_csTargetFile;
        int pos = m_csTargetFile.lastIndexOf('.');
		if (pos > 0)
		{
			tempFileName = m_csTargetFile.mid(0,pos);
		}
		m_csTargetFile = tempFileName + ".ts";
		ui->DestPathEdit->setText(m_csTargetFile);
	}
}

void FileConvert_demo::on_radioBtnPS_clicked()
{
	if (m_nConvertType == PS_CONVERT)
		return;

	m_nConvertType = PS_CONVERT;
	if (m_csTargetFile != "")
	{
		QString tempFileName = m_csTargetFile;
        int pos = m_csTargetFile.lastIndexOf('.');
		if (pos > 0)
		{
			tempFileName = m_csTargetFile.mid(0,pos);
		}
		m_csTargetFile = tempFileName + ".ps";
		ui->DestPathEdit->setText(m_csTargetFile);
	}
}

void FileConvert_demo::on_radioBtnDAV_clicked()
{
	if (m_nConvertType == DAV_CONVERT)
		return;

	m_nConvertType = DAV_CONVERT;
	if (m_csTargetFile != "")
	{
		QString tempFileName = m_csTargetFile;
		int pos = m_csTargetFile.lastIndexOf('.');
		if (pos > 0)
		{
			tempFileName = m_csTargetFile.mid(0, pos);
		}
		m_csTargetFile = tempFileName + ".dav";
		ui->DestPathEdit->setText(m_csTargetFile);
	}
}

void FileConvert_demo::on_lineEdit_Seg_Size_textChanged(const QString &arg1)
{ 
    if (!m_bSegment)   // If you don’t do segmentation, just exit
	{
		return;
	}

	bool ok = true;
	QString tempstr;
	QString valuestr = ui->lineEdit_Seg_Size->text();
	m_nSegmentSize = valuestr.toInt(&ok);

	if (!ok)
	{
		return;
	}

	if (m_nSegmentSize <= 0)
	{
		
		m_nSegmentSize = 0;
	}
	else if (m_nSegmentSize > 10000)
	{
		QMessageBox::warning(this, "FileConvert_demo", LANG_CS("Segment Size is too large!"));
		m_nSegmentSize = 0;
	}
}


void FileConvert_demo::onComboxSegmentChange(int value)
{
    if (0 == value)  //Not Segmented
	{
		m_bSegment = false;
	}
    else             //Segmented
	{
		m_bSegment = true;
	}

	ui->radioBtnDAV->setEnabled(m_bSegment);
	ui->label_Seg_Size->setEnabled(m_bSegment);
	ui->lineEdit_Seg_Size->setEnabled(m_bSegment);
	ui->label_Seg_Units->setEnabled(m_bSegment);

    //Reset radiobutton
	ui->radioBtnAVI->setChecked(true);
	ui->radioBtnASF->setChecked(false);
	ui->radioBtnMP4->setChecked(false);
	ui->radioBtnTS->setChecked(false);
	ui->radioBtnPS->setChecked(false);
	ui->radioBtnDAV->setChecked(false);
}
