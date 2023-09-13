#include <qtextedit.h>

#include <QApplication>

#include "bcmsapp.h"

void customMessageHandler(QtMsgType type, const QMessageLogContext &context,
                          const QString &msg) {
    QString logText;

    // 根據日誌類型設置不同的標籤
    switch (type) {
        case QtDebugMsg:
            logText = "Debug: " + msg;
            break;
        case QtWarningMsg:
            logText = "Warning: " + msg;
            break;
        case QtCriticalMsg:
            logText = "Critical: " + msg;
            break;
        case QtFatalMsg:
            logText = "Fatal: " + msg;
            break;
        case QtInfoMsg:
            logText = "Info: " + msg;
            break;
        default:
            logText = "Other: " + msg;
            break;
    }

    // 獲取當前日期和時間
    QString currentDateTime = QDateTime::currentDateTime().toString("yy_MM_dd");

    // 打開日誌文件，以追加的方式寫入日誌
    QFile file("./debug/" + currentDateTime + "_log");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream textStream(&file);
        textStream.setCodec("UTF-8");
        textStream << logText << "\n";
        file.close();
    }
}

int main(int argc, char *argv[]) {
    // 設置自定義的日誌處理函式
    qInstallMessageHandler(customMessageHandler);

    QApplication::setStyle("Fusion");
    QApplication a(argc, argv);
    BcmsApp w;
    QIcon appIcon(":/img/bcms/bcms2009.ico");
    w.setWindowIcon(appIcon);

    // qDebug() << "keys--" << QStyleFactory::keys();
    w.show();
    return a.exec();
}
