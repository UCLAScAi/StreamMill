public class SumUda 
{

	static {
		System.loadLibrary("SumUda");
	}


	public native static int myiterate(int s);
	public native static int terminate(int cur, int next);

	public static void main(String[] argv)
	{
		int retval_1 = 0;
		int retval_2 = 0;
		int result = 0;
		retval_1 = myiterate(1);
		retval_2 = myiterate(2);
		System.out.println("Invocation returned " + retval_1 + " and " + retval_2);
		result =terminate(retval_1,retval_2);
		System.out.println("Invocation returned " + result);
	}
}
