/*
 * Created on Feb 18, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring;

/*
 * BasicDnD.java is a 1.4 example that requires
 * no other files.
 */

import java.awt.*;
import java.lang.reflect.Constructor;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.util.Vector;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.event.ChangeListener;

import atlas.monitoring.category.MCategory;
import atlas.monitoring.componentFrame.MComponentFrameDispatcher;
import atlas.monitoring.proxy.MProxy;
import atlas.monitoring.tabbedPane.MTabbedPane;

public class MEngineFrameMainPanel extends JPanel {
    
	private final String POPUP_MENU_ACTIVATE = "Activate";
	private final String POPUP_MENU_UNACTIVATE = "Unactivate";
	private final String POPUP_MENU_SHOW = "Show";
	private final String POPUP_MENUITEM_WAVEFORM = "Waveform viewer";
	private final String POPUP_MENUITEM_TEXT = "Text viewer";
	private final String POPUP_MENUITEM_METER = "Meter viewer";

	private final String RESOURCE_KEYWORD_CATEGORY = "MCategory";
	private final String CATEGORY_CLASS_PREFIX = "MCategory";
	private final String CATEGORY_PACKAGE_PREFIX = "atlas.monitoring.category";	
	//
	// Variable
	//
	private MProxy _proxy;
	private ResourceBundle _resources;
    //
	// Left
	//
	private JTabbedPane _tabbedPane;
	private MTabbedPane _monitoringTabbedPaneComponents[];
	
    //
    // Bottom
    //
    private Box _statusBox;
	private JTextField _statusTextField;				// Very bottom of the interface
	private JTextField _infoTextField;				// Inside the ststusbox
	//
	// Right
	//
	private JDesktopPane _desktop;
	private JMenu _libraryMenu;



    public MEngineFrameMainPanel(MProxy proxy) {
        super(new BorderLayout());
        
        this._proxy = proxy;
 
        _desktop = new JDesktopPane();
        
        JPanel leftPanel = createVerticalBoxPanel(5,0,0,0);
        JPanel rightPanel = createVerticalBoxPanel(5,5,5,5);
        
        //LEFT COLUMN
        _tabbedPane = createTabbedPanelUI();
        leftPanel.add(_tabbedPane);
        
        //RIGHT COLUMN
        _desktop.setPreferredSize(new Dimension(600,600));
        _desktop.setBackground(Color.LIGHT_GRAY);
        rightPanel.add(_desktop);
        
        //SPLIT PANE
        JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,leftPanel, rightPanel);
        splitPane.setOneTouchExpandable(true);
        add(splitPane, BorderLayout.CENTER);
        
        //STATUS BOX
        this._statusBox = createBox();
        add(this._statusBox, BorderLayout.PAGE_END);
        setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
        
        /**
         * TODO: fix here
         */
        int len = 0;
        
        if(_monitoringTabbedPaneComponents !=null)
        	len = _monitoringTabbedPaneComponents.length;
        
        MComponentFrameDispatcher[] dispatchers = new MComponentFrameDispatcher[len];
        
        for( int i=0; i<len; i++) {
        	dispatchers[i] = _monitoringTabbedPaneComponents[i].GetDispatcher();
        }
        
       this._proxy.StartListener(new MMainDispatcher(dispatchers));
    }

    public void CleanUp() {
    	
    	if(_monitoringTabbedPaneComponents!=null) {
    	   	
    		
    		for(int i=0; i<_monitoringTabbedPaneComponents.length; i++)
    	   		_monitoringTabbedPaneComponents[i].CleanUp();
    	}
    }
