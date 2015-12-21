/*
 * Created on Mar 5, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.library;

import java.util.Hashtable;


/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MGlobal {
	
	public static final int FADE_IDX_MAX = 10;
	public static final int FADE_IDX_MIN = 0;		//must be zero
	
	//***************************************
	//* Datatype
	//***************************************
	
	public static final int DATA_TYPE_IDX_UNKNOWN = 0;				
	public static final int DATA_TYPE_IDX_QUERY_OUTPUT = 1;			
	public static final int DATA_TYPE_IDX_QUERY_PERFORMANCE_BUFFER_SIZE = 2;
	public static final int DATA_TYPE_IDX_QUERY_PERFORMANCE_DELAY = 3;
	public static final int DATA_TYPE_IDX_QUERY_PERFORMANCE_PRO_TIME = 4;
	public static final int DATA_TYPE_IDX_SYSTEM_PERFORMANCE = 5;
	
	public static final String DES_UNKNOWN = "unknown";				
	public static final String DES_QUERY_OUTPUT = "Ouput";			
	public static final String DES_QUERY_PERFORMANCE_BUFFER_SIZE = "buffer size";
	public static final String DES_QUERY_PERFORMANCE_DELAY = "delay";
	public static final String DES_QUERY_PERFORMANCE_PRO_TIME = "processing time";
	public static final String DES_SYSTEM_PERFORMANCE = "System";
	
	public static final String DATA_TYPE_STR_UNKNOWN = "u";				
	public static final String DATA_TYPE_STR_QUERY_OUTPUT = "o";			
	public static final String DATA_TYPE_STR_QUERY_PERFORMANCE_BUFFER_SIZE = "b";
	public static final String DATA_TYPE_STR_QUERY_PERFORMANCE_DELAY = "d";
	public static final String DATA_TYPE_STR_QUERY_PERFORMANCE_PRO_TIME = "p";
	public static final String DATA_TYPE_STR_SYSTEM_PERFORMANCE = "s";
	
	public static final int numDataType = 6;
	
	public static final int DEFAULT_FIELD_INDEX = 1;
	
	//********************************************
	// Data type hash
//	********************************************
	private static Hashtable _dataTypeSTRtoIDX;
	private static Hashtable _dataTypeIDXtoSTR;
	private static Hashtable _dataTypeDescription;
	
	static {
		try {
			_dataTypeSTRtoIDX = new Hashtable(numDataType);
			_dataTypeIDXtoSTR = new Hashtable(numDataType);
			_dataTypeDescription = new Hashtable(numDataType);
			
			//Sorry.... hopefully, i can have more time to organize this class better
			_dataTypeDescription.put(String.valueOf(DATA_TYPE_IDX_UNKNOWN),						DES_UNKNOWN);
			_dataTypeDescription.put(String.valueOf(DATA_TYPE_IDX_QUERY_OUTPUT),					DES_QUERY_OUTPUT);
			_dataTypeDescription.put(String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_BUFFER_SIZE),	DES_QUERY_PERFORMANCE_BUFFER_SIZE);
			_dataTypeDescription.put(String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_DELAY),		DES_QUERY_PERFORMANCE_DELAY);
			_dataTypeDescription.put(String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_PRO_TIME),		DES_QUERY_PERFORMANCE_PRO_TIME);
			_dataTypeDescription.put(String.valueOf(DATA_TYPE_IDX_SYSTEM_PERFORMANCE),				DES_SYSTEM_PERFORMANCE);
						
			_dataTypeSTRtoIDX.put(DATA_TYPE_STR_UNKNOWN,						String.valueOf(DATA_TYPE_IDX_UNKNOWN));
			_dataTypeSTRtoIDX.put(DATA_TYPE_STR_QUERY_OUTPUT,					String.valueOf(DATA_TYPE_IDX_QUERY_OUTPUT));
			_dataTypeSTRtoIDX.put(DATA_TYPE_STR_QUERY_PERFORMANCE_BUFFER_SIZE,	String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_BUFFER_SIZE));
			_dataTypeSTRtoIDX.put(DATA_TYPE_STR_QUERY_PERFORMANCE_DELAY,		String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_DELAY));
			_dataTypeSTRtoIDX.put(DATA_TYPE_STR_QUERY_PERFORMANCE_PRO_TIME,		String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_PRO_TIME));
			_dataTypeSTRtoIDX.put(DATA_TYPE_STR_SYSTEM_PERFORMANCE,				String.valueOf(DATA_TYPE_IDX_SYSTEM_PERFORMANCE));
			
			_dataTypeIDXtoSTR.put(String.valueOf(DATA_TYPE_IDX_UNKNOWN),						DATA_TYPE_STR_UNKNOWN);
			_dataTypeIDXtoSTR.put(String.valueOf(DATA_TYPE_IDX_QUERY_OUTPUT),					DATA_TYPE_STR_QUERY_OUTPUT);
			_dataTypeIDXtoSTR.put(String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_BUFFER_SIZE),	DATA_TYPE_STR_QUERY_PERFORMANCE_BUFFER_SIZE);
			_dataTypeIDXtoSTR.put(String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_DELAY),		DATA_TYPE_STR_QUERY_PERFORMANCE_DELAY);
			_dataTypeIDXtoSTR.put(String.valueOf(DATA_TYPE_IDX_QUERY_PERFORMANCE_PRO_TIME),		DATA_TYPE_STR_QUERY_PERFORMANCE_PRO_TIME);
			_dataTypeIDXtoSTR.put(String.valueOf(DATA_TYPE_IDX_SYSTEM_PERFORMANCE),				DATA_TYPE_STR_SYSTEM_PERFORMANCE);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * Get the dataType description
	 * @param dataType
	 * @return
	 */
	public static String getDescription(int dataType) {
		String key = String.valueOf(dataType);
		
		if(!_dataTypeDescription.containsKey(key))
			return DES_UNKNOWN;
		
		return (String)_dataTypeDescription.get(key);
	}
	
	/**
	 * Get the dataType base on the input key
	 * @param key
	 * @return
	 */
	public static int GetDataType(String key) {
		if(!_dataTypeSTRtoIDX.containsKey(key))
		{
			
			return DATA_TYPE_IDX_UNKNOWN;
		}
		
		return Integer.parseInt((String)_dataTypeSTRtoIDX.get(key));
	}
	
	/**
	 * Get the data type string
	 * @param dataType
	 * @return
	 */
	public static String GetDataTypeKey(int dataType) {
		String key = String.valueOf(dataType);
		
		if(!_dataTypeIDXtoSTR.containsKey(key)) {
			return DATA_TYPE_STR_UNKNOWN;
		}
		return (String)_dataTypeIDXtoSTR.get(key);
	}
}
