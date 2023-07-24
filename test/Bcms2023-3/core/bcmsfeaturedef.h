#ifndef BCMSFEATUREDEF_H
#define BCMSFEATUREDEF_H

#include <QObject>

#include "qdebug.h"

class BcmsFeaturedef : public QObject {
    Q_OBJECT

   public:
    BcmsFeaturedef(const BcmsFeaturedef&) = delete;
    BcmsFeaturedef& operator=(const BcmsFeaturedef&) = delete;
    explicit BcmsFeaturedef(QObject* parent = nullptr);
    static BcmsFeaturedef& instance() {
        static BcmsFeaturedef ins;
        return ins;
    }

    QString rowData() { return *__rowData; }

   signals:

   private:
    //! 圖層設定
    QString* __rowData = nullptr;
#ifdef DEVELOPING

   private:
    ~BcmsFeaturedef() { qDebug() << "BcmsFeaturedef destructor called!"; }
    // BcmsFeaturedef() { qDebug() << "BcmsFeaturedef constructor called!"; }
#endif
};

// class Car1 {
//    public:
//     static Car1* instance() {
//         if (!ins) {
//             ins = new Car1();
//         }
//         return ins;
//     }

//    private:
//     static Car1* ins;
// };

// class Car2 {
//    public:
//     ~Car2() { qDebug() << "destructor called!"; }
//     Car2(const Car2&) = delete;
//     Car2& instance() {
//         static Car2 ins;
//         return ins;
//     }

//    private:
//     Car2() { qDebug() << "constructor called!"; }
// };

#endif  // BCMSFEATUREDEF_H
