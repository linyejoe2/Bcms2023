#include <qgsmaptooldigitizefeature.h>

class QgsHighlight;

//! This tool adds new point/line/polygon features to already existing vector
//! layers
class QgsMapToolAddFeature : public QgsMapToolDigitizeFeature {
    Q_OBJECT

   public:
    //! \since QGIS 3.26
    QgsMapToolAddFeature(QgsMapCanvas *canvas,
                         QgsAdvancedDigitizingDockWidget *cadDockWidget,
                         CaptureMode mode);
    /**
     * \since QGIS 2.12
     * \deprecated Will be made in QGIS 4
     */
    QgsMapToolAddFeature(QgsMapCanvas *canvas, CaptureMode mode);

   private slots:

    void featureDigitized(const QgsFeature &feature) override;

   private:
    bool addFeature(QgsVectorLayer *vlayer, const QgsFeature &f,
                    bool showModal = true);

    /**
     * Creates a highlight corresponding to the captured geometry map tool and
     * transfers ownership to the caller.
     */
    std::unique_ptr<QgsHighlight> createHighlight(QgsVectorLayer *layer,
                                                  const QgsFeature &f);

    /**
     * Check if CaptureMode matches layer type. Default is TRUE.
     * \since QGIS 2.12
     */
    bool mCheckGeometryType;
};
