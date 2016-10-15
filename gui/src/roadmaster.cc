#include <err.h>
#include <fcntl.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qcolor.h>
#include <qpalette.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "roadmaster.hh"


RoadMaster::RoadMaster(QWidget* parent,
                       const char* name,
                       WFlags fl):RoadMasterBase(parent, name, fl)
{
    int fd_shm = shm_open("/roadmaster", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd_shm < 0)
        err(2, "shm_open");

    if (ftruncate(fd_shm, sizeof (struct rm_data)) < 0)
        err(2, "ftruncate");

    this->d = (struct rm_data *) mmap(NULL, sizeof (struct rm_data),
            PROT_READ | PROT_WRITE,
            MAP_SHARED, fd_shm, 0);
    if (this->d == MAP_FAILED)
        err(2, "mmap");

    connect(Backward, SIGNAL(clicked()), this, SLOT(backward()));
    connect(Backward1, SIGNAL(clicked()), this, SLOT(backward()));

    connect(SetCalib, SIGNAL(clicked()), this, SLOT(set_calib()));
    connect(ResetCalib, SIGNAL(clicked()), this, SLOT(reset_calib()));
    connect(IncrRPK, SIGNAL(clicked()), this, SLOT(incr_calib()));
    connect(DecrRPK, SIGNAL(clicked()), this, SLOT(decr_calib()));

    connect(UpdateGlobalDistance, SIGNAL(clicked()), this,
            SLOT(set_global_distance()));

    connect(ComputeLocalDistanceTarget, SIGNAL(clicked()), this,
            SLOT(compute_local_distance_target()));

    connect(ComputeLocalSpeedTarget, SIGNAL(clicked()), this,
            SLOT(compute_local_speed_target()));

    connect(ComputeLocalTimeTarget, SIGNAL(clicked()), this,
            SLOT(compute_local_time_target()));

    connect(IncrLocalDistance, SIGNAL(clicked()), this,
            SLOT(incr_distance()));
    connect(DecrLocalDistance, SIGNAL(clicked()), this,
            SLOT(decr_distance()));

    connect(IncrGlobalDistance, SIGNAL(clicked()), this,
            SLOT(incr_distance()));
    connect(DecrGlobalDistance, SIGNAL(clicked()), this,
            SLOT(decr_distance()));

    connect(SetGlobalDistanceTarget, SIGNAL(valueChanged(int)), this,
            SLOT(set_global_distance_target(int)));
    connect(SetLocalDistanceTarget, SIGNAL(valueChanged(int)), this,
            SLOT(set_local_distance_target(int)));
    connect(SetLocalSpeedTarget, SIGNAL(valueChanged(int)), this,
            SLOT(set_local_speed_target(int)));
    connect(SetLocalTimeHour, SIGNAL(valueChanged(int)), this,
            SLOT(set_local_time_hour_target(int)));
    connect(SetLocalTimeMin, SIGNAL(valueChanged(int)), this,
            SLOT(set_local_time_min_target(int)));
    connect(SetLocalTimeSec, SIGNAL(valueChanged(int)), this,
            SLOT(set_local_time_sec_target(int)));

    connect(&this->timer, SIGNAL(timeout()), this, SLOT(refresh()));

    this->refresh();
}

RoadMaster::~RoadMaster()
{
}

void RoadMaster::backward()
{
    this->d->ctrl.forward = !this->d->ctrl.forward;
}

