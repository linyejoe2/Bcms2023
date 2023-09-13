#include "bcmsfeaturedef.h"

#include <qgsfillsymbol.h>
#include <qgsfillsymbollayer.h>
#include <qgslinesymbollayer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbol.h>

#include <QJsonDocument>
#include <QJsonObject>

#include "../services/geoviewer.h"

//! 改寫 QJsonArray swap 方法
inline void swap(QJsonValueRef val1, QJsonValueRef val2) {
    QJsonValue temp(val1);
    val1 = QJsonValue(val2);
    val2 = temp;
}

BcmsFeaturedef::BcmsFeaturedef(QObject* parent) : QObject{parent} {
    //! init featuredef
    // 呼叫服務讀出 JSON
    auto __rowData =
        GeoViewer::instance()->connectApi("/api/v3/featuredef/get").toUtf8();
    __object = QJsonDocument::fromJson(__rowData);
    // qDebug() << "featuredef raw data: " << __rowData;

    // 按照高度排序圖層定義
    auto arr = __object.array();
    std::sort(arr.begin(), arr.end(),
              [](const QJsonValue& val1, const QJsonValue& val2) -> bool {
                  return val1.toObject()["z_index"].toInt() <
                         val2.toObject()["z_index"].toInt();
              });

    // 轉換 jsonArr to QVector
    foreach (const QJsonValue& val, arr) {
        if (val.isObject()) {
            auto obj = val.toObject();
            if (obj.contains("fname") && obj["fname"] != "LD")
                __idList.append(obj["fname"].toString());
        }
    }

#ifdef DEVELOPING
    QJsonArray* tempArr = GeoViewer::instance()->getFeaturedef();

    qDebug() << "### DEV001: "
             << "featuredef JSON test";

    qDebug() << "asdf" << tempArr->at(0).toObject().value("fdesc").toString();
    if (!tempArr->isEmpty()) {
        qDebug() << "is not empty";
        // 获取第一个元素的 QJsonObject
        QJsonObject firstObject = tempArr->at(0).toObject();

        // 检查 symbol_type 字段是否存在
        if (firstObject.contains("symbol_type")) {
            qDebug() << "has symbol_type";
            // 获取 symbol_type 字段的值
            QJsonValue symbolTypeValue = firstObject.value("symbol_type");

            // 检查 symbol_type 的值是否为数组
            if (symbolTypeValue.isArray()) {
                qDebug() << "symbol_type is array";
                QJsonArray symbolTypeArray = symbolTypeValue.toArray();

                // 检查数组中是否有至少一个元素
                if (!symbolTypeArray.isEmpty()) {
                    qDebug() << "symbol_type is not empty";
                    // 获取第一个元素的 QJsonObject
                    QJsonObject symbolTypeObject =
                        symbolTypeArray.at(0).toObject();

                    // 检查 Type 字段是否存在
                    if (symbolTypeObject.contains("Type")) {
                        // 获取 Type 字段的值
                        QString typeValue =
                            symbolTypeObject.value("Type").toString();
                        qDebug() << "Type:" << typeValue;
                    }
                }
            }
        }
    }
    qDebug() << "### END DEV001 ###";
#endif
}

QJsonObject BcmsFeaturedef::getSymbolConfig(const QString& fname) {
    auto tempArr = __object.array();

    foreach (const QJsonValue& value, tempArr) {
        if (value.isObject()) {
            auto obj = value.toObject();
            if (obj.contains("fname") && obj.value("fname") == fname) {
                return obj;
            }
        }
    }
}

QString BcmsFeaturedef::getSymbolDesc(const QString& fname) {
    auto tempArr = __object.array();

    foreach (const QJsonValue& value, tempArr) {
        if (value.isObject()) {
            auto obj = value.toObject();
            if (obj.contains("fname") && obj.value("fname") == fname) {
                return obj.value("fdesc").toString();
            }
        }
    }
}

QString BcmsFeaturedef::getSymbolByDesc(const QString& fdesc) {
    auto tempArr = __object.array();

    foreach (const QJsonValue& value, tempArr) {
        if (value.isObject()) {
            auto obj = value.toObject();
            if (obj.contains("fdesc") && obj.value("fdesc") == fdesc) {
                return obj.value("fname").toString();
            }
        }
    }
}

QString BcmsFeaturedef::getSymbolZIndex(const QString& fname) {
    auto tempArr = __object.array();

    foreach (const QJsonValue& value, tempArr) {
        if (value.isObject()) {
            auto obj = value.toObject();
            if (obj.contains("fname") && obj.value("fname") == fname) {
                return obj.value("z_index").toString();
            }
        }
    }
}

QgsFeatureRenderer* BcmsFeaturedef::getSymbolRenderer(const QString& fname) {
    auto tempArr = __object.array();

    QgsFillSymbol* fillSymbol = new QgsFillSymbol();

    foreach (const QJsonValue& value, tempArr) {
        if (value.isObject()) {
            auto obj = value.toObject();
            if (obj.contains("fname") &&
                obj.value("fname").toString() == fname) {
                // 對 fillSymbol 做某些設定
                auto symbolTypeArr = obj.value("symbol_type").toArray();
                for (int i = 0; i < symbolTypeArr.size(); i++) {
                    QJsonObject tSet = symbolTypeArr[i].toObject();
                    QgsPalLayerSettings layerLabelSetting;
                    if (tSet["Type"].toString() == "Simple Line") {
                        QgsSimpleLineSymbolLayer* lSL =
                            new QgsSimpleLineSymbolLayer();

                        // 設定顏色
                        lSL->setColor(QColor(tSet["Color"].toString()));
                        // 設定邊框寬度
                        lSL->setWidth(tSet["Stroke width"].toDouble());
                        // 設定邊框樣式
                        if (tSet["Stroke style"].isString()) {
                            if (tSet["Stroke style"].toString() ==
                                "Solid Line") {
                            }
                        }
                        if (i == 0) {
                            fillSymbol->changeSymbolLayer(0, lSL);
                        } else
                            fillSymbol->insertSymbolLayer(1, lSL);
                    } else if (tSet["Type"].toString() == "Simple Fill") {
                        QgsSimpleFillSymbolLayer* fSL =
                            new QgsSimpleFillSymbolLayer();

                        // 設定顏色
                        fSL->setColor(QColor(tSet["Color"].toString()));
                        // 設定填滿樣式
                        if (tSet["Fill style"].isString()) {
                            if (tSet["Fill style"].toString() == "Solid") {
                            } else if (tSet["Fill style"].toString() ==
                                       "BDiagonal") {
                                fSL->setBrushStyle(Qt::BDiagPattern);
                            }
                        }
                        // 設定邊框寬度
                        fSL->setStrokeWidth(tSet["Stroke width"].toDouble());
                        // 設定邊框樣式
                        if (tSet["Stroke style"].isString() &&
                            tSet["Stroke style"].toString() == "Solid Line") {
                            fSL->setStrokeStyle(Qt::SolidLine);
                        }
                        if (i == 0) {
                            fillSymbol->changeSymbolLayer(0, fSL);
                        } else
                            fillSymbol->insertSymbolLayer(1, fSL);
                    }
                }
            }
        }
    }
    return new QgsSingleSymbolRenderer(fillSymbol);
}
