#include "bcmsloadform.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardItemModel>

#include "core/const.hpp"
#include "ui_bcmsloadform.h"

BcmsLoadForm::BcmsLoadForm(QWidget *parent)
    : QDockWidget(parent), ui(new Ui::BcmsLoadForm) {
    ui->setupUi(this);

    this->setWindowTitle(QStringLiteral("地段載入器"));

    fetchLandCodeFromAPI();

    //! 載入選擇地段
    connect(ui->loadBtn, SIGNAL(clicked()), this, SLOT(load()));
    //! 每次修改選擇的地段就更新一次畫面
    connect(ui->zonComboBox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(refreshUI()));
    //! 查詢地籍預覽資料
    connect(ui->searchBtn, SIGNAL(clicked()), this, SLOT(loadLandData()));
    //! 清除畫面
    connect(ui->cleanBtn, SIGNAL(clicked()), this, SLOT(cleanWidget()));
    //! 關閉視窗
    connect(ui->cancelBtn, SIGNAL(clicked()), this, SLOT(closeWidget()));
    // connect(ui->sectionComboBox, SIGNAL(currentIndexChanged(int)), this,
    // SLOT(refreshUI()));

    ui->tableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
}

BcmsLoadForm::~BcmsLoadForm() { delete ui; }

void BcmsLoadForm::load() {
    auto qIndex = ui->tableView->currentIndex();

    selectedLand.landmon =
        ui->tableView->model()->index(qIndex.row(), 2).data().toString();
    selectedLand.landchild =
        ui->tableView->model()->index(qIndex.row(), 3).data().toString();

    emit loadSignal(selectedLand);

    //! 最後關閉載入視窗
    closeWidget();
}

void BcmsLoadForm::refreshUI() {
    model->clear();
    refreshSectionSelected();
}

void BcmsLoadForm::cleanWidget() {
    ui->zonComboBox->setCurrentIndex(0);
    refreshUI();
}

void BcmsLoadForm::loadLandData() {
    //! 例外退出
    if (ui->sectionComboBox->currentData() == "000") return;
    //! 清除輸出
    model->clear();
    // ui->tableView->setModel(model);

    QString url = CONST::instance()->getGeoViewerJSServiceUrl() +
                  "/api/v3/land/preview/" +
                  ui->zonComboBox->currentData().toString() + "/" +
                  ui->sectionComboBox->currentData().toString();

    //! get data
    QNetworkAccessManager manager;
    QNetworkReply *res = manager.get(QNetworkRequest(url));
    QEventLoop event;
    connect(res, &QNetworkReply::finished, &event, &QEventLoop::quit);
    event.exec();
    QString jsonString = res->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonResponse.array();

    //! 例外退出
    if (jsonArray.size() == 0) return;

    // 创建数据模型
    // QStandardItemModel *model = new QStandardItemModel(this);

    // 设置表头
    model->setHorizontalHeaderLabels(
        {QStringLiteral("地區"), QStringLiteral("地段"), QStringLiteral("母號"),
         QStringLiteral("子號")});

    // 从QJsonArray中提取数据并添加到数据模型中
    foreach (const QJsonValue &value, jsonArray) {
        QJsonObject obj = value.toObject();
        QString zon = obj.value("zon").toString() + "-" +
                      findZonDesc(obj.value("zon").toString());
        QString section = obj.value("section").toString() + "-" +
                          findSectionDesc(obj.value("section").toString());
        QString landmon = obj.value("landmon").toString();
        QString landchild = obj.value("landchild").toString();

        QList<QStandardItem *> rowData;
        rowData.append(new QStandardItem(zon));
        rowData.append(new QStandardItem(section));
        rowData.append(new QStandardItem(landmon));
        rowData.append(new QStandardItem(landchild));

        model->appendRow(rowData);
    }

    // 将数据模型设置给QTableView
    ui->tableView->setModel(model);
}

void BcmsLoadForm::refreshSectionSelected() {
    //! check if zonComboBox selected
    if (ui->zonComboBox->currentData() == "000") {
        ui->sectionComboBox->clear();
        ui->sectionComboBox->addItem("", "000");
    } else {
        ui->sectionComboBox->clear();

        foreach (const auto ele, mSectionList) {
            if (ele.zon == ui->zonComboBox->currentData())
                ui->sectionComboBox->addItem(ele.section + ": " + ele.desc,
                                             ele.section);
        }
    }

    //! 清除輸出
    ui->tableView->clearSpans();
}

void BcmsLoadForm::fetchLandCodeFromAPI() {
    //! fetch ZON
    QNetworkAccessManager manager;
    QNetworkReply *res = manager.get(
        QNetworkRequest(CONST::instance()->getGeoViewerJSServiceUrl() +
                        "/api/v3/bldcode/get/zon"));
    QEventLoop event;
    connect(res, &QNetworkReply::finished, &event, &QEventLoop::quit);
    event.exec();
    QString jsonString = res->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonResponse.array();
    foreach (const QJsonValue &value, jsonArray) {
        QJsonObject obj = value.toObject();
        mZonList.push_back({obj.value("code_seq").toString(), "",
                            obj.value("code_desc").toString()});
    }
    std::sort(
        mZonList.begin(), mZonList.end(),
        [](const ILandCode &a, const ILandCode &b) { return a.zon < b.zon; });

    //// qDebug() << mZonList[10].id << mZonList[10].name;

    //! fetch section
    res = manager.get(
        QNetworkRequest(CONST::instance()->getGeoViewerJSServiceUrl() +
                        "/api/v3/bldcode/get/sec"));
    connect(res, &QNetworkReply::finished, &event, &QEventLoop::quit);
    event.exec();
    jsonString = res->readAll();
    jsonResponse = QJsonDocument::fromJson(jsonString.toUtf8());
    jsonArray = jsonResponse.array();
    foreach (const QJsonValue &val, jsonArray) {
        QJsonObject obj = val.toObject();
        mSectionList.push_back({obj.value("code_seq").toString(),
                                obj.value("sub_seq").toString(),
                                obj.value("code_desc").toString()});
    }
    std::sort(mSectionList.begin(), mSectionList.end(),
              [](const ILandCode &a, const ILandCode &b) {
                  return a.section < b.section;
              });

    //! insert into comboBox
    ui->zonComboBox->addItem("", "000");
    foreach (const auto ele, mZonList) {
        ui->zonComboBox->addItem(ele.zon + ": " + ele.desc, ele.zon);
    }
    // foreach (const auto ele, mSectionList) {
    //     ui->sectionComboBox->addItem(ele.section + ": " + ele.desc,
    //                                  ele.section);
    // }
}

QString BcmsLoadForm::findZonDesc(const QString &zon) {
    auto it =
        std::find_if(mZonList.begin(), mZonList.end(),
                     [&zon](const ILandCode &code) { return code.zon == zon; });

    if (it != mZonList.end()) {
        return it->desc;
    }

    return QString();  // 如果找不到，则返回一个空字符串
}

QString BcmsLoadForm::findSectionDesc(const QString &section) {
    auto it = std::find_if(
        mSectionList.begin(), mSectionList.end(),
        [&section](const ILandCode &code) { return code.section == section; });

    if (it != mSectionList.end()) {
        return it->desc;
    }

    return QString();  // 如果找不到，则返回一个空字符串
}
