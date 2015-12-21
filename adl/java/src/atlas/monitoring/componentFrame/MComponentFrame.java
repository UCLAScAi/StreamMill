/*
 * Created on Feb 21, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.componentFrame;

import javax.swing.event.InternalFrameEvent;

import atlas.event.IRemoveMeEventListener;
import atlas.event.RemoveMeEvent;
import atlas.gui.MenuInternalFrame;
import atlas.monitoring.component.MComponent;
import atlas.monitoring.library.MGlobal;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MComponentFrame extends MenuInternalFrame {

	protected MComponent _monitoringComponent;
	
	protected int _fieldIndex = 1;
	protected int _dataType;
	private String _parameter_id;
	private String _frameName;

	protected javax.swing.event.EventListenerList _removeMeListenerList = new javax.swing.event.EventListenerList();
	
	public MComponentFrame(MComponent com, String frameName, String parameter_id, int dataType, Object[] objs ) {
		super(com,frameName,"<" + MGlobal.getDescription(dataType) + "> " + parameter_id);
		
		this._monitoringComponent = com;

		this.setName(frameName);
		this._frameName = frameName;
		this._parameter_id = parameter_id;
		this._dataType = dataType;
		this._fieldIndex = MGlobal.DEFAULT_FIELD_INDEX;
	}
	
	
	
    public void CleanUp() {
    	if(_monitoringComponent != null) {
    		_monitoringComponent.CleanUp();
    		_monitoringComponent = null;
    	}
    }
    
    public void internalFrameClosing(InternalFrameEvent e) { 
    	CleanUp();
    	fireRemoveMeEvent(new RemoveMeEvent(this));
    }
    
    public void Shutdown() {
    	CleanUp();
    	setVisible(false);
    }

	//***************************************
	// Get methods
	//***************************************   
    
    public String GetMComponentFrameType() {
		return this.getName();
	}
    
    public String GetParameterID() {
    	return _parameter_id;
    }
    
	public int GetDataType() {
		return _dataType;
	}
	
	public int GetFieldIndex() {
		return _fieldIndex;
	}
    
	//***************************************
	// Event handling
	//***************************************
    
	public void AddRemoveMeEventListener(IRemoveMeEventListener listener) {
		_removeMeListenerList.add(IRemoveMeEventListener.class, listener);
    }

    // This methods allows classes to unregister for MyEvents
    public void RemoveRemoveMeEventListener(IRemoveMeEventListener listener) {
    	_removeMeListenerList.remove(IRemoveMeEventListener.class, listener);
    }

    // This private class is used to fire MyEvents
    private void fireRemoveMeEvent(RemoveMeEvent evt) {
        Object[] listeners = _removeMeListenerList.getListenerList();
        // Each listener occupies two elements - the first is the listener class
        // and the second is the listener instance
        for (int i=0; i<listeners.length; i+=2) {
            if (listeners[i]==IRemoveMeEventListener.class) {
                ((IRemoveMeEventListener)listeners[i+1]).RemoveMeEventOccurred(evt);
            }
        }
    }
    
    //
    // Abstract method
    //
    abstract public void AddValue(String[] textValue);
}
