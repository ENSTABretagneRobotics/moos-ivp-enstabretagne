#include "vibes.h"
#include "simulator.h"
#include "maplocalizer.h"


int main(int argc, char** argv){

    // Set Vibes Params
    vibes::beginDrawing();
    vibes::newFigure("test");
    vibes::setFigureProperties("test", vibesParams("x",0,"y",0,"width",1000,"height",1000 ));
    vibes::axisLimits(-10, 140, -10, 140);

    // Intantiare the simulator xith the map
    Simulator simu("map.txt", "traj.txt");
    simu.setSonarFreq(5); // 5hz
    simu.setNbOutliers(0.05); // 5 % outliers

    // Intantiate Localizer object
    MapLocalizer localizer("map.txt");
    Pose &p0 = simu.currentPose();
    localizer.setInitialPosition(p0.x, p0.y, p0.t);
    localizer.setSpeedNoise(0.02);
    localizer.setHeadingNoise(2*M_PI/180.);
    localizer.setBufferSize(20);
    localizer.setNOutliers(5);


    while( simu.nextStep() != -1){

        // Current point of the trajectory
        Pose &p = simu.currentPose();

        // Draw the current situation
        vibes::clearFigure("test");
        simu.drawMap();
        vibes::drawAUV(p.x, p.y, p.theta*180/M_PI, 1);


        //*****************************************************
        localizer.predict(p.speed, p.theta, p.t);
        //*****************************************************

        double d = simu.genSonarDist(p.x, p.y, p.theta, p.t);
        if( d >= 0){ // the measurment is valid
           //*****************************************************
            double alpha = simu.getSonarAngle(p.t) + p.theta; // sonar beam angle
            Interval ialpha = Interval(alpha).inflate(2*M_PI/180.0);
            Interval irho = Interval(d).inflate(0.2);
            localizer.update(irho, ialpha, p.t);
            //*****************************************************
        }

        // Draw Results
        // Current position with corrections
        // Pour Thomas, quand la boite est vide on la remplace par
        // la valeur de la boite inertielle pure.
        if (localizer.X_cur.is_empty()){
            localizer.X_cur[0] = localizer.x_inertial;
            localizer.X_cur[1] = localizer.y_inertial;
            localizer.pos[0] = localizer.X_cur;
        }
        const IntervalVector& tmp = localizer.X_cur;

        vibes::drawBox( tmp[0].lb(), tmp[0].ub(), tmp[1].lb(), tmp[1].ub(), "g" );

        // Current postion without corrections (inertial only)
        Interval &x_cur = localizer.x_inertial;
        Interval &y_cur = localizer.y_inertial;
        vibes::drawBox(x_cur.lb(), x_cur.ub(), y_cur.lb(), y_cur.ub(), "r");
        usleep(4000);
    }
    return 0;
}
