/*
 * Created on Mar 12, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import atlas.monitoring.library.MGlobal;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MClusterElement {

	private boolean _hasValue;
	private int _x;
	private int _y;
	private int _clusterId;
	private int _fadeIdx;
	private int _slideWindowIdx;
	
	public MClusterElement(int x, int y, int clusterId, int fadeIdx, int slideWindowIdx) {
		_x = x;
		_y = y;
		_clusterId = clusterId;
		_fadeIdx = fadeIdx;
		_slideWindowIdx = slideWindowIdx;
		_hasValue = true;
	}
	
	public void Set(int x, int y, int clusterId, int fadeIdx, int slideWindowIdx) {
		_x = x;
		_y = y;
		_clusterId = clusterId;
		_fadeIdx = fadeIdx;
		_slideWindowIdx = slideWindowIdx;
		_hasValue = true;
	}
	
	public void Clear() {
		_hasValue = false;
	}
	
	public boolean HasValue() {
		return _hasValue;
	}
	
	public int GetX() {
		return _x;
	}
	
	public int GetY() {
		return _y;
	}
	
	public int GetClusterId() {
		return _clusterId;
	}
	
	public int GetFadeIdx() {
		return _fadeIdx;
	}
	
	public int GetClusterIndex() {
		return _slideWindowIdx;
	}
	
	public int IncFadeIdx() {
		_fadeIdx++;
		
		_fadeIdx = (_fadeIdx > MGlobal.FADE_IDX_MAX ? MGlobal.FADE_IDX_MAX : _fadeIdx);
		
		return _fadeIdx;
	}
	
	public int DecFadeIdx() {
		_fadeIdx--;
		
		_fadeIdx = (_fadeIdx < MGlobal.FADE_IDX_MIN ? MGlobal.FADE_IDX_MIN : _fadeIdx);
		
		return _fadeIdx;
	}
}
