#include "render_thread_manager.h"
#include <complex>
#include <iostream>

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
        start();
    } else {
        need_cancel = true;
        condition.notify_one();
    }
}

void render_thread_manager::run()
{
    forever {
        int pc = QThread::idealThreadCount();

        std::unique_lock ul(mx);
        int w = size.width();
        int h = size.height();
        QPointF c = center;
        double sc = scale;
        count_running_threads += pc;
        ul.unlock();

        QImage img(w, h, QImage::Format_RGB888);
        unsigned char* data = img.bits();
        qsizetype bpl = img.bytesPerLine();
        int lpp = (h+pc-1)/pc;

        std::vector<std::thread> all_threads;
        unsigned char* const data_start = data;
        unsigned char* const data_end = data + bpl * h;
        std::cout << data_start << " " << data_end << std::endl;
        unsigned int i = 0;
        unsigned int line = 0;

        for (; i < h - pc*(lpp-1); ++i, data += lpp*bpl, line += lpp) {
            all_threads.emplace_back([=, &w, &bpl, &c, &sc](){ do_work(data, line, line + lpp, w, bpl, c, sc); });
        }
        for (; i < pc; ++i, data += (lpp-1)*bpl, line += lpp-1) {
            all_threads.emplace_back([=, &w, &bpl, &c, &sc](){ do_work(data, line, line + lpp-1, w, bpl, c, sc); });
        }

        for (auto& t : all_threads) {
            t.join();
        }

        ul.lock();
        if (!need_cancel)
            emit render_thread_manager::image_rendered(img);

        if (!need_cancel)
            condition.wait(ul);
        need_cancel = false;
        ul.unlock();
    }
}

void render_thread_manager::do_work(unsigned char* data, int y1, int y2, int w, qsizetype bpl, QPointF center, double scale)
{
    for (int y = y1; y < y2; ++y)
    {
        {
            std::lock_guard lg(mx);
            if (need_cancel)
                break;
        }
        unsigned char *p = data + (y-y1) * bpl;
        for (int x = 0; x != w; ++x)
        {
            set_color(p, value(QPoint(x, y), center, scale));
        }
    }
    std::lock_guard lg(mx);
    --count_running_threads;
}

QColor render_thread_manager::value(QPoint const& p, QPointF const& center, double scale) const
{
    std::complex<double> c(p.x() - center.x(), p.y() - center.y());
    c *= scale;
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
