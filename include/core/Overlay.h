#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

struct OverlayTarget {
    QPoint position;
    QRect boundingBox;
    QColor color;
    bool isSelected;
};

class Overlay : public QWidget {
    Q_OBJECT

public:
    explicit Overlay(QWidget* parent = nullptr);
    ~Overlay();

    // FOV Circle
    void setFOVRadius(int radius);
    int getFOVRadius() const;

    void setFOVColor(const QColor& color);
    QColor getFOVColor() const;

    void setFOVVisible(bool visible);
    bool isFOVVisible() const;

    // Target indicators
    void setTargets(const std::vector<OverlayTarget>& targets);
    void clearTargets();

    void setTargetIndicatorVisible(bool visible);
    bool isTargetIndicatorVisible() const;

    // Crosshair
    void setCrosshairVisible(bool visible);
    bool isCrosshairVisible() const;

    void setCrosshairColor(const QColor& color);
    QColor getCrosshairColor() const;

    void setCrosshairSize(int size);
    int getCrosshairSize() const;

    // General
    void setOverlayEnabled(bool enabled);
    bool isOverlayEnabled() const;

    void updatePosition();

public slots:
    void refresh();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    int m_fovRadius;
    QColor m_fovColor;
    bool m_fovVisible;

    std::vector<OverlayTarget> m_targets;
    bool m_targetIndicatorVisible;

    bool m_crosshairVisible;
    QColor m_crosshairColor;
    int m_crosshairSize;

    bool m_overlayEnabled;
    QTimer* m_refreshTimer;

    void setupOverlayWindow();
    void drawFOVCircle(QPainter& painter);
    void drawTargets(QPainter& painter);
    void drawCrosshair(QPainter& painter);

#ifdef _WIN32
    void makeClickThrough();
#endif
};

#endif // OVERLAY_H
