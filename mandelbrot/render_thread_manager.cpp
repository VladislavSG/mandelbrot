#include "render_thread_manager.h"
#include <complex>

render_thread_manager::render_thread_manager()
{}

render_thread_manager::~render_thread_manager()
{}

void render_thread_manager::render(QPointF const& c, double const& sc, QSize const& sz)
{
    std::lock_guard lg(mx);
    center = c;
    scale = sc;
    size = sz;

    if (!isRunning()) {
        start(LowPriority);
    } else {
        need_cancel = true;
        condition.notify_one();
    }
}

void render_thread_manager::run()
{
    forever {
        std::unique_lock ul(mx);
        int w = size.width();
        int h = size.height();
        QPointF c = center;
        double sc = scale;
        ul.unlock();

        int pc = QThread::idealThreadCount();
        QImage img(w, h, QImage::Format_RGB888);
        emit render_thread_manager::image_rendered(img);
        unsigned char* data = img.bits();
        qsizetype bpl = img.bytesPerLine();

        std::vector<std::thread> all_threads;
        count_running_threads += pc;

        for (int i = 0; i < pc; ++i) {
            all_threads.emplace_back([&](){ do_work(data, h/pc*i, std::min(h/pc*(i+1), h), w, bpl); });
        }

        for (auto& t : all_threads) {
            t.join();
        }

        if (!need_cancel)
            emit render_thread_manager::image_rendered(img);

        ul.lock();
        if (!need_cancel)
            condition.wait(ul);
        need_cancel = false;
        ul.unlock();
    }
}

void render_thread_manager::do_work(unsigned char* data, int y1, int y2, int w, qsizetype bpl)
{
    for (int y = y1; y != y2; ++y)
    {
        {
            std::lock_guard lg(mx);
            if (need_cancel)
                break;
        }
        unsigned char *p = data + y * bpl;
        for (int x = 0; x != w; ++x)
        {
            set_color(p, render_thread_manager::value(QPoint(x, y), center, scale));
        }
    }
    std::lock_guard lg(mx);
    --count_running_threads;
}

QColor render_thread_manager::value(QPoint const& p, QPointF const& center, double scale) const
{
    std::complex<double> c(p.x()*scale - center.x(), p.y()*scale - center.y());
    size_t const MAX_STEPS = 100;
    std::complex<double> z = 0;
    for (size_t step = 0;; ++step)
    {
        if (std::abs(z) >= 2.)
            return QColor((step % 43)/ 42. * 256, 0, 0);
        if (step == MAX_STEPS)
            return QColor(0, 0, 0);
        z = z * z + c;
    }
}

void render_thread_manager::set_color(unsigned char*& p, QColor c)
{
    *p++ = c.red();
    *p++ = c.green();
    *p++ = c.blue();
}
