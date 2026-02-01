#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QDialog>
#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QTimer>
#include <QWidget>

class ColorPicker : public QDialog {
    Q_OBJECT

public:
    explicit ColorPicker(QWidget* parent = nullptr);
    ~ColorPicker();

    QColor getSelectedColor() const;
    void setColor(const QColor& color);

signals:
    void colorSelected(const QColor& color);
    void colorChanged(const QColor& color);

private slots:
    void onPickFromScreenClicked();
    void onRGBChanged();
    void onHexChanged();
    void onPickerTimer();
    void onOkClicked();
    void onCancelClicked();

private:
    QColor m_selectedColor;
    QColor m_originalColor;
    bool m_isPicking;

    // UI Elements
    QLabel* m_previewLabel;
    QLabel* m_originalLabel;

    QSlider* m_redSlider;
    QSlider* m_greenSlider;
    QSlider* m_blueSlider;

    QLineEdit* m_redEdit;
    QLineEdit* m_greenEdit;
    QLineEdit* m_blueEdit;
    QLineEdit* m_hexEdit;

    QPushButton* m_pickButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

    QTimer* m_pickerTimer;

    void setupUI();
    void updatePreview();
    void updateSliders();
    void updateEdits();
    QColor getColorAtCursor();
};

#endif // COLORPICKER_H
