#include "mandelbrot_widget.h"
#include <QKeyEvent>
#include <QThread>
#include <thread>
#include <vector>

mandelbrot_widget::mandelbrot_widget(QWidget *parent) : QWidget(parent)
{
    connect(&manager, &render_thread_manager::image_rendered, this, &mandelbrot_widget::redraw);
}

void mandelbrot_widget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    manager.render(center, scale, size());
}

void mandelbrot_widget::redraw(QImage const& img)
{
    QPainter p(this);
    p.drawImage(0, 0, img);
}

void mandelbrot_widget::wheelEvent(QWheelEvent *event)
{
    const int numDegrees = event->angleDelta().y() / 8;
    const double numSteps = numDegrees / double(15);
    scale *= pow(0.8, numSteps);
    update();
}

void mandelbrot_widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        mousePos = event->position().toPoint();
}

void mandelbrot_widget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        center += event->position().toPoint() - mousePos;
        mousePos = event->position().toPoint();
        update();
    }
}
