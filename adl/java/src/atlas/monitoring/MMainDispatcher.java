/*
 * Created on Feb 19, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring;

import atlas.monitoring.componentFrame.MComponentFrameDispatcher;
import atlas.monitoring.library.MGlobal;
import atlas.monitoring.library.MPacket;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MMainDispatcher {

	private MComponentFrameDispatcher[] _dispatchers;
	
	public MMainDispatcher(MComponentFrameDispatcher[] dispatchers) {
		if(dispatchers==null) {
			this._dispatchers = new MComponentFrameDispatcher[0];
		}
		else {
			this._dispatchers = dispatchers;
		}
	}
	
	public void DispatchData(String rawData) {
		
		String[] toks = rawData.split("\\|\\|");
		
		//System.out.println("MonitoringMainDispatcher.dispatchData = " + rawData);
		
		for(int i=0; i<toks.length; i++) {
			toks[i] = toks[i].trim();
		}
		
		/**
		 * Example: rawData = "d||123_456_789_000||1$2$3$4"
		 * toks[0] = dataType
		 * toks[1] = parameter_id
		 * toks[2] = data
		 */
		if(toks.length>=3) {
			int dataType = MGlobal.GetDataType(toks[0]);
			String bufName;
			String data = toks[2];
			
			if(dataType==MGlobal.DATA_TYPE_IDX_QUERY_OUTPUT ||
				dataType==MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_BUFFER_SIZE ||
				dataType==MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_DELAY ||
				dataType==MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_PRO_TIME ) {
				//bufName = "stdout_" + toks[1];  //Hetal -- trying to do it right
        bufName = toks[1];
			}
			else {
				bufName = toks[1];
			}
			
			for(int i=0; i<_dispatchers.length; i++) {
				_dispatchers[i].AddMonitoringPacket(new MPacket(dataType,bufName,data));
			}
		}
	}
}
