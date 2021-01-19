#ifndef RENDER_THREAD_MANAGER_H
#define RENDER_THREAD_MANAGER_H

#include <QThread>
#include <QImage>
#include <mutex>
#include <condition_variable>

class render_thread_manager : public QThread
{
    Q_OBJECT

public:
    render_thread_manager();
    ~render_thread_manager();
    void render(QPointF const& c, double const& sc, QSize const& sz);
    void run() override;

signals:
    void image_rendered(QImage const& img);

private:
    void do_work(unsigned char* data, int y1, int y2, int w, qsizetype bpl);
    QColor value(QPoint const& p, QPointF const& center, double scale) const;
    void set_color(unsigned char*& p, QColor c);
    void cancel();

    std::mutex mx;
    bool need_cancel;
    std::condition_variable condition;
    unsigned int count_running_threads = 0;
    QPointF center;
    double scale;
    QSize size;
};

#endif // RENDER_THREAD_MANAGER_H
