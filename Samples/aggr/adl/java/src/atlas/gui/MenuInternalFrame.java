/*
 * Created on Feb 22, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.gui;

import java.awt.Component;
import java.awt.Insets;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.lang.reflect.Constructor;
import java.net.URL;
import java.util.Hashtable;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import javax.swing.Action;
import javax.swing.Box;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JInternalFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JToolBar;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;

import atlas.monitoring.component.MComponent;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MenuInternalFrame extends JInternalFrame implements InternalFrameListener{

	private final String MENUBAR_SUFFIX = "Menubar";
	private final String TOOLBAR_SUFFIX = "Toolbar";
	private final String DEFAULT_STREAMMILL_ICON = "StreamMillIcon";
	
	private ResourceBundle _resources;
	// Define Suffix Used In Resource File
	protected static final String IMAGE_SUFFIX = "Image";
	protected static final String LABEDL_SUFFIX = "Label";
	protected static final String ACTION_SUFFIX = "Action";
	protected static final String TOOLTIP_SUFFIX = "Tooltip";
	private Hashtable _menuItems;
	private String _frameName;
	
	public MenuInternalFrame(MComponent com, String frameName, String parameter_id) {
		super(parameter_id,true,true,true,true);
		
		_frameName = frameName;
		
		setJMenuBar(createMenubar());
		
		addInternalFrameListener(this);
	}

	final protected JMenuBar createMenubar(){
		//not used...
		JMenuItem item;

		JMenuBar bar = new JMenuBar();
		
		//System.out.println(getClass().getName() + MENUBAR_SUFFIX);
		
		String[] menuKeys = getResourceStrings(MENUBAR_SUFFIX);
		
		//System.out.println("Number of menuKeys = " + menuKeys.length);
		
		for (int i=0; i<menuKeys.length; i++){
			
			//System.out.println("menuKeys[i] = " + menuKeys[i]);
			
			JMenu menu = createMenu(menuKeys[i]);
			if (menu != null){
				bar.add(menu);
			}
			
			//System.out.println(menu==null?"null":"OK");
		}
		return bar;
	}
    
	final protected ResourceBundle getResource() {
    	
    	if( _resources == null) {
	    	try
			{
	    		_resources = ResourceBundle.getBundle("atlas/resources/properties." + _frameName, Locale.getDefault());
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
	
	// Create Menu From Resource File
	final protected JMenu createMenu(String key){

		String[] itemKeys = getResourceStrings(key);
		JMenu menu = new JMenu(getResourceString(key + LABEDL_SUFFIX));


		for (int i = 0; i < itemKeys.length; i++){
      
			if (itemKeys[i].equals("-")){
				menu.addSeparator();
			}
			else{
				JMenuItem item = createMenuItem(itemKeys[i]);
				menu.add(item);
			}
		}
		
		return menu;
	}
	
	// Look For Keyword In Resource File
	final protected String getResourceString(String nm) {
		String str;

		try {
			str = getResource().getString(nm);
		}
		catch (MissingResourceException mre) {
			str = null;
		}
		return str;
    	}
	
	// Create Menu Items From Source File
	final protected JMenuItem createMenuItem(String cmd){

		JMenuItem item = new JMenuItem(getResourceString(cmd + LABEDL_SUFFIX));
		URL imageSource = getResource(cmd + IMAGE_SUFFIX);

		if (imageSource != null){
			item.setHorizontalTextPosition(JButton.RIGHT);
			item.setIcon(new ImageIcon(imageSource));
		}
	
		String actionStr = getResourceString(cmd + ACTION_SUFFIX);

		if (actionStr  == null) {
		    actionStr  = cmd;
		}

		item.setActionCommand(actionStr);
		Action act = getAction(actionStr);			

		if (act != null) {
		    item.addActionListener(act);
		    act.addPropertyChangeListener(createActionChangeListener(item));
		    item.setEnabled(act.isEnabled());
		}
 		else {
		    item.setEnabled(false);
		}		

		return item;
    }
	
	final protected Action getAction(String cmd){
		cmd = getClass().getName() + "$" + cmd + ACTION_SUFFIX;

		Action act;
		
		try {
	    	//System.out.print("creating " + cmd);
			Class cl = Class.forName(cmd);
			//System.out.print(" 1 ");
	    	Constructor con = cl.getDeclaredConstructor(new Class[] {getClass()});
	    	//System.out.print(" 2 ");
	    	act = (Action)con.newInstance(new Object[] {this});
	    	//System.out.print(" 3 ");
		}
		catch(Exception ex) {
			//System.out.println("    Fail");
			ex.printStackTrace();
			return null;
		}
		
		//System.out.println("   OK");
		return act;
	}
	
	
	final protected PropertyChangeListener createActionChangeListener(JMenuItem b) {
		return new ActionChangedListener(b);
    }

    // Yanked from JMenu, ideally this would be public.
	final private class ActionChangedListener implements PropertyChangeListener {

        JMenuItem menuItem;
        
        ActionChangedListener(JMenuItem mi) {
            super();
            this.menuItem = mi;
        }

        final public void propertyChange(PropertyChangeEvent e) {

			String propertyName = e.getPropertyName();

            if (e.getPropertyName().equals(Action.NAME)) {
                String text = (String) e.getNewValue();
                menuItem.setText(text);
            } else if (propertyName.equals("enabled")) {
                Boolean enabledState = (Boolean) e.getNewValue();
                menuItem.setEnabled(enabledState.booleanValue());
            }
        }
	}

	
//	 Get Image Source File From Resource File
	final protected URL getResource(String key) {
		String name = getResourceString(key);
		
		if (name != null) {
			URL imageSource = this.getClass().getResource(name);
			
			return imageSource;
		}
			return null;
	}
	
	//////////////////////////////////////////////////////////////
	//							    //
	//                  ToolBar IMPLEMENTATION //
	//							    //
	//////////////////////////////////////////////////////////////

	final protected Component createToolbar() {

		JToolBar toolbar = new JToolBar(JToolBar.HORIZONTAL);
		
		toolbar.setFloatable(false);
		
		String[] toolKeys = tokenize(getResourceString(getClass().getName() + TOOLBAR_SUFFIX));

		for (int i = 0; i < toolKeys.length; i++) {
			
			if (toolKeys[i].equals("-")) {
				toolbar.add(Box.createHorizontalStrut(20));
			} else {
				JButton button = createToolbarButton(toolKeys[i]); 
				if(button!=null)
					toolbar.add(button);
			}
		}

		toolbar.add(Box.createHorizontalGlue());
		return toolbar;
	}

	final protected String[] tokenize(String string) {
		if(string==null)
			return new String[0];
		
		return string.split("\\s");
	}

	private JButton createToolbarButton(String key) {

		URL imageURL = getResource(key + IMAGE_SUFFIX);

		if(imageURL==null)
			return null;
		
		JButton button = new JButton(new ImageIcon(imageURL)) {
			public float getAlignmentY() {
				return 0.5f;
			}
		};

		button.setRequestFocusEnabled(false);
		button.setMargin(new Insets(1, 1, 1, 1));

		String actionStr = getResourceString(key + ACTION_SUFFIX);

		if (actionStr == null) {
			actionStr = key;
		}

		Action act = getAction(actionStr);

		if (act != null) {
			button.setActionCommand(actionStr);
			button.addActionListener(act);
		} else {
			button.setEnabled(false);
		}

		String tip = getResourceString(key + TOOLTIP_SUFFIX);

		if (tip != null) {
			button.setToolTipText(tip);
		}

		return button;
	}
	
	//**************
	
    public void internalFrameClosing(InternalFrameEvent e) {}
    public void internalFrameClosed(InternalFrameEvent e) {}
    public void internalFrameOpened(InternalFrameEvent e) {}
    public void internalFrameIconified(InternalFrameEvent e) {}
    public void internalFrameDeiconified(InternalFrameEvent e) {}
    public void internalFrameActivated(InternalFrameEvent e) {}
    public void internalFrameDeactivated(InternalFrameEvent e) {}
}
