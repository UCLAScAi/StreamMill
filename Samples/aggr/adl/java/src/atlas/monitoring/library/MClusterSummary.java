/*
 */
package atlas.monitoring.library;


public class MClusterSummary {
  protected double sumx;
  protected double sumy;
  protected int count;
  protected boolean processed;
  
  protected double maxX;
  protected double minX;
  protected double maxY;
  protected double minY;
  
  public MClusterSummary() {
    sumx = 0;
    sumy = 0;
    count = 0;
    setProcessed(false);
    
    maxX =0;
    maxY = 0;
    minX = 0;
    minY = 0;
  }
  
  public MClusterSummary(double x, double y) {
    sumx = x;
    sumy = y;
    count = 1;
    setProcessed(false);
    
    maxX = x;
    minX = x;
    maxY = y;
    maxY = y;
  }
  
  private void updateMaxMins(double x, double y) {
    if(x > maxX) {
      maxX = x;
    }
    if(x < minX) {
      minX = x;
    }
    
    if(y > maxY) {
      maxY = y;
    }
    if(y < minY) {
      minY = y;
    }
  }
  
  public void add(int cnt) {
    count=count+cnt;
  }
  
  public void addXY(double x, double y) {
    sumx = sumx+x;
    sumy = sumy+y;
    count++;
    
    updateMaxMins(x, y);
  }
  
  public double getAverageX() {
    if(count == 0) {
      return 0;
    }
    else return sumx/count;
  }
  
  public double getAverageY() {
    if(count == 0) {
      return 0;
    }
    else return sumy/count;
  }
  
  public int getCount() {
    return count;
  }

	public void setProcessed(boolean processed) {
		this.processed = processed;
	}

	public boolean isProcessed() {
		return processed;
	}
  
  public double getMaxDistanceAllowed() {
    //used to match cluster in a window with clusters in the previous window
    return Math.sqrt(Math.pow(maxX-minX, 2) + Math.pow(maxY-minY, 2))/2;
    //this formula is pretty arbitrary, use it because it seems to work
    //may need modifications if gives wrong results
  }
}
