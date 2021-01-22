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
    void image_rendered(QImage& img, unsigned int img_sc);

private:
    struct task
    {
        unsigned char* data;
        unsigned int y1;
        unsigned int y2;
        QSize const& size;
        qsizetype const& bpl;
        QPointF const& center;
        double const& scale;
    };

    void do_work(task const& t);
    QColor value(QPointF const& p);
    void set_color(unsigned char*& p, QColor c);
    void cancel();

    std::mutex mx;
    bool need_cancel;
    std::condition_variable has_job;
    std::condition_variable no_threads;
    int count_threads = 0;
    QPointF center;
    double scale;
    QImage img;
    AllSettings settings;
};

#endif // RENDER_THREAD_MANAGER_H
