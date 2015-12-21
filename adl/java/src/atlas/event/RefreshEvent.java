/*
 * Created on Feb 27, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.event;


import java.util.*;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class RefreshEvent extends EventObject {
	
	private boolean _hasValue;
	private int _value;
	
	public RefreshEvent(Object source, boolean isHasValue, int value) {
        super(source);
      
        _hasValue = isHasValue;
        
        if(_hasValue)
        	_value = value;
        else
        	_value = 0;
    }
	
	public boolean IsHasValue() {
		return _hasValue;
	}
	
	public int GetValue() {
		return _value;
	}
}