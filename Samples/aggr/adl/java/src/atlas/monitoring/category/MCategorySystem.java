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
public class MCategorySystem extends MCategory {

	public static final String VIEW_PERFORMANCES_COMMAND = "ViewPerformances";
	public static final String MONITOR_PERFORMANCES_COMMAND = "MonitorPerformances";
	public static final String UNMONITOR_PERFORMANCES_COMMAND = "UnMonitorPerformances";

	public MCategorySystem(String name) {
		super(name);
	}
	
	public String GetDonwloadingParametersCommand() {
		return VIEW_PERFORMANCES_COMMAND;
	}
	
	public String GetStartMonitoringCommand() {
		return MONITOR_PERFORMANCES_COMMAND;
	}
	
	public String GetStopMonitoringCommand() {
		return UNMONITOR_PERFORMANCES_COMMAND;
	}
}
