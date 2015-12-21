/*
 * Created on Mar 5, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.tabbedPane;

import javax.swing.JComponent;
import javax.swing.JDesktopPane;

import atlas.monitoring.category.MCategory;
import atlas.monitoring.componentFrame.MComponentFrameDispatcher;
import atlas.monitoring.proxy.MProxy;


/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MTabbedPane extends JComponent {
	
	protected MComponentFrameDispatcher _dispatcher;
	protected MCategory _monitoringCategory;
	protected JDesktopPane _desktop;
	protected MProxy _proxy;
	protected JComponent _comPanel;
	
	public MTabbedPane(MCategory mc, JDesktopPane desktop, MProxy proxy) {
		_monitoringCategory = mc;
		_desktop = desktop;
		_proxy = proxy;
		_dispatcher = new MComponentFrameDispatcher();
	}

	/**
	 * Display name for this monitoring tabbed Pane Component
	 * @return Name
	 */
	final public String GetName() {
		return _monitoringCategory.GetName();
	}
	
	final public MComponentFrameDispatcher GetDispatcher() {
		return _dispatcher;
	}
	
	final public void CleanUp() {
		
		CustomizedCleanUp();
		
		if(_dispatcher!=null)
			_dispatcher.CleanUp();
	}
	
	final public JComponent GetTabbedComponentPanel() {
		return _comPanel;
	}
	
	protected void CustomizedCleanUp() {};
	

		
	//
	// Abstract method
	//
	abstract public boolean LoadParameterToCategory(boolean enforced);
}
