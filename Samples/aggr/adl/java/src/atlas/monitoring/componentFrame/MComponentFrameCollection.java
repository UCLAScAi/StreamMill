/*
 * Created on Mar 6, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.componentFrame;

import java.util.Enumeration;
import java.util.Vector;

import atlas.event.IRemoveMeEventListener;
import atlas.event.RemoveMeEvent;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentFrameCollection implements IRemoveMeEventListener {
	
	private int _dataType;
	private String _parameterId;
	
	private Vector _vMonitoringCom;
	private javax.swing.event.EventListenerList _removeMeListenerList = new javax.swing.event.EventListenerList();
	
	public MComponentFrameCollection(int dataType, String parameterID) {
		
		if(parameterID==null||parameterID.length()==0)
			parameterID = "unknown";
		
		this._parameterId = parameterID;
		this._dataType = dataType;
		
		_vMonitoringCom = new Vector();
	}
	
	public int GetDataType() {
		return this._dataType;
	}
	
	public void AddMComponentFrame(MComponentFrame com) {
		
		_vMonitoringCom.add(com);
		
		com.AddRemoveMeEventListener(this);
	}

	public String GetParameterId() {
		return this._parameterId;
	}
	
	/**
	 * Handle the come request to remove in the storage
	 */
	public void RemoveMeEventOccurred(RemoveMeEvent ce) {
			
		try {
			_vMonitoringCom.remove(ce.getSource());

			//Please remove me if I contain no frame
			if(_vMonitoringCom.size()==0) {
				fireRemoveMeEvent(new RemoveMeEvent(this));
			}
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
	}
	
	/**
	 * Check to see if the frame type and monitoring field exist
	 * @param frameType
	 * @param fieldIndex
	 * @return
	 */
	public boolean isExist(String frameType, int fieldIndex) {
		
		Enumeration enum = _vMonitoringCom.elements();
		MComponentFrame mcf;
		while(enum.hasMoreElements()) {
			mcf = (MComponentFrame)enum.nextElement();
			
			if(mcf.GetMComponentFrameType()==frameType &&
					mcf.GetFieldIndex() == fieldIndex ) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * Return number of frame store in the frame collection
	 * @return The number of frame
	 */
	public int GetNumOfFrame() {
		return _vMonitoringCom.size();
	}
	
	/**
	 * Clear all components
	 */
	public void Clear() {

		int len = _vMonitoringCom.size();
		
		for(int i=0; i<len; i++) {
			//don't use do default action that will cause cycle effect
			//cause by the fireRemoveMeEvent
			((MComponentFrame)_vMonitoringCom.get(i)).Shutdown();
		}
		
		_vMonitoringCom.removeAllElements();
		
		fireRemoveMeEvent(new RemoveMeEvent(this));
	}
	
	/**
	 * Dispatch the data to each component frame
	 * @param rawData
	 */
	public void Dispatch(String[] rawData) {
		Enumeration enum = _vMonitoringCom.elements();
		
		while(enum.hasMoreElements()) {
			((MComponentFrame)enum.nextElement()).AddValue(rawData);
		}
	}
	
	//************************************************
	// Event handling
	//************************************************
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
}