/**
 * From now on, use the tabbedPaneComponet to do the loading
 */
   
    /**
     * Load the parameter to current tabbedPane
     */
    public void LoadParameterToCategory() {
    	this.LoadParameterToCategory(false);
    }
    
    public void LoadParameterToCategory(boolean enforced) {
    	
    	boolean status = false;
    	
    	try {
	    	int tabbedIndex = _tabbedPane.getSelectedIndex();
	    	
	    	if(tabbedIndex<0)
	    		return;
	    	
	    	/**
	    	 * From now on, call the tabbedPaneComponent to do it.
	    	 */
	    	MTabbedPane com = _monitoringTabbedPaneComponents[tabbedIndex];
	    	
	    	status = com.LoadParameterToCategory(enforced);
    	}
    	catch(Exception ex) {
    		ex.printStackTrace();
    		status = false;
    	}
    	this._statusTextField.setText((status?"Ready":"Fail to download parameter.   Please check connection..."));
    }

    public void LoadParameterToAllCategories() {
    	boolean status = false;
    	
    	try {
    		for( int i=0; i<_monitoringTabbedPaneComponents.length; i++ ) {
		    	MTabbedPane com = _monitoringTabbedPaneComponents[i];
		    	status = com.LoadParameterToCategory(true);
    		}
    	}
    	catch(Exception ex) {
    		ex.printStackTrace();
    		status = false;
    	}
    }
    
    
    /**
     * Create the tabbedPanel UI
     * @return
     */
    protected JTabbedPane createTabbedPanelUI() {
    	JTabbedPane tabbedPane = new JTabbedPane();
        tabbedPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
        tabbedPane.setPreferredSize(new Dimension(180,100));
    	
        MCategory[] mc = createMCategory();
        
        //if no monitoring category found
        if(mc==null || mc.length==0)
        	return tabbedPane;
        
        /**
         * This is the monitoringCategories which store the tabbedPanel information
         * Need to modify
         */
    	int len = mc.length;
    	String name = null;
    	
    	/**
    	 * I may not need this one since everything is stored in the 
    	 * MonitoringTabbedPaneComponent. I can use this one to do the 
    	 * indexing job and get the required object
    	 */
    	//this._tabbedPaneComponents = new JComponent[len];
    	this._monitoringTabbedPaneComponents = new MTabbedPane[len];
    	
    	MTabbedPane tabbedPaneComponet = null;
    	
    	for( int i=0; i<len; i++) {
    		try {
    		
    			_monitoringTabbedPaneComponents[i] = createTabbedPaneComponent(mc[i]);
	    		
	    		/**
	    		 * So, what I have to do is to make the component here
	    		 */
	    		//this._tabbedPaneComponents[i] = createPaneForParameterList(this._tabbedPaneActivatedList[i],this._tabbedPaneUnactivatedList[i]);
	    		
	    		name = _monitoringTabbedPaneComponents[i].GetName();
	    		
	    		tabbedPane.addTab(name, null, _monitoringTabbedPaneComponents[i].GetTabbedComponentPanel(), name + " monitoring category");
    		}
    		catch(Exception e) {
    			e.printStackTrace();
    		}
    	}
        
    	tabbedPane.addChangeListener(
    			new ChangeListener(){
    				public void stateChanged(ChangeEvent e) {
    					LoadParameterToCategory();
    				}
    			}
   	
    	);
    	
        return tabbedPane;
    }
    
    /**
     * Creat the monitoring category array
     * @return
     */
    private MCategory[] createMCategory() {
		
		MCategory[] categories = null;
		
		Object[] objectParm = new Object[1];
		
		try {
			String[] categoryName = getResourceStrings(RESOURCE_KEYWORD_CATEGORY);
			
			if( categoryName == null || categoryName.length == 0) {
				categories = new MCategory[0];
			}
			else {
				
				Vector v = new Vector(categoryName.length);
				
				for( int i=0; i<categoryName.length; i++) {
					
					if(categoryName[i] == null || categoryName[i].length()<=0)
						continue;
					
					try {
							Class cl = Class.forName(this.CATEGORY_PACKAGE_PREFIX + "." + CATEGORY_CLASS_PREFIX + categoryName[i] );
							
							Constructor co = cl.getConstructor(new Class[] {Class.forName("java.lang.String")}); 
							
							objectParm[0] = categoryName[i];
							
							MCategory category = (MCategory)co.newInstance(objectParm);
							
							v.add(category);
					}
					catch( Exception ex ) {
						ex.printStackTrace();
					}
				}
				
				if( v.isEmpty()) {
					categories = new MCategory[0];
				}
				else {
					v.trimToSize();
					
					categories = new MCategory[v.size()];
					
					v.toArray(categories);
				}
			}
		}
		catch(Exception ex) {
			categories = new MCategory[0];
			ex.printStackTrace();
		}
		return categories;
	}
    
    /**
     * Get the resource bundle
     * @return Resource bundle regarding to corresponding property file
     */
	final protected ResourceBundle getResource() {
    	
    	if( _resources == null) {
	    	try
			{
	    		_resources = ResourceBundle.getBundle("atlas/resources/properties.MonitoringEngineFrame", Locale.getDefault());
			}
			catch (MissingResourceException mre){
				mre.printStackTrace();
			}
    	}
    	return _resources;
    }
	
	
	// Look For Keyword In Resource File
	final protected String[] getResourceStrings(String nm) {
		String[] strings = null;
		
		try {
			strings = getResource().getString(nm).split("\\s");
		}
		catch (Exception ex) {
			strings = new String[0];
		}
		
		return strings;
    }
    
    private MTabbedPane createTabbedPaneComponent(MCategory mc) {
 	
    	String cmd = "atlas.monitoring.tabbedPane.MTabbedPane" + mc.GetName();
    	MTabbedPane com;
    	
    	try {
	    	//System.out.print("creating " + cmd);
			Class cl = Class.forName(cmd);
			//System.out.print(" 1 ");
	    	Constructor con = cl.getDeclaredConstructor(new Class[] {MCategory.class, JDesktopPane.class, MProxy.class});
	    	//System.out.print(" 2 ");
	    	com = (MTabbedPane)con.newInstance(new Object[] {mc, _desktop, _proxy});
	    	//System.out.print(" 3 ");
		}
		catch(Exception ex) {
			//System.out.println("    Fail");
			ex.printStackTrace();
			return null;
		}

		return com;
    }
   
    protected Box createBox() {
    	Box myBox = new Box(BoxLayout.X_AXIS);
        this._statusTextField = new JTextField("Ready!", 80);
		this._infoTextField = new JTextField("", 15);

		this._statusTextField.setEnabled(false);
		this._infoTextField.setEnabled(false);
		myBox.add(this._statusTextField);
		myBox.add(this._infoTextField);
		
		return myBox;
    	
    }
    
    public void showMessage(String msg) {
    	_statusTextField.setText(msg);
    }
    
    protected JPanel createVerticalBoxPanel(int top, int left, int bottom, int right) {
        JPanel p = new JPanel();
        p.setLayout(new BoxLayout(p, BoxLayout.PAGE_AXIS));
        p.setBorder(BorderFactory.createEmptyBorder(top,left,bottom,right));
        return p;
    }
    
    //
    // organize internal frame
    //
    
    public void Cascad() {
    	if(_desktop!=null)
    		cascade(_desktop);
    }
    
    private static void cascade( JDesktopPane desktopPane, int layer ) {
        JInternalFrame[] frames = desktopPane.getAllFramesInLayer( layer );
        if ( frames.length == 0) return;
     
        cascade( frames, desktopPane.getBounds(), 24 );
    }
    private static void cascade( JDesktopPane desktopPane ) {
        JInternalFrame[] frames = desktopPane.getAllFrames();
        if ( frames.length == 0) return;

        cascade( frames, desktopPane.getBounds(), 24 );
    }
    private static void cascade( JInternalFrame[] frames, Rectangle dBounds, int separation ) {
        int margin = frames.length*separation + separation;
        int width = dBounds.width - margin;
        int height = dBounds.height - margin;
        for ( int i = 0; i < frames.length; i++) {
            frames[i].setBounds( separation + dBounds.x + i*separation,
                                 separation + dBounds.y + i*separation,
                                 width, height );
        }
    }
}
