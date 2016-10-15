#ifndef ROADMASTER_HH_
#define ROADMASTER_HH_

#include <qtimer.h>

#include "roadmaster_base.h"
#include "../../include/roadmaster/data.h"

class RoadMaster : public RoadMasterBase
{
    Q_OBJECT
    
    public:
        RoadMaster(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
        ~RoadMaster();

    public slots:
        void refresh();
        void set_calib();
        void reset_calib();
        void backward();
        void incr_calib();
        void decr_calib();
        void set_global_distance();
        void compute_local_distance_target();
        void compute_local_speed_target();
        void compute_local_time_target();
        void incr_distance();
        void decr_distance();
        void set_global_distance_target(int dist);
        void set_local_distance_target(int dist);
        void set_local_speed_target(int speed);
        void set_local_time_hour_target(int hour);
        void set_local_time_min_target(int min);
        void set_local_time_sec_target(int sec);

    private:
        void set_bg_color(QColor color);
        void update_ghost_color();
        void refresh_timeval();

        struct rm_data *d;
        QTimer timer;
};

#endif /* ROADMASTER_HH_ */
