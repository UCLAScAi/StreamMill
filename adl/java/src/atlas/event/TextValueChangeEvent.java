/*
 * Created on Feb 27, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.event;

import java.util.EventObject;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class TextValueChangeEvent extends EventObject  {

	private String _name;
	private String _value;
		
	public TextValueChangeEvent(Object source, String name, String value) {
        super(source);
      
        _name = name;
        _value = value;
        
    }
	
	public String GetName() {
		return this._name;
	}
	
	public String GetValue() {
		return _value;
	}
}
