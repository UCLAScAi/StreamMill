/*
 * Created on Mar 23, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MComponentHistogram  extends MComponent {
	
	public MComponentHistogram(String name) {
		super(name);
	}
	
	/**
	 * No use
	 */
	public void AddValue(String value) {}
	
	public void CleanUp() {}
	
	abstract public void AddValue(int x, int y, int bin, int windowIdx);

  abstract public void AddValue(int cnt, int bin, int windowIdx);
}
