#include <qgsmaptooldigitizefeature.h>

//! This tool adds new point/line/polygon features to already existing vector
//! layers
class QgsMapToolAddFeature : public QgsMapToolDigitizeFeature {
    Q_OBJECT

   public:
    QgsMapToolAddFeature(QgsMapCanvas *canvas,
                         QgsAdvancedDigitizingDockWidget *cadDockWidget,
                         CaptureMode mode);

   private slots:

    void featureDigitized(const QgsFeature &feature) override;

   private:
    bool addFeature(QgsVectorLayer *vlayer, const QgsFeature &f,
                    bool showModal = true);

    /**
     * Check if CaptureMode matches layer type. Default is TRUE.
     * \since QGIS 2.12
     */
    bool mCheckGeometryType;
};
