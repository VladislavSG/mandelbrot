#include "mandelbrot_widget.h"
#include <QKeyEvent>
#include <QThread>
#include <thread>
#include <vector>

static const double DEFAULT_SCALE = 0.005;

mandelbrot_widget::mandelbrot_widget(QWidget *parent) : QWidget(parent), scale(DEFAULT_SCALE)
{
    connect(&manager, &render_thread_manager::image_rendered, this, &mandelbrot_widget::redraw);
}

void mandelbrot_widget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (img_buf.isNull())
    {
        rerender();
    }
    else
    {
        QPainter p(this);
        p.drawImage(rect(), img_buf, QRect(QPoint(0, 0), size()/img_scale));
    }
}

void mandelbrot_widget::redraw(QImage& img, unsigned int img_sc)
{
    img_buf.swap(img);
    img_scale = img_sc;
    update();
}

void mandelbrot_widget::wheelEvent(QWheelEvent *event)
{
    const int numDegrees = event->angleDelta().y() / 8;
    const double numSteps = numDegrees / double(15);
    scale *= pow(0.75, numSteps);
    rerender();
}

void mandelbrot_widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        mousePos = event->pos();
}

void mandelbrot_widget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        center -= QPointF(event->pos() - mousePos)*scale;
        mousePos = event->pos();
        rerender();
    }
}

void mandelbrot_widget::resizeEvent(QResizeEvent *)
{
    rerender();
}

void mandelbrot_widget::reset()
{
    center = {0, 0};
    scale = DEFAULT_SCALE;
    rerender();
}

void mandelbrot_widget::rerender()
{
    manager.render(center, scale, size());
}

void mandelbrot_widget::settingsUpdate(AllSettings const& settings)
{
    manager.set_settings(settings);
    rerender();
}
