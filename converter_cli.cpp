#include "converter_cli.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QFile>
#include <unistd.h>
#include "fileconvertthread.h"
#include "dhplayEx.h"

#define PORT                    0
#define SOURCEBUF_SIZE          (500 * 1024)

converter_cli::converter_cli(QObject *parent):
    QObject(parent),
    m_convertThread(NULL),
    m_isConverting(false),
    m_nChangeCount(0)
{
    m_nConvertType = AVI_CONVERT;
    m_csSourceFile = "";
    m_csTargetFile = "";
}


void converter_cli::on_StartConvert(){
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


void converter_cli::run()
{
    QFile srcFile(m_csSourceFile);
    if (!srcFile.open(QFile::ReadOnly))
    {
        qDebug() << "failed";
        return;
    }
    if (!startConvert())
    {
        srcFile.close();
        return;
    }

    qint64 fileLen = srcFile.size();
    qint64 readPos = 0;
    const int READ_LEN = 8 * 1024;
    unsigned char readBuf[READ_LEN] = {0};

    qDebug() <<"running...";

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
            double proPercent = ((double)readPos / (double)fileLen) *100;
            qDebug() << proPercent;
        }
    }
    if(m_isConverting)
    {
        qDebug() << "Converting Completed";
        stopConvert();
        exit(0);
    }

    while ((PLAY_GetBufferValue(PORT, BUF_VIDEO_RENDER) + PLAY_GetSourceBufferRemain(PORT)) > 0)
    {
        usleep(5);
    }

    stopConvert();
    srcFile.close();
}


int converter_cli::convertType(int nDataType)
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

bool converter_cli::startConvert(){
    qDebug() << m_csTargetFile;
    qDebug() <<"In startConvert";

    PLAY_SetStreamOpenMode(PORT, STREAME_FILE);
    PLAY_OpenStream(PORT, NULL, 0, SOURCEBUF_SIZE);
    PLAY_Play(PORT, NULL);

    BOOL retValue = PLAY_StartDataRecordEx(PORT, (char*)m_csTargetFile.toStdString().c_str(),
                                           convertType(m_nConvertType),NULL,NULL);
    if (!retValue)
    {
        // Trigger failure event
        qDebug() <<"FAILED";
        m_isConverting = false;
        PLAY_Stop(PORT);
        PLAY_CloseStream(PORT);

        return false;
    }

    return true;
}

void converter_cli::stopConvert()
{
    PLAY_StopDataRecord(PORT);

    PLAY_Stop(PORT);
    PLAY_CloseStream(PORT);
}

void converter_cli::return_input_file(QString dummy) {
    m_csSourceFile = dummy;
}

void converter_cli::return_output_file(QString dummy) {
    m_csTargetFile = dummy;
}

void converter_cli::initConvert(){
    on_StartConvert();
}

