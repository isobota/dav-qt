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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "converter_cli.h"
#include <QDebug>
#include <QFile>
#include "fileconvertthread.h"
#include "dhplayEx.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    converter_cli converter;
    QCoreApplication::setApplicationName("DAV Converter");
    QCoreApplication::setApplicationVersion("0.1.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("DAV Converter"
                                     "Simple CLI Tool to convert any DAV format to AVI/MP4");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption  input(QStringList() << "i" << "input",
                              QCoreApplication::translate("main", "Select input file .dav/.DAV" ), "file", "");
    parser.addOption(input);

    QCommandLineOption  output(QStringList() << "o" << "output",
                               QCoreApplication::translate("main", "Set output name" ), "outputname", "");
    parser.addOption(output);

    QCommandLineOption  convert_type(QStringList() << "t" << "convert",
                                     QCoreApplication::translate("main", "Chose Conversion Type (AVI/MP4/DAV)" ),
                                     "convertype", "");
    parser.addOption(convert_type);
    parser.process(app);
    if(parser.value(convert_type).toStdString() == "mp4"){
        converter.m_nConvertType = MP4_CONVERT;
    }
    if(parser.value(convert_type).toStdString() == "avi"){
        converter.m_nConvertType = AVI_CONVERT;
    }
    if(parser.value(convert_type).toStdString() == "dav"){
        converter.m_nConvertType = DAV_CONVERT;
    }
    if(parser.value(convert_type).toStdString() == "ps"){
        converter.m_nConvertType = PS_CONVERT;
    }
    if(parser.value(convert_type).toStdString() == "ts"){
        converter.m_nConvertType = TS_CONVERT;
    }

    QString parsed_input = parser.value(input);
    QString parsed_output = parser.value(output);
    converter.return_input_file(parsed_input);
    converter.return_output_file(parsed_output);

    if(parser.value(input).isNull()||parser.value(output).isNull())
    {
        qDebug() << "Missing Args";
        return 1;
    }
    converter.initConvert();

    return app.exec();

}
