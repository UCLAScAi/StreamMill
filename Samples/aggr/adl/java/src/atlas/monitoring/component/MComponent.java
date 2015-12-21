/*
 * Created on Feb 20, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import java.awt.Dimension;

import javax.swing.JComponent;


/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MComponent extends JComponent{
	
	private String _type;
	protected Dimension _preferredSize = new Dimension(0,0);
	
	/**
	 * 
	 * @param type
	 */
	public MComponent(String type) {
		if(type==null)
			type = "Unknow";
		
		this._type = type;
	}
	
	/**
	 * Text or waveform type
	 * @return
	 */
	public String GetMonitoringComponentType() {
		return this._type;
	}
	
	/**
	 * Get the PrefrredSize
	 */
	public Dimension getPreferredSize() {
		return _preferredSize;
	}
	
	public void setPreferredSize(Dimension dim) {
		_preferredSize = dim;
	}
	
	public void setPreferredSize(int width, int height) {
		_preferredSize = new Dimension(width,height);
	}
	

	
	/**
	 * Abstract classes
	 */
	abstract public void AddValue(String data);
	abstract public void CleanUp();
}
