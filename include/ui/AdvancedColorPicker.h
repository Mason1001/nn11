#ifndef ADVANCEDCOLORPICKER_H
#define ADVANCEDCOLORPICKER_H

#include <QDialog>
#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QTabWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTimer>
#include <QWidget>
#include <QString>
#include <vector>

struct ColorFormat {
    QString name;
    QString value;
};

class AdvancedColorPicker : public QDialog {
    Q_OBJECT

public:
    explicit AdvancedColorPicker(QWidget* parent = nullptr);
    ~AdvancedColorPicker();

    QColor getSelectedColor() const;
    void setColor(const QColor& color);

signals:
    void colorSelected(const QColor& color);

private slots:
    void onPickFromScreenClicked();
    void onRGBChanged();
    void onHSVChanged();
    void onHSLChanged();
    void onCMYKChanged();
    void onHexChanged();
    void onHistoryItemClicked(QListWidgetItem* item);
    void onPickerTimer();
    void onOkClicked();
    void onCancelClicked();

private:
    QColor m_selectedColor;
    bool m_isPicking;
    bool m_updatingUI;
    std::vector<QColor> m_colorHistory;

    // Preview
    QLabel* m_previewLabel;
    QLabel* m_magnifierLabel;

    // RGB Tab
    QSlider* m_redSlider;
    QSlider* m_greenSlider;
    QSlider* m_blueSlider;
    QLineEdit* m_redEdit;
    QLineEdit* m_greenEdit;
    QLineEdit* m_blueEdit;

    // HSV Tab
    QSlider* m_hueSlider;
    QSlider* m_saturationSlider;
    QSlider* m_valueSlider;
    QLineEdit* m_hueEdit;
    QLineEdit* m_saturationEdit;
    QLineEdit* m_valueEdit;

    // HSL Tab
    QSlider* m_hslHueSlider;
    QSlider* m_hslSatSlider;
    QSlider* m_lightnessSlider;
    QLineEdit* m_hslHueEdit;
    QLineEdit* m_hslSatEdit;
    QLineEdit* m_lightnessEdit;

    // CMYK Tab
    QSlider* m_cyanSlider;
    QSlider* m_magentaSlider;
    QSlider* m_yellowSlider;
    QSlider* m_blackSlider;
    QLineEdit* m_cyanEdit;
    QLineEdit* m_magentaEdit;
    QLineEdit* m_yellowEdit;
    QLineEdit* m_blackEdit;

    // HEX
    QLineEdit* m_hexEdit;

    // All formats display
    QLabel* m_allFormatsLabel;

    // History
    QListWidget* m_historyList;

    // Buttons
    QPushButton* m_pickButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

    QTimer* m_pickerTimer;
    QTabWidget* m_tabWidget;

    void setupUI();
    QWidget* createRGBTab();
    QWidget* createHSVTab();
    QWidget* createHSLTab();
    QWidget* createCMYKTab();
    void updateAllDisplays();
    void updatePreview();
    void updateRGBTab();
    void updateHSVTab();
    void updateHSLTab();
    void updateCMYKTab();
    void updateHexEdit();
    void updateAllFormatsLabel();
    void addToHistory(const QColor& color);
    void loadHistory();
    void saveHistory();
    QColor getColorAtCursor();
    QString colorToAllFormats(const QColor& color);
};

#endif // ADVANCEDCOLORPICKER_H
