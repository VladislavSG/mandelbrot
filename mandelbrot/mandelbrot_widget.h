#ifndef MANDELBROT_WIDGET_H
#define MANDELBROT_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <complex>
#include <mutex>
#include <condition_variable>
#include "render_thread_manager.h"
#include "AllSettings.h"

class mandelbrot_widget : public QWidget
{
    Q_OBJECT

public:
    explicit mandelbrot_widget(QWidget *parent = nullptr);

public slots:
    void settingsUpdate(AllSettings const&);
    void reset();

private:
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void rerender();

    QImage img_buf;
    unsigned int img_scale;
    double scale;
    QPointF center;
    QPoint mousePos;
    render_thread_manager manager;

private slots:
    void redraw(QImage const&, unsigned int img_sc);
};

#endif // MANDELBROT_WIDGET_H