void RoadMaster::set_calib()
{
    if (!this->d->ctrl.calib.running &&
            QMessageBox::warning(this, "Etalonnage",
                "L'ancienne valeur d'etalonnage \nsera perdue.\n"
                "Etes-vous sur de vouloir \ncommencer l'etalonnage ?",
                QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        this->d->ctrl.calib.start = true;
    else
        this->d->ctrl.calib.finish = true;
}

void RoadMaster::reset_calib()
{
    if (QMessageBox::warning(this, "Etalonnage",
                "Etes-vous sur de vouloir \nreset l'etalonnage ?",
                QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        this->d->settings.rot_per_km = ROADMASTER_DEFAULT_RPK;
}

void RoadMaster::incr_calib()
{
    this->d->settings.rot_per_km += 1;
}

void RoadMaster::decr_calib()
{
    this->d->settings.rot_per_km -= 1;
}

void RoadMaster::set_global_distance()
{
    this->d->distance.global = SetGlobalDistance->value() / 10;
}

void RoadMaster::incr_distance()
{
    this->d->distance.global += 1;
    this->d->distance.local += 1;
}

void RoadMaster::decr_distance()
{
    this->d->distance.global -= 1;
    this->d->distance.local -= 1;
}

void RoadMaster::compute_local_distance_target()
{
    this->refresh_timeval();
    this->d->distance.local_target = (this->d->speed.local_target
        * this->d->timers.local_target.tv_sec) / 360;
    SetLocalDistanceTarget->setValue(this->d->distance.local_target * 10);
}

void RoadMaster::compute_local_speed_target()
{
    if (this->d->timers.local_target.tv_sec)
        this->d->speed.local_target = (this->d->distance.local_target * 360)
            / this->d->timers.local_target.tv_sec;
    else
        this->d->speed.local_target = 0;
    SetLocalSpeedTarget->setValue(this->d->speed.local_target * 100);
}

void RoadMaster::compute_local_time_target()
{
    if (this->d->speed.local_target)
        this->d->timers.local_target.tv_sec = (this->d->distance.local_target * 360)
            / this->d->speed.local_target;
    else
        this->d->timers.local_target.tv_sec = 0;
    gmtime_r(&this->d->timers.local_target.tv_sec,
            &this->d->timers.local_target_tm);
    SetLocalTimeHour->setValue(this->d->timers.local_target_tm.tm_hour);
    SetLocalTimeMin->setValue(this->d->timers.local_target_tm.tm_min);
    SetLocalTimeSec->setValue(this->d->timers.local_target_tm.tm_sec);
}

void RoadMaster::set_global_distance_target(int dist)
{
    this->d->distance.global_target = dist / 10;
}

void RoadMaster::set_local_distance_target(int dist)
{
    this->d->distance.local_target = dist / 10;
}

void RoadMaster::set_local_speed_target(int speed)
{
    this->d->speed.local_target = speed / 100;
}

void RoadMaster::set_local_time_hour_target(int hour)
{
    this->d->timers.local_target_tm.tm_hour = hour;
    this->refresh_timeval();
}

void RoadMaster::set_local_time_min_target(int min)
{
    this->d->timers.local_target_tm.tm_min = min;
    this->refresh_timeval();
}

void RoadMaster::set_local_time_sec_target(int sec)
{
    this->d->timers.local_target_tm.tm_sec = sec;
    this->refresh_timeval();
}

void RoadMaster::refresh_timeval()
{
    this->d->timers.local_target_tm.tm_mday = 1;
    this->d->timers.local_target_tm.tm_mon = 0;
    this->d->timers.local_target_tm.tm_year = 70;
    this->d->timers.local_target_tm.tm_isdst = 0;
    this->d->timers.local_target.tv_sec = mktime(&this->d->timers
            .local_target_tm) + 3600;
}

void RoadMaster::refresh()
{
    int dist;

    if (!this->d->ctrl.forward)
        this->set_bg_color(QColor(250, 60, 60));
    else if (this->d->ctrl.start)
    {
        if (this->d->ctrl.hard_start)
            this->set_bg_color(QColor(0, 255, 0));
        else
            this->set_bg_color(QColor(30, 180, 30));
    }
    else
        this->set_bg_color(QColor(0, 0, 0));

    // Home
    SpeedInst1->display(QString("%1")
            .arg(this->d->speed.inst));

    Clock->display(QString("%1:%2:%3")
            .arg(this->d->timers.current_tm.tm_hour)
            .arg(this->d->timers.current_tm.tm_min)
            .arg(this->d->timers.current_tm.tm_sec));

    // ZR
    SpeedInst->display(QString("%1")
            .arg(this->d->speed.inst));

    LocalSpeedAverage->display(QString("%1.%2")
            .arg(this->d->speed.local_average / 10)
            .arg(this->d->speed.local_average % 10));


    if (this->d->distance.local_ghost)
        LocalGhostDiff->display(QString("%1%2")
                .arg(this->d->distance.local_ghost_diff >= 0 ? "+" : "")
                .arg((double) this->d->distance.local_ghost_diff / 100.,
                    0, 'f', 2));
    else
        LocalGhostDiff->display("--");
    this->update_ghost_color();

    LocalDistance->display(QString("%1")
            .arg((double) this->d->distance.global / 100., 0, 'f', 2));

    PartDistance->display(QString("%1")
            .arg((double) this->d->distance.part / 100., 0, 'f', 2));

    dist = this->d->distance.local_target - this->d->distance.local;
    if (dist < 0)
        LocalDistanceTarget->display("--");
    else
        LocalDistanceTarget->display(QString("%1.%2")
                .arg(dist / 100)
                .arg((dist / 10) % 10));

    Chrono->display(QString("%1:%2:%3")
            .arg(this->d->timers.local_timer_tm.tm_hour, 2)
            .arg(this->d->timers.local_timer_tm.tm_min, 2)
            .arg(this->d->timers.local_timer_tm.tm_sec, 2));

    if (this->d->timers.local_target.tv_sec == 0)
        Timer->display("--:--:--");
    else
        Timer->display(QString("%1:%2:%3")
                .arg(this->d->timers.local_timer_dec_tm.tm_hour, 2)
                .arg(this->d->timers.local_timer_dec_tm.tm_min, 2)
                .arg(this->d->timers.local_timer_dec_tm.tm_sec, 2));

    // Calib
    RotPerKm->display(QString("%1").arg(this->d->settings.rot_per_km));
    if (this->d->ctrl.calib.running)
        SetCalib->setText("Fin de zone");
    else
        SetCalib->setText("Depart de zone");


    // Rally
    SpeedInst2->display(QString("%1")
            .arg(this->d->speed.inst));

    GlobalDistance->display(QString("%1")
            .arg((double) this->d->distance.global / 100., 0, 'f', 2));

    SpeedGlobalAverage->display(QString("%1.%2")
            .arg(this->d->speed.global_average / 10)
            .arg(this->d->speed.global_average % 10));

    PartDistance1->display(QString("%1")
            .arg((double) this->d->distance.part / 100., 0, 'f', 2));

    dist = this->d->distance.global_target - this->d->distance.global;
    if (dist < 0)
        GlobalDistanceTarget->display("-");
    else
        GlobalDistanceTarget->display(QString("%1.%2")
                .arg(dist / 100)
                .arg((dist / 10) % 10));

    this->timer.start(200, TRUE);
}

void RoadMaster::set_bg_color(QColor color)
{
    LocalDistance->setPalette(QPalette(QColor(255, 255, 0), color));
    GlobalDistance->setPalette(QPalette(QColor(255, 255, 0), color));
}

#define GHOST_TOL 30

void RoadMaster::update_ghost_color()
{
    QColor c = QColor(0, 0, 0);

    if (this->d->distance.local_ghost)
    {
        if (this->d->distance.local_ghost_diff < -GHOST_TOL)
            c = QColor(255, 255, 0);
        else if (this->d->distance.local_ghost_diff > GHOST_TOL)
            c = QColor(255, 100, 100);
        else
            c = QColor(0, 255, 0);
    }
    LocalGhostDiff->setPalette(QPalette(QColor(0, 0, 0), c));
}
