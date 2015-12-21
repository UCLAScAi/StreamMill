package atlas.monitoring;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.io.Serializable;

import javax.swing.*;
import javax.swing.BoxLayout;

import atlas.StreamMill;
import atlas.gui.MenuFrame;
import atlas.monitoring.category.MCategory;
import atlas.monitoring.library.MColorMgr;
import atlas.monitoring.proxy.*;


/*
 * Created on Feb 13, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MonitoringEngineFrame extends MenuFrame  {

	private StreamMill _parent;
    private MEngineFrameMainPanel _mainPanel;
    private MCategory[] _monitoringCategories;
    private MProxy _proxy;
    private final String PROXY_KEYWORD = "Proxy";
	
	public MonitoringEngineFrame(StreamMill parent, String loginName ) {
		super("StreamMill Monitoring Engine","MonitoringEngineFrame");
	    this._parent = parent;

	    this._proxy = createProxy(loginName,StreamMill.StreamServer,StreamMill.StreamServerPort);
        
	    //Create the main panel to contain the two sub panels.
        _mainPanel = new MEngineFrameMainPanel(_proxy);

        Container container = getContentPane();
        container.setLayout(new BoxLayout(container, BoxLayout.Y_AXIS));
        container.add(_mainPanel);
        setJMenuBar(createMenubar());
        container.add("North", createToolbar());
        container.add("Center", _mainPanel); //panel consists of editor and result
        
        pack();
        
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        screen.width -= getWidth();
        screen.height -= getHeight();
        setLocation((int)screen.width/2,(int)screen.height/2);
	    show();
	    
	    MColorMgr.GetInstance();	//pre-build the color
	    _mainPanel.LoadParameterToCategory(true);
	}
	
	private MProxy createProxy(String loginName, String server, int port) {
		try {
			String[] strings = getResourceStrings(PROXY_KEYWORD);
			
			//default
			if(strings == null || strings.length == 0)
				return new MProxyUDP(loginName,server,port);
			
			if(strings[0].equalsIgnoreCase("tcp"))
				return new MProxyTCP(loginName,server,port);
			
			return new MProxyUDP(loginName,server,port);
		}
		catch(Exception ex) {
			//default: proxy udp
			return new MProxyUDP(loginName,server,port);	
		}
	}
	
	
	//Window Event Listener
	public void windowClosing(WindowEvent e) {
		
		if(this._proxy!=null) {
			this._proxy.StopListening();
		}
		
		Thread bg = new Thread(new bgProcess());
		bg.start();
	}
	
	private class bgProcess implements Runnable {
		public bgProcess() {}
		
		public void run() {
			if(_mainPanel!=null) {
				_mainPanel.CleanUp();
			}
		}
	}
	
	//
	// Menu Action 
	//
	public class RefreshingAction extends AbstractAction implements Serializable {
	    
		public RefreshingAction() {
	      super("RefreshingAction");
	    }
		
		/**
		 * TODO: add it back
		 */
		public void actionPerformed(ActionEvent e) {
			_mainPanel.LoadParameterToCategory(true);
		}
	}
	
	public class CascadeAction extends AbstractAction implements Serializable {
	    
		public CascadeAction() {
	      super("CascadeAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			_mainPanel.Cascad();
		}
	}
}
