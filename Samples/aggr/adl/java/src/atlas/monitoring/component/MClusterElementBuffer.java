/*
 * Created on Mar 12, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import atlas.monitoring.library.MGlobal;
import atlas.monitoring.library.MBinCounter;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MClusterElementBuffer {

	private boolean _isAllFadedIn;
	private boolean _isAllFadeOut;
	private int _nextElmentIdx;
	private int _slideWindowIdx;
	private MClusterElement[] _elements;
	
	private MBinCounter _clusterBinCnt;
	
	private int _max_x;
	private int _min_x;
	private int _max_y;
	private int _min_y;
	
	/**
	 * Constructor
	 * @param slideWindowIdx
	 * @param numOfElements
	 */
	public MClusterElementBuffer(int slideWindowIdx, int numOfElements) {
		_slideWindowIdx = slideWindowIdx;
		_elements = new MClusterElement[numOfElements];
		_nextElmentIdx = 0;
		_isAllFadedIn = false;
		_isAllFadeOut = false;
		_clusterBinCnt = new MBinCounter(numOfElements);
	}

	/**
	 * Get the cluster index
	 * @return
	 */
	public int GetClusterIndex() {
		return _slideWindowIdx;
	}
	
	public int[] GetBins() {
		return this._clusterBinCnt.GetBins();
	}
	
	public int GetMaxBinIndex() {
		return this._clusterBinCnt.GetMaxBinIndex();
	}
	
	public int GetMaxBinValue() {
		return this._clusterBinCnt.GetMaxBinValue();
	}
	
	public int GetNumActiveBin() {
		return this._clusterBinCnt.GetNumOfActiveBins();
	}
	
	public boolean IsAllFadedIn() {
		return _isAllFadedIn;
	}
	
	public boolean IsAllFadeout() {
		return _isAllFadeOut;
	}
	
	public int GetMaxX() {
		return _max_x;
	}
	
	public int GetMaxY() {
		return _max_y;
	}
	
	public int GetMinX() {
		return _min_x;
	}
	
	public int GetMinY() {
		return _min_y;
	}
	
	/**
	 * Add the element in the list
	 * @param element
	 */
	public void AddElement(MClusterElement element) {
		
		if(element == null || _nextElmentIdx>=_elements.length)
		{
			System.out.println("Drop elements");
			
			return;	//packet drop
		}
		_elements[_nextElmentIdx] = null;
		_elements[_nextElmentIdx] = element;
		
		setMaxMin(element.GetX(), element.GetY(), (_nextElmentIdx==0));
		
		_nextElmentIdx++;
		
		_clusterBinCnt.Inc(element.GetClusterId());
		
		//at least one element is not faded in
		SetAllFadeIn(false);
	}
	
	private void setMaxMin(int x, int y, boolean isFirstOne) {
		
		if(isFirstOne) {
			_max_x = x;
			_min_x = x;
			_max_y = y;
			_min_y = y;
		}
		else {
			if(x > _max_x) 
			{
				_max_x = x;
			}
			else if(x < _min_x) {
				_min_x  = x;
			}
			
			if(y > _max_y) {
				_max_y = y;
			}
			else if(y < _min_y) {
				_min_y = y;
			}
		}
		
		//System.out.println( "x: " + _min_x + "," + _max_x + "   " + _min_y + "," + _max_y);
	}
	
	/**
	 * Get the number of elements in the list
	 */
	public int GetNumOfElements() {
		return _nextElmentIdx;
	}
	
	/**
	 * Get element at 
	 * @param i
	 * @return
	 */
	public MClusterElement GetElementAt(int i) {
		if(i>=_nextElmentIdx)
			return null;
		
		return _elements[i];
	}
	
	/**
	 * Increase all the fade index for all element
	 *
	 */
	public void IncFadeIdxToAllElement() {
		
		boolean isAllFadedIn = _isAllFadedIn;
		
		if(isAllFadedIn)
			return;
		
		isAllFadedIn = true;
		
		for(int i=0; i<_nextElmentIdx-1; i++) {
			
			int value = _elements[i].IncFadeIdx();
			
			if(value != MGlobal.FADE_IDX_MAX) {
				isAllFadedIn = false;
			}
		}
		
		SetAllFadeIn(isAllFadedIn);
	}
	
	/**
	 * Decrease the fade index for all the element
	 */
	public void DecFadeIdxToAllElement() {
		
		boolean isAllFadedOut = _isAllFadeOut;
		
		if(isAllFadedOut)
			return;
		
		isAllFadedOut = true;
		
		for(int i=0; i<_nextElmentIdx; i++) {
			if(_elements[i].DecFadeIdx() != MGlobal.FADE_IDX_MIN) {
				isAllFadedOut = false;
			}
		}
		SetAllFadeOut(isAllFadedOut);
	}
	
	/**
	 * set the all fade in state
	 * @param val
	 */
	public synchronized void SetAllFadeIn(boolean val) {
		this._isAllFadedIn = val;
	}
	
	/**
	 * set the all fade out state
	 * @param val
	 */
	public synchronized void SetAllFadeOut(boolean val) {
		this._isAllFadeOut = val;
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#finalize()
	 */
	protected void finalize() throws Throwable {
		// TODO Auto-generated method stub
		super.finalize();
		
		for( int i=0; i<_elements.length;i++) {
			_elements[i] = null;
		}
		
		if(_clusterBinCnt!=null)
			_clusterBinCnt.CleanUp();
	}
	
//*****************************************************
//* NO USE NOW (May be use later)
//*****************************************************
	
	
//	private void addElementValue(int x, int y, int clusterId) {
//		if(_elements[_nextElmentIdx]==null) {
//			_elements[_nextElmentIdx] = new MClusterElement(x,y,clusterId,MGlobal.FADE_IDX_MIN);
//		}
//		else {
//			_elements[_nextElmentIdx].Set(x,y,clusterId,MGlobal.FADE_IDX_MIN);
//		}
//		_nextElmentIdx++;
//	}
	
//	/**
//	 * Add the value to 
//	 * @param x
//	 * @param y
//	 * @param clusterId
//	 */
//	public void AddValue(int x, int y, int clusterId) {
//		if(_nextElmentIdx>=_elements.length)
//			return;	//packet drop
//		
//		addElementValue(x,y,clusterId);
//	}
//	public void Clear() {
//		_nextElmentIdx = 0;
//	}
	
//	public void SetClusterIndex(int slideWindowIdx) {
//	_slideWindowIdx = slideWindowIdx;
//}	
	
}
