#include "simulator.h"
#include "vibes.h"



Simulator::Simulator(const string &map_filename, const string &traj_filename)
{
    Pose pose;
    ifstream readFile(traj_filename);
    while(readFile >> pose){
        navData.push_back(pose);
    }
    loadMap(map_filename);
    idx = 0;
    freq = 5;
    w = M_PI/4.;
    nbOutliers = 0.05;

}



void Simulator::setSonarFreq(double freq)
{
    this->freq  = freq;
}

void Simulator::setNbOutliers(double pNb)
{
    this->nbOutliers = pNb;
}

void Simulator::setSonarRotSpeed(double w){
    this->w = w;
}

double Simulator::getSonarAngle(double time)
{
    return 2*M_PI*w*time;
}

double Simulator::genSonarDist(double px, double py, double theta, double t)
{
    double alpha = theta + getSonarAngle(t);
    {
        vector<double> cx = {px, px+3*cos(alpha)};
        vector<double> cy = {py, py+3*sin(alpha)};
        vibes::drawLine(cx, cy, "r");
    }

    if( t * freq - round(t*freq) == 0){
        double d=oo;
        for (uint j=0;j<walls.size();j++){
            Wall &w  = walls[j];
            double dj,phij;
            DistanceDirSegment(dj,phij,px,py,alpha,w[0],w[1],w[2],w[3]);
            d = (dj < d) ? dj : d;
        }
        if ( (rand() /( (double) RAND_MAX)) > (1 - this->nbOutliers) )
            d-= 15;
        if (d < 100){
            vector<double> cx = {px, px+d*cos(alpha)};
            vector<double> cy = {py, py+d*sin(alpha)};
            vibes::drawLine(cx, cy, "g");
        }
        return ( d < 100 ) ? d  : -1;
    }

    return -1;
}

void Simulator::drawMap()
{
    for (int i = 0; i < walls.size(); i++){
        Wall &w = walls[i];
        vector<double> x = {w[0], w[2]};
        vector<double> y = {w[1], w[3]};
        vibes::drawLine(x, y);
    }
}

bool Simulator::nextStep()
{
    if(idx >= navData.size()){
        idx = 0;
        return -1;
    }
    return ++idx;
}

Pose &Simulator::currentPose()
{
    return navData[idx];
}

void Simulator::loadMap(const string &map_filename)
{
    std::ifstream in_file;
    in_file.open(map_filename, ios::in);
    if(in_file.fail()) {
        std::stringstream s;
        s << "Simulator [load]: cannot open file " << map_filename << "for reading the map";
        std::cerr << s << std::endl;
        exit(-1);
    }
    Wall wall;
    while(in_file >> wall){
        walls.push_back(wall);
    }
    in_file.close();

}
