/*  linreg.h
    Linear Regression calculation class

    by: David C. Swaim II, Ph.D.

    This class implements a standard linear regression on
    experimental data using a least squares fit to a straight
    line graph.  Calculates coefficients a and b of the equation:

                        y = a + b * x

    for data points of x and y.  Also calculates the coefficient of
    determination, the coefficient of correlation, and standard
    error of estimate.

    The value n (number of points) must be greater than 2 to
    calculate the regression.  This is primarily because the
    standard error has a (N-2) in the denominator.

    Check haveData() to see if there is enough data in
    LinearRegression to get values.

    You can think of the x,y pairs as 2 dimensional points.
    The class Point2D is included to allow pairing x and y
    values together to represent a point on a plane.

*/
#include <iostream.h>

#define WINDOW_LR 3 
class Point2D
{
    public:
        Point2D(double X = 0.0, double Y = 0.0) : x(X), y(Y) { }

        void setPoint(double X, double Y) { x = X; y = Y; }
        void setX(double X) { x = X; }
        void setY(double Y) { y = Y; }

        double getX() const { return x; }
        double getY() const { return y; }

    private:
        double x, y;
};

class LinearRegression
{
    friend ostream& operator<<(ostream& out, LinearRegression& lr);

    public:
        // Constructor using an array of Point2D objects
        // This is also the default constructor
        LinearRegression(Point2D *p = 0, long size = 0);
        // Constructor using arrays of x values and y values
        LinearRegression(double *x, double *y, long size = 0);
        LinearRegression(int i);

virtual void addXY(const double& x, const double& y);
        void addPoint(const Point2D& p) { addXY(p.getX(), p.getY()); }
        int iterateLR(Point2D nextPt);
        void giveLRResults(char** tmp);
        void reset_LP();
        // Must have at least 3 points to calculate
        // standard error of estimate.  Do we have enough data?
        int haveData() const { return (n > 2 ? 1 : 0); }
        long items() const { return n; }

virtual double getA() const { return a; }
virtual double getB() const { return b; }

        double getCoefDeterm() const  { return coefD; }
        double getCoefCorrel() const { return coefC; }
        double getStdErrorEst() const { return stdError; }
virtual double estimateY(double x) const { return (a + b * x); }

    protected:
        long n;             // number of data points input so far
        int hasMoreInWindow; 
        double sumX, sumY;  // sums of x and y
        double sumXsquared, // sum of x squares
               sumYsquared; // sum y squares
        double sumXY;       // sum of x*y

        double a, b;        // coefficients of f(x) = a + b*x
        double coefD,       // coefficient of determination
               coefC,       // coefficient of correlation
               stdError;    // standard error of estimate
        long i;

        void Calculate();   // calculate coefficients
};

