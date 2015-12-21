/*
 * Created on Feb 19, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.category;



/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MCategoryQuery extends MCategory {
	
	public static final String VIEW_BUFFERS_COMMAND = "ViewBuffers";
	public static final String MONITOR_BUFFER_COMMAND = "MonitorBuffer";
	public static final String UNMONITOR_BUFFER_COMMAND = "UnMonitorBuffer";
  public static final String UNMONITOR_ALL_OF_IP_COMMAND = "UnMonitorAllOfIP";
	
	public MCategoryQuery( String name ) {
		super(name);
	}	
	
	public String GetDonwloadingParametersCommand() {
		return VIEW_BUFFERS_COMMAND;
	}
	
	public String GetStartMonitoringCommand() {
		return MONITOR_BUFFER_COMMAND;
	}
	
	public String GetStopMonitoringCommand() {
		return UNMONITOR_BUFFER_COMMAND;
	}
}
