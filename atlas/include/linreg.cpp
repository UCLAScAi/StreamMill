/*
    file linreg.cpp
*/
#include <math.h>
#include <float.h>
#include "linreg.h"


LinearRegression::LinearRegression(Point2D *p, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addPoint(p[i]);


}

LinearRegression::LinearRegression(double *x, double *y, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addXY(x[i], y[i]);
}

/* Constructor for windowed case */
LinearRegression::LinearRegression(int a)
{
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;
    i=0;
}

void LinearRegression::reset_LP() {
   n = sumX = sumY = sumXsquared = sumYsquared = sumXY = a = b = coefD = coefC = stdError = hasMoreInWindow = 0;
}

void LinearRegression::addXY(const double& x, const double& y)
{
    hasMoreInWindow = WINDOW_LR;
    n++;
    sumX += x;
    sumY += y;
    sumXsquared += x * x;
    sumYsquared += y * y;
    sumXY += x * y;
    Calculate();
}

/* This is an iterate function which inserts points 
 * until window is not full
 */
int LinearRegression::iterateLR(Point2D nextPt) 
{
        addPoint(nextPt);
        i++; 
        if ((i == WINDOW_LR)) {
            //printf("here\n");
            //cout << "Reached the end of window " 
            //     << "f(x) = " << getA()
            //<< " + ( " << getB()
            //<< " * x )" << endl;
            //cout << 1 << endl;
            //sprintf(stdout, "%i",1);
            //char* tmp = (char*)malloc(1000*sizeof(char));
            //sprintf(tmp,"%s %f %s %f %s","f(x) = ",getA()," + ( ", getB(), " * x ) ");
            //printf("%s",tmp);
	}
}


void LinearRegression::giveLRResults(char** tmp) {
	if (i == WINDOW_LR){ 
           sprintf(*tmp,"%s %f %s %f %s","f(x) = ",getA()," + ( ", getB(), " * x ) ");
           i = 0;
           a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
           n = 0L;
        }
	else
	   *tmp = "window not full";
}

void LinearRegression::Calculate()
{
    if (haveData())
    {
        if (fabs( double(n) * sumXsquared - sumX * sumX) > DBL_EPSILON)
        {
            b = ( double(n) * sumXY - sumY * sumX) /
                ( double(n) * sumXsquared - sumX * sumX);
            a = (sumY - b * sumX) / double(n);

            double sx = b * ( sumXY - sumX * sumY / double(n) );
            double sy2 = sumYsquared - sumY * sumY / double(n);
            double sy = sy2 - sx;

            coefD = sx / sy2;
            coefC = sqrt(coefD);
            stdError = sqrt(sy / double(n - 2));
        }
        else
        {
            a = b = coefD = coefC = stdError = 0.0;
        }
    }
}

ostream& operator<<(ostream& out, LinearRegression& lr)
{
    if (lr.haveData())
        out << "f(x) = " << lr.getA()
            << " + ( " << lr.getB()
            << " * x )";
    return out;
}
