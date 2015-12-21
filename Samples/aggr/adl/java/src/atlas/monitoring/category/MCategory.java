/*
 * Created on Feb 18, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.category;

import java.util.List;
import java.util.Vector;

import atlas.monitoring.library.MElement;



/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MCategory {
	//
	// Variable
	//
	private String _name;
	private MElement[] _elements; 
	private boolean _isParameterLoaded = false;
								
	//
	// Public Method
	//
	public MCategory( String name ) {
		this._name = name;
	}
	
	final public String GetName() {
		return this._name;
	}
	
	public boolean IsParameterLoaded() {
		return this._isParameterLoaded;
	}
	
	public int GetNumOfParameters() {
		return GetMonitoringElements().length;
	}
	
	public MElement GetMonitoringElementAt(int i){
		if(i<0 || i>GetMonitoringElements().length) {
			return null;
		}
		
		return GetMonitoringElements()[i];
	}
			
	public void ParseMonitoringParametersList(List list) {
		 
		Vector v = new Vector(list.size());
		
		boolean isMonitoring = false;
		String parameterName = null;
		String hi = null;
		String[] part = null;
		MElement element = null;
				
		for( int i=0; i<list.size();i++) {
			try {
				hi = (String)list.get(i);
				part = hi.split("\\s");
				
				isMonitoring = part[0].equalsIgnoreCase("yes") ? true : false;
				parameterName = part[1];
				
				if(parameterName==null||parameterName.length()==0)
					continue;
											
				element = new MElement(parameterName,isMonitoring);
				
				v.add(element);
			}
			catch(Exception ex) {
				ex.printStackTrace();
			}
		}
		
		try {
			v.trimToSize();
		
			this._elements = new MElement[v.size()];
		
			v.toArray(this._elements);
			
			this._isParameterLoaded = true;
		}
		catch(Exception ex) {
			this._elements = new MElement[0];
		}
	}
	
	abstract public String GetDonwloadingParametersCommand();
	abstract public String GetStartMonitoringCommand();
	abstract public String GetStopMonitoringCommand();
	
	//
	// protected method
	//
	final public MElement[] GetMonitoringElements() {
		//use this method to avoid return null and cause exception
		if(this._elements==null)
			this._elements = new MElement[0];
		
		return this._elements;
	}
}
