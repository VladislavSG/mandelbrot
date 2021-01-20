#include "render_thread_manager.h"
#include <complex>
#include <iostream>

static const int BLOCK_SIZE = 32;

int rup_div(int a, int b)
{
    return (a+b-1)/b;
}

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

void render_thread_manager::set_settings(const AllSettings &set)
{
    std::unique_lock ul(mx);
    need_cancel = true;
    condition.wait(ul, [this](){return count_threads == 0;});
    settings = set;
}

void render_thread_manager::run()
{
    forever {
        int pc = QThread::idealThreadCount();

        std::unique_lock ul(mx);
        QSize sz = size;
        QPointF c = center;
        double sc = scale;
        ul.unlock();

        for (int img_scale = BLOCK_SIZE; img_scale > 0; img_scale /= 2)
        {
            {
                std::lock_guard lg(mx);
                if (need_cancel)
                    break;
                else
                    count_threads += pc;
            }
            QSize new_sz{rup_div(sz.width(), img_scale), rup_div(sz.height(), img_scale)};
            double new_sc = sc * img_scale;

            QImage img(new_sz, QImage::Format_RGB888);
            unsigned char* data = img.bits();
            qsizetype bpl = img.bytesPerLine(); // bytes per line
            int lpt = rup_div(new_sz.height(), pc); // lines per thread

            std::vector<std::thread> all_threads;
            int i = 0;
            unsigned int line = 0;

            for (; i < new_sz.height() - pc*(lpt-1); ++i, data += lpt*bpl, line += lpt) {
                all_threads.emplace_back([=, &new_sz, &bpl, &c, &new_sc](){ do_work(data, line, line + lpt, new_sz, bpl, c, new_sc); });
            }
            for (; i < pc; ++i, data += (lpt-1)*bpl, line += lpt-1) {
                all_threads.emplace_back([=, &new_sz, &bpl, &c, &new_sc](){ do_work(data, line, line + lpt-1, new_sz, bpl, c, new_sc); });
            }

            for (auto& t : all_threads) {
                t.join();
            }

            {
                std::lock_guard lg(mx);
                if (!need_cancel)
                    emit render_thread_manager::image_rendered(img, img_scale);
            }
        }
        ul.lock();
        if (!need_cancel)
            condition.wait(ul);
        need_cancel = false;
        ul.unlock();
    }
}

void render_thread_manager::do_work(unsigned char* data, int y1, int y2, QSize s, qsizetype bpl, QPointF center, double scale)
{
    for (int y = y1; y < y2; ++y)
    {
        {
            std::lock_guard lg(mx);
            if (need_cancel)
                break;
        }
        unsigned char *p = data + (y-y1) * bpl;
        for (int x = 0; x != s.width(); ++x)
        {
            set_color(p, value(QPointF(x - s.width()/2, y - s.height()/2)*scale + center));
        }
    }
    std::lock_guard lg(mx);
    --count_threads;
}

QColor render_thread_manager::value(QPointF const& p)
{
    std::complex<double> c(p.x(), p.y());
    std::complex<double> z = 0;
    for (size_t step = 0;; ++step)
    {
        if (std::abs(z) >= 2.)
        {
            int color = (step % 43)*256/42;
            switch (step/43 % 2)
            {
                case 0:
                    return QColor(color/2, color, color/4);
                case 1:
                    return QColor(color, color/2, color/4);
            }
        }
        if (step == settings.max_step)
            return QColor(0, 0, 0);
        z = pow(z, settings.pow_deg) + c;
    }
}

void render_thread_manager::set_color(unsigned char*& p, QColor c)
{
    *p++ = c.red();
    *p++ = c.green();
    *p++ = c.blue();
}
