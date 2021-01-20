#ifndef MANDELBROT_WIDGET_H
#define MANDELBROT_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <complex>
#include <mutex>
#include <condition_variable>
#include "render_thread_manager.h"

class mandelbrot_widget : public QWidget
{
    Q_OBJECT

public:
    explicit mandelbrot_widget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

//signals:
//    void output_changed();

private:
    QImage img_buf;
    mutable std::mutex m;
    std::atomic<uint32_t> input_version;
    std::condition_variable input_changed;
    double scale = 0.005;
    QPointF center;
    bool notify_output_quied;
    QPoint mousePos;

    render_thread_manager manager;

private slots:
    void redraw(QImage const&);
};

#endif // MANDELBROT_WIDGET_H
