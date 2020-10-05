/*
 * Copyright © 2020, Vibhoothi < mindfreeze at xiph dot org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
