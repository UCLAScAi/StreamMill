/*
 */
package atlas.monitoring.library;


public class MBinSummaryUI {
  private int binIndex;
  private int value;
  private boolean isOld;
  private boolean hasDis;
  
  
  public MBinSummaryUI() {
    setBinIndex(-1);
    setValue(0);
    setOld(true);
    setHasDis(true);
  }
  
  public MBinSummaryUI(int binId, int val, boolean isO, boolean hasDis) {
    setBinIndex(binId);
    setValue(val);
    setOld(isO);
    setHasDis(hasDis);
  }

	public void setBinIndex(int binIndex) {
		this.binIndex = binIndex;
	}

	public int getBinIndex() {
		return binIndex;
	}

	public void setValue(int value) {
		this.value = value;
	}

	public int getValue() {
		return value;
	}

	public void setOld(boolean isOld) {
		this.isOld = isOld;
	}

	public boolean isOld() {
		return isOld;
	}

	public void setHasDis(boolean hasDis) {
		this.hasDis = hasDis;
	}

	public boolean isHasDis() {
		return hasDis;
	}  
}
