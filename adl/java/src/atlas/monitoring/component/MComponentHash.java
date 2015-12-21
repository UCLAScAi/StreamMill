/*
 * Created on Mar 6, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import java.util.Enumeration;
import java.util.Hashtable;

import atlas.event.IRemoveMeEventListener;
import atlas.event.RemoveMeEvent;
import atlas.monitoring.componentFrame.MComponentFrame;
import atlas.monitoring.componentFrame.MComponentFrameCollection;
import atlas.monitoring.library.MGlobal;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentHash implements IRemoveMeEventListener {

	private Hashtable _dataTypeHash;
	
	public MComponentHash() {
		_dataTypeHash = new Hashtable();
	}
	
	/**
	 * Add new monitoring component frame into the storage
	 * @param dataType
	 * @param parameterId
	 * @param frame
	 */
	public void AddFrame(MComponentFrame frame) {
		try {
			
			String parameterId = frame.GetParameterID();
			
			int dataType = frame.GetDataType();
			
			Hashtable frameCollectionHash = getFrameCollectionHash(dataType);
			MComponentFrameCollection frameCollection;
			
			if(frameCollectionHash.containsKey(parameterId)) {
				frameCollection = (MComponentFrameCollection)frameCollectionHash.get(parameterId);
			}
			else {
				frameCollection = new MComponentFrameCollection(dataType,parameterId);
				frameCollection.AddRemoveMeEventListener(this);
				frameCollectionHash.put(parameterId,frameCollection);
			}
			frameCollection.AddMComponentFrame(frame);
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
	}
	
	/**
	 * Get the frame collect hash base on the given data type
	 * @param dataType {@link MGlobal} 
	 * @return
	 */
	private Hashtable getFrameCollectionHash(int dataType) {
		
		String dataTypeString = String.valueOf(dataType);
		Hashtable frameCollectionHash;
		
		if(_dataTypeHash.containsKey(dataTypeString)) {
			frameCollectionHash = (Hashtable)_dataTypeHash.get(dataTypeString);
		}
		else {
			frameCollectionHash = new Hashtable();
			_dataTypeHash.put(dataTypeString,frameCollectionHash);
		}
		return frameCollectionHash;		
	}
	
	/**
	 * Remove the frameCollection from frameCollectionHash
	 */
	public void RemoveMeEventOccurred(RemoveMeEvent ce) {
		try {

			MComponentFrameCollection frameCollection = (MComponentFrameCollection)ce.getSource();
	
			Hashtable frameCollectionHash = getFrameCollectionHash(frameCollection.GetDataType());
			
			String id = frameCollection.GetParameterId();
			
			if(frameCollectionHash.containsKey(id)) {
				frameCollectionHash.remove(id);
			}
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
	}
	
	/**
	 * Remave all the frame by parameterId
	 * @param parameterId
	 */
	public void RemoveByParameterId(String parameterId) {
		try {

			Enumeration enumDataTypeHash = _dataTypeHash.elements();
			Hashtable frameCollectionHash;
			MComponentFrameCollection frameCollection;
			
			while(enumDataTypeHash.hasMoreElements()) {
				frameCollectionHash = (Hashtable)enumDataTypeHash.nextElement();
				
				if(frameCollectionHash.containsKey(parameterId)) {
					
					frameCollection = (MComponentFrameCollection)frameCollectionHash.get(parameterId);
					
					frameCollection.Clear();	//shut down all frames
				}
			}
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
	}
	
	/**
	 * Remove everything in the storage
	 *
	 */
	public void Clear() {
		
		Enumeration enumDataTypeHash = _dataTypeHash.elements();
		Enumeration enumframeCollectionHash;
		Hashtable frameCollectionHash;
		MComponentFrameCollection frameCollection;
		
		while(enumDataTypeHash.hasMoreElements()) {
			frameCollectionHash = (Hashtable)enumDataTypeHash.nextElement();
			
			enumframeCollectionHash = frameCollectionHash.elements();
			
			while(enumframeCollectionHash.hasMoreElements()) {
				frameCollection = (MComponentFrameCollection)enumframeCollectionHash.nextElement();
				
				frameCollection.Clear();
			}
			
			frameCollectionHash.clear();
		}
		_dataTypeHash.clear();
	}
	
	public boolean IsFrameExist(MComponentFrame frame) {
		
		try {
			String parameterId = frame.GetParameterID();
			String key = String.valueOf(frame.GetDataType());
			
			if(_dataTypeHash.containsKey(key)) {
				if(((Hashtable)_dataTypeHash.get(key)).containsKey(parameterId)) {
					return true;
				}
			}
		}
		catch(Exception ex) {}
		return false;
	}
	
	/**
	 * Dispatch the data to correspond dataType and parameterID frame
	 * @param dataType
	 * @param parameterId
	 * @param data
	 */
	public void Dispatch(int dataType, String parameterId, String raw) {
		
		String key = String.valueOf(dataType);
		Hashtable frameCollectionHash;
		MComponentFrameCollection frameCollection;
		
		if(_dataTypeHash.containsKey(key)) {
			
			frameCollectionHash = (Hashtable)_dataTypeHash.get(key);
			
			if(frameCollectionHash.containsKey(parameterId)) {
				frameCollection = (MComponentFrameCollection)frameCollectionHash.get(parameterId);
				String toks[] = raw.split("\\$");
				
				for(int i=0; i<toks.length; i++) {
					toks[i] = toks[i].trim();
				}
				frameCollection.Dispatch(toks);
			}
		}
	}
	
	//********************************************
	// Testing purpose: count the internal components
	//********************************************
	
	/**
	 * Count the number of data types
	 */
	public int CountNumOfDataType() {
		return _dataTypeHash.size();
	}
	
	/**
	 * Count the number of frameCollection (parameter id)
	 * @return
	 */
	public int CountNumOfFrameCollection() {
		int cnt = 0;
		
		Enumeration enum = _dataTypeHash.elements();
		Hashtable frameCollectionHash;
		
		while(enum.hasMoreElements()) {
			frameCollectionHash = (Hashtable)enum.nextElement();
			
			cnt += frameCollectionHash.size();
			
			
		}
		
		return cnt;
	}
	
	private void dumpFrameCollectionHash(Hashtable hash) {
		Enumeration enum = hash.elements();
		
		while(enum.hasMoreElements()) {
			System.out.println(((MComponentFrameCollection)enum.nextElement()).GetParameterId());
		}
	}
	
	/**
	 * Count the number of Monitoring component frame
	 * @return
	 */
	public int CountNumOfFrame() {
		int cnt = 0;
		
		Enumeration enumDataTypeHash = _dataTypeHash.elements();
		Enumeration enumframeCollectionHash;
		Hashtable frameCollectionHash;
		MComponentFrameCollection frameCollection;
		
		while(enumDataTypeHash.hasMoreElements()) {
			frameCollectionHash = (Hashtable)enumDataTypeHash.nextElement();
			
			enumframeCollectionHash = frameCollectionHash.elements();
			
			while(enumframeCollectionHash.hasMoreElements()) {
				frameCollection = (MComponentFrameCollection)enumframeCollectionHash.nextElement();
				
				cnt += frameCollection.GetNumOfFrame();
			}
		}
		return cnt;
	}
}
