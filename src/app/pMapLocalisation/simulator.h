#include <ibex.h>

#define oo 1/0.

struct Pose {
    double t;
    double x;
    double y;
    double theta;
    double speed;

    friend std::istream& operator>>(std::istream& str, Pose& data)
    {
        std::string line;
        Pose tmp;
        if (std::getline(str,line)){
            std::stringstream iss(line);
            try{
                iss >> tmp.t >> tmp.x >> tmp.y;
                iss >> tmp.speed >> tmp.theta;
                std::swap(data,tmp);
            } catch (std::exception& e){
                str.setstate(std::ios::failbit);
            }
        }
        return str;
    }
};

typedef std::vector<Pose> NavData;

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
class Simulator {

public:
    Simulator(const string& map_filename, const string& traj_filename);
    void setSonarFreq(double freq);
    void setNbOutliers(double pNb);
    double getSonarAngle(double time);
    double genSonarDist(double px, double py, double theta, double t);
    void drawMap();
    bool nextStep();

    Pose & currentPose();


    NavData navData;

    void setSonarRotSpeed(double w);
private:
    void loadMap(const string& map_filename);
    Walls walls;
    int idx;
    double nbOutliers;
    double freq, w;
};



//*********************************************************************************
// INLINE IMPL.
//*********************************************************************************


inline double Det(double &ax, double& ay, double& bx, double &by){
    return ax*by - ay*bx;
}

inline void DistanceDirSegment(double& d,double& phi,
                                               double mx, double my, double theta,
                                               double ax, double ay, double bx, double by)
{      // Distance directionnelle du point m au segment [a,b].
    double ma_x=ax-mx;
    double ma_y=ay-my;
    double mb_x=bx-mx;
    double mb_y=by-my;
    double ab_x=bx-ax;
    double ab_y=by-ay;
    double ux=cos(theta);
    double uy=sin(theta);
    double z1=Det(ma_x,ma_y,ux,uy);
    double z2=Det(ux,uy,mb_x,mb_y);
    double z3=Det(ma_x,ma_y,ab_x,ab_y);
    double z4=Det(ux,uy,ab_x,ab_y);
    double z5=min(z1,min(z2,z3));
    double d1=z3/z4;
    d= (z5 < 0) ? oo :d1;
    phi=atan2(-ab_x,ab_y); //phi is the angle of the normal vector of [a,b]
}

inline void DistanceDirSegments(double& d,double& phi,
                                double mx, double my, double theta,
                                vector<double> ax, vector<double> ay, vector<double> bx, vector<double> by)
{      // Distance directionnelle relativement � un polygone (le tri�dre m-a-b doit �tre direct, sinon cette distance est infinie)
        d=oo;
        for (uint j=0;j<ax.size();j++)
           {double dj,phij;
            DistanceDirSegment(dj,phij,mx,my,theta,ax[j],ay[j],bx[j],by[j]);
            if (dj<d) {d=dj;phi=phij;};
            }
}

//#endif // MAINWINDOW_H
