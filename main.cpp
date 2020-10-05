

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
