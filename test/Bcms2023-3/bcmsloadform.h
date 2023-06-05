#ifndef BCMSLOADFORM_H
#define BCMSLOADFORM_H

#include <QDockWidget>
#include <QStandardItemModel>

namespace Ui {
class BcmsLoadForm;
}

struct ILandCode {
    QString zon;
    QString section;
    QString desc;
    QString zonDesc;
    QString sectionDesc;
    QString landmon;
    QString landchild;
};

class BcmsLoadForm : public QDockWidget {
    Q_OBJECT

   public:
    explicit BcmsLoadForm(QWidget *parent = nullptr);
    ~BcmsLoadForm();

   signals:

    void loadSignal(const ILandCode &land);

   private slots:

    //! 載入該地段
    void load();

    //! 每次畫面需要更新就扣這支，會自動判斷哪些東西需要重製等等的。
    void refreshUI();

    //! 按照選取區段載入顯示地段資料
    void loadLandData();

    //! 清除畫面
    void cleanWidget();

    //! 關閉視窗
    void closeWidget() {
        // cleanWidget();
        this->close();
    };

   private:
    Ui::BcmsLoadForm *ui;

    std::vector<ILandCode> mZonList;
    std::vector<ILandCode> mSectionList;
    ILandCode selectedLand;

    // 地籍預覽資料
    QStandardItemModel *model = new QStandardItemModel(this);

    void fetchLandCodeFromAPI();

    //! 檢查地段應該要塞入什麼
    void refreshSectionSelected();

    //! 傳入地籍地段的數字傳出中文
    QString findZonDesc(const QString &);
    QString findSectionDesc(const QString &);
};

#endif  // BCMSLOADFORM_H
