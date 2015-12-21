/*
 * Created on Feb 20, 2005
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
public class MElement {
	private String _name;
	private boolean _isMonitoring;
	
	public MElement(String name, boolean isMonitoring) {
		this._name = (name == null? "" : name);
		this._isMonitoring = isMonitoring;
	}
	
	public MElement(String name) {
		this._name = (name == null? "" : name);
		this._isMonitoring = false;
	}
	
	public String GetName() {
		return this._name;
	}
	
	public boolean IsMonitoring() {
		return this._isMonitoring;
	}
	
	public void SetMonitoring(boolean isMonitoring) {
		this._isMonitoring = isMonitoring;
	}
}
