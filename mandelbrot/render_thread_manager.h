#ifndef RENDER_THREAD_MANAGER_H
#define RENDER_THREAD_MANAGER_H

#include <QThread>
#include <QImage>
#include <mutex>
#include <condition_variable>
#include <AllSettings.h>

class render_thread_manager : public QThread
{
    Q_OBJECT

public:
    render_thread_manager();
    ~render_thread_manager();
    void render(QPointF const& c, double const& sc, QSize const& sz);
    void set_settings(AllSettings const& settings);
    void run() override;

signals:
    void image_rendered(QImage const& img, unsigned int img_sc);

private:
    void do_work(unsigned char* data, int y1, int y2, QSize s, qsizetype bpl, QPointF center, double scale);
    QColor value(QPointF const& p);
    void set_color(unsigned char*& p, QColor c);
    void cancel();

    std::mutex mx;
    bool need_cancel;
    std::condition_variable condition;
    int count_threads = 0;
    QPointF center;
    double scale;
    QSize size;
    AllSettings settings;
};

#endif // RENDER_THREAD_MANAGER_H
