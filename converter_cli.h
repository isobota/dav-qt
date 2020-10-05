#ifndef MAIN_H
#define MAIN_H
#include "fileconvertthread.h"
#include "dhplayEx.h"
#include<QFile>
typedef enum
{
    AVI_CONVERT,
    ASF_CONVERT,
    MP4_CONVERT,
    PS_CONVERT,
    TS_CONVERT,
    DAV_CONVERT
}ConvertType;

class converter_cli: public QObject, public IRunnable
{
    Q_OBJECT

public:
    explicit converter_cli(QObject *parent =0);

private slots:

    int convertType(int);
    void on_StartConvert();

public:
        virtual void run();

public:
    void stopConvert();
    void initConvert();
    bool startConvert();
    void return_input_file(QString);
    void return_output_file(QString);
    QString m_csSourceFile;
    QString m_csTargetFile;
    int  m_nConvertType;


private:
    FileConvertThread    *m_convertThread;
    bool                 m_isConverting;
    int                  m_nChangeCount;


};

#endif // MAIN_H
