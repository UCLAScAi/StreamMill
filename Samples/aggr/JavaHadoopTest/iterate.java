public class iterate 
{

	static {
                System.out.println("MAIN PATH : "  + System.getProperty("java.library.path"));

		System.loadLibrary("iterate");
		
	}


	public native static int hadoopiterate(int s);
	public native static int hadoopterminate(int cur, int next);

	public static void main(String[] argv)
	{
		int retval_1 = 0;
		int retval_2 = 0;
		int result = 0;
		retval_1 = hadoopiterate(1);
		System.out.println("Invocation returned " + retval_1);
	}
}
