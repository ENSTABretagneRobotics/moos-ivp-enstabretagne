#include "maplocalizer.h"
#include "ibex_QInter.h"

#include <fstream>
using std::ifstream;
#include <strstream>
#include "vibes.h"

MapLocalizer::MapLocalizer() : X_cur(2), spd_err(0.01), hdg_err(0.01) {}

MapLocalizer::MapLocalizer(const string &map_filename) :
X_cur(2), spd_err(0.01), hdg_err(0.01) {
    loadMap(map_filename);
}

void MapLocalizer::setMap(const string &map_filename)
{
    loadMap(map_filename);
}

void MapLocalizer::setInitialPosition(ibex::Interval &x, ibex::Interval &y, double &time) {
    x_inertial = x;
    y_inertial = y;
    X_cur[0] = x;
    X_cur[1] = y;
    t_old = time;
}

void MapLocalizer::setInitialPosition(double x, double y, double time) {
    x_inertial = Interval(x).inflate(0.01);
    y_inertial = Interval(y).inflate(0.01);
    X_cur[0] = x_inertial;
    X_cur[1] = y_inertial;
    t_old = time;
}

void MapLocalizer::update(ibex::Interval &rho, ibex::Interval &theta, double &time) {
    // Add the new measurment to the list and remove the last entry
    data.push_front(Data_t(x_inertial, y_inertial, rho, theta, time));
    pos.push_front(X_cur);



    vector<IntervalVector> boxes(data.size(), pos[0]);
    ibex::Array<IntervalVector> array_boxes(pos.size());
    // case i == 0, only contraction
    Data_t &t0 = data[0];
    IntervalVector dX(2);
    contract(boxes[0], data[0].rho, data[0].theta);
    if (pos.size() <= NOutliers) { // not enough measurments in the buffer
        pos[0] &= boxes[0];
        return;
    }
    array_boxes.set_ref(0, boxes[0]);


    for (int i = 1; i < data.size(); i++) {
        Data_t &ti = data[i];
        double dx1 = t0.x.lb() - ti.x.lb();
        double dx2 = t0.x.ub() - ti.x.ub();
        dX[0] = Interval(std::min(dx1, dx2), std::max(dx1, dx2));
        double dy1 = t0.y.lb() - ti.y.lb();
        double dy2 = t0.y.ub() - ti.y.ub();
        dX[1] = Interval(std::min(dy1, dy2), std::max(dy1, dy2));
        IntervalVector old_pos = pos[i];

        old_pos &= pos[0] - dX;
        contract(old_pos, ti.rho, ti.theta);

        boxes[i] &= old_pos + dX;
        ;
        array_boxes.set_ref(i, boxes[i]);
    }


    pos[0] &= ibex::qinter_projf(array_boxes, pos.size() - this->NOutliers);


    int NMax = 30;
    if (data.size() >= NMax) {
        data.pop_back();
        pos.pop_back();
    }
    X_cur &= pos[0];


}


// map_filename contains list of segments
// ax1 ax1 bx1 by1
// ax2 ax2 bx2 by2
// ....
// Load the map and fill <Walls> vector

void MapLocalizer::loadMap(const string& map_filename) {
    // Clear the previous map
    this->walls.clear();

    std::ifstream in_file;
    in_file.open(map_filename, ios::in);
    if (in_file.fail()) {
        std::stringstream s;
        s << "MapLocalizer [load]: cannot open file " << map_filename << "for reading the map";
        std::cerr << s << std::endl;
        exit(-1);
    }
    try {
        std::string line;
        // Read the header and fill it in with wonderful values
        while (!in_file.eof()) {

            getline(in_file, line);
            // Ignore empty lines
            if (line == "")
                continue;
            std::stringstream sstream(line);
            Wall w;
            sstream >> w[0] >> w[1] >> w[2] >> w[3];
            walls.push_back(w);
            std::cout << w[0] << " " << w[1] << " "
                    << w[2] << " " << w[3]
                    << " " << std::endl;
        }

    } catch (std::exception& e) {
        std::stringstream s;
        s << "MapLocalizer [load]: reading error " << e.what() << std::endl;
        std::cerr << s << std::endl;
    }
    in_file.close();

}

void MapLocalizer::contractSegment(Interval& x, Interval& y, Wall& wall) {
    IntervalVector tmp(6);
    tmp[0] = x;
    tmp[1] = y;
    tmp[2] = Interval(wall[0]);
    tmp[3] = Interval(wall[1]);
    tmp[4] = Interval(wall[2]);
    tmp[5] = Interval(wall[3]);
    this->ctcSegment.contract(tmp);
    x &= tmp[0];
    y &= tmp[1];
    if (x.is_empty() || y.is_empty()) {
        x.set_empty();
        y.set_empty();
    }
}

void MapLocalizer::contractOneMeasurment(Interval&x, Interval&y, Interval& rho, Interval& theta, Wall& wall) {

    Interval ax = rho * cos(theta);
    Interval ay = rho * sin(theta);

    Interval qx = x + ax;
    Interval qy = y + ay;

    contractSegment(qx, qy, wall);

    bwd_add(qx, x, ax);
    bwd_add(qy, y, ay);

    if (x.is_empty() || y.is_empty()) {
        x.set_empty();
        y.set_empty();
    }
}

void MapLocalizer::contract(IntervalVector& X, Interval& rho, Interval& theta, int q) {
    // List of boxes initialized with X
    std::vector<IntervalVector> boxes(walls.size(), X);
    IntervalVector res(IntervalVector::empty(2));
    for (int i = 0; i < walls.size(); i++) {
        contractOneMeasurment(boxes[i][0], boxes[i][1], rho, theta, walls[i]);
        res |= boxes[i];
    }
    X &= res;
}

void MapLocalizer::predict(double& v, double& theta, double& t) {

    Interval iV(v);
    Interval iTheta(theta);
    iV.inflate(spd_err);
    iTheta.inflate(hdg_err);

    double dt = t - t_old;

    Interval df_x = iV * cos(iTheta) * dt;
    Interval df_y = iV * sin(iTheta) * dt;
    // Update inertial tube
    x_inertial += df_x;
    y_inertial += df_y;
    // Update current position box
    X_cur[0] += df_x;
    X_cur[1] += df_y;

    t_old = t;
}

void MapLocalizer::updateGPS(const double &easting, const double &northing) {
    IntervalVector fix(2);
    fix[0] = easting;
    fix[1] = northing;
    fix.inflate(gps_noise);

    X_cur &= fix;
}