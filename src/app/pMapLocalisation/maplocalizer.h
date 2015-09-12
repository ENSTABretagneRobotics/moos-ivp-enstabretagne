#ifndef MAPLOCALIZER_H
#define MAPLOCALIZER_H
#include <string>
#include <ibex.h>
#include <vector>
#include <array>
#include <deque>

using std::string;
using std::vector;
using std::array;
using std::deque;

using ibex::Interval;
using ibex::IntervalVector;
using ibex::CtcSegment;
using ibex::CtcInverse;
using ibex::Function;

#ifndef WALL_MAP
#define WALL_MAP
struct Wall {
    double w[4];
    friend std::istream& operator>>(std::istream& str, Wall& data)
    {
        std::string line;
        Wall tmp;
        if (std::getline(str,line)){
            std::stringstream iss(line);
            try{
                iss >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3];
                std::swap(data,tmp);
            } catch (std::exception& e){
                str.setstate(std::ios::failbit);
            }
        }
        return str;
    }
    double & operator[](std::size_t idx) { return w[idx]; }
};

typedef std::vector<Wall> Walls;
#endif

typedef struct Data_t{
    Interval x;
    Interval y;
    Interval rho;
    Interval theta;
    double time;

    Data_t(Interval& x, Interval& y, Interval& rho, Interval& theta, double time):
        x(x), y(y), rho(rho), theta(theta), time(time) {}
} Data_t;


class MapLocalizer
{
public:
    MapLocalizer(const string &map_filename);
    ~MapLocalizer();

    void setInitialPosition(Interval& x, Interval&y, double& time);
    void setInitialPosition(double x, double y, double time);
    void setSpeedNoise(double spd_err) {this->spd_err=spd_err;}
    void setHeadingNoise(double hdg_err) {this->hdg_err = hdg_err;}
    void setBufferSize(int N) {this->bufferSize = N;}
    void setNOutliers(int q) {this->NOutliers = q;}
    void update(Interval& rho, Interval& theta, double &time, int q);
    void predict(double &v, double &theta, double &t);


    Walls walls;
    Interval x_inertial, y_inertial;
    IntervalVector X_cur;
    deque<IntervalVector> pos;

private:

    CtcSegment ctcSegment;
    Function *f;
    void contract(IntervalVector &X, Interval &rho, Interval &theta, int q=0);
    void contractSegment(Interval &x, Interval &y, Wall &wall);
    void contractOneMeasurment(Interval &x, Interval &y, Interval &rho, Interval &theta, Wall &wall);
    void loadMap(const string &map_filename);

    double spd_err;
    double hdg_err;
    int bufferSize;
    int NOutliers;
    double t_old;
    deque<Data_t> data;
};

#endif // MAPLOCALIZER_H
