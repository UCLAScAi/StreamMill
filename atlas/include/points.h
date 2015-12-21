// points.h: interface for the points class.

struct pointDB
{
	float x,
	    y;
};


class C_pointDB 
{
	public:
		pointDB w;
		int *num;
		int count;
		~C_pointDB() {
			delete []num;
		}
};
