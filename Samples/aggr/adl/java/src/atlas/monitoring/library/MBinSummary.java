/*
 * Created on Mar 14, 2005
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
public class MBinSummary {

	private MClusterSummary[] _bins;
	
	private int _numOfActiveBin;
	private int _maxBinIndex;
	private int _maxBinValue;
	
	private boolean _isDynamicResize;
	private int _resizeTime = 2;
	private int _size;
	
	public MBinSummary(int size) {
		_bins = new MClusterSummary[size];

    for(int i = 0; i < _bins.length; i++) {
      _bins[i] = new MClusterSummary();
    }
		_maxBinIndex = 0;
		_numOfActiveBin = 0;
		_maxBinValue = 0;
		_isDynamicResize = false;
		_size = size;
	}
	
	public void SetDynamicResize(boolean choice) {
		_isDynamicResize = choice;
	}
	
  
  public void Add(int cnt, int bin) {
  
    if(bin<0 )
      return;
    
    if(bin>=_bins.length) {
      if(!_isDynamicResize) {
        return;
      }
      
      int newSize = (_resizeTime++) * _size;
      
      while(bin>=newSize) {
        newSize = (_resizeTime++) * _size;
      }
      
      MClusterSummary[] newBin = new MClusterSummary[newSize];
      
      for( int i=0; i<_bins.length; i++ ) {
        newBin[i] = _bins[i];
      }
      for(int i = _bins.length; i <newBin.length; i++) {
        newBin[i] = new MClusterSummary();
      }
      
      _bins = newBin;
    }
    
    if(bin>_maxBinIndex)
      _maxBinIndex=bin;

    //System.out.println( "bin,maxbinindex = "  + bin + "," + _maxBinIndex);
    
    if(_bins[bin].getCount()==0)
      _numOfActiveBin++;
    
    _bins[bin].add(cnt);
    
    if(_bins[bin].getCount()>_maxBinValue)
      _maxBinValue=_bins[bin].getCount();

  } 
  
	public void Add(int x, int y, int bin) {
	
		if(bin<0 )
			return;
		
		if(bin>=_bins.length) {
			if(!_isDynamicResize) {
				return;
			}
			
			int newSize = (_resizeTime++) * _size;
			
			while(bin>=newSize) {
				newSize = (_resizeTime++) * _size;
			}
			
			MClusterSummary[] newBin = new MClusterSummary[newSize];
			
			for( int i=0; i<_bins.length; i++ ) {
				newBin[i] = _bins[i];
			}
      for(int i = _bins.length; i <newBin.length; i++) {
        newBin[i] = new MClusterSummary();
      }
			
			_bins = newBin;
		}
		
		if(bin>_maxBinIndex)
			_maxBinIndex=bin;

		//System.out.println( "bin,maxbinindex = "  + bin + "," + _maxBinIndex);
    
		if(_bins[bin].getCount()==0)
			_numOfActiveBin++;
		
		_bins[bin].addXY(x, y);
		
		if(_bins[bin].getCount()>_maxBinValue)
			_maxBinValue=_bins[bin].getCount();

	}	
	
	//*********************************************
	
	public void CleanUp() {
		_bins = null;
		_maxBinIndex = 0;
		_numOfActiveBin = 0;
	}
	
	public int GetMaxBinValue() {
		return this._maxBinValue;
	}
	
	public int GetNumOfActiveBins() {
		return this._numOfActiveBin;
	}
	
	public MClusterSummary[] GetBins() {
		return this._bins;
	}
	
	public int GetMaxBinIndex() {
		return this._maxBinIndex;
	}
	
	public String toString() {
		return "Active bin = " + _numOfActiveBin + ", max bin value = " + _maxBinValue + ", index = " + _maxBinIndex;
	}
}
