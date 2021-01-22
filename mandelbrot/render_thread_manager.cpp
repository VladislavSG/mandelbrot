#include "render_thread_manager.h"
#include <complex>
#include <iostream>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

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
    std::unique_lock ul(mx);
    if (count_threads != 0) {
        need_cancel = true;
        no_threads.wait(ul, [&](){return count_threads == 0;});
    }
    center = c;
    scale = sc;
    if (img.size() != sz)
        img = QImage(sz, QImage::Format_RGB888);

    if (!isRunning()) {
        start();
    } else {
        need_cancel = true;
        has_job.notify_one();
    }
}

void render_thread_manager::set_settings(const AllSettings &set)
{
    std::unique_lock ul(mx);
    need_cancel = true;
    if (count_threads != 0)
        no_threads.wait(ul, [&](){return count_threads == 0;});
    settings = set;
}

void render_thread_manager::run()
{
    forever {
        int pc = QThread::idealThreadCount();

        std::unique_lock ul(mx);
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
            QSize new_sz{rup_div(img.width(), img_scale), rup_div(img.height(), img_scale)};
            double new_sc = sc * img_scale;

            unsigned char* data = img.bits();
            qsizetype bpl = img.bytesPerLine(); // bytes per line
            int lpt = rup_div(new_sz.height(), pc); // lines per thread

            int i = 0;
            unsigned int line = 0;

            QVector<task> tasks;

            for (; i < new_sz.height() - pc*(lpt-1); ++i, data += lpt*bpl, line += lpt) {
                tasks << task{data, line, line + lpt, new_sz, bpl, c, new_sc};
            }
            for (; i < pc; ++i, data += (lpt-1)*bpl, line += lpt-1) {
                tasks << task{data, line, line + lpt-1, new_sz, bpl, c, new_sc};
            }

            QFuture<void> future = QtConcurrent::map(tasks, std::bind(&render_thread_manager::do_work, this, std::placeholders::_1));
            future.waitForFinished();

            {
                std::lock_guard lg(mx);
                if (!need_cancel)
                    emit render_thread_manager::image_rendered(img, img_scale);
            }
        }
        ul.lock();
        if (!need_cancel)
            has_job.wait(ul);
        need_cancel = false;
        ul.unlock();
    }
}

void render_thread_manager::do_work(task const& t)
{
    for (unsigned int y = t.y1; y < t.y2; ++y)
    {
        {
            std::lock_guard lg(mx);
            if (need_cancel)
                break;
        }
        unsigned char *p = t.data + (y-t.y1) * t.bpl;
        for (int x = 0; x != t.size.width(); ++x)
        {
            set_color(p, value(QPointF(x - t.size.width()/2., y - t.size.height()/2.)*t.scale + t.center));
        }
    }
    std::lock_guard lg(mx);
    --count_threads;
    if (count_threads == 0)
        no_threads.notify_one();
}

QColor render_thread_manager::value(QPointF const& p)
{
    std::complex<double> c(p.x(), p.y());
    std::complex<double> z = 0;
    for (size_t step = 0;; ++step)
    {
        if (std::abs(z) >= 2.)
        {
            int color = (step % 43)*255/42;
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
