/*
 * Created on Mar 6, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.library;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MPacket {
	private int _dataType;
	private String _parameterId;
	private String _rawData;
	
	public MPacket(int dataType, String parameterId, String rawData) {
		_dataType = dataType;
		_parameterId = parameterId;
		_rawData = rawData;
	}
	
	public int GetDataType() {
		return this._dataType;
	}
	
	public String GetParameterId() {
		return this._parameterId;
	}
	
	public String GetRawData() {
		return this._rawData;
	}
}
