#include "fileconvert_demo.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
	/********** only for qt4.X **********/
	//QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );
	//QTextCodec::setCodecForLocale( QTextCodec::codecForName("UTF-8") );
	//QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
	
    FileConvert_demo w;
    w.show();
    
    return a.exec();
}
