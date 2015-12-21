/*
 * Created on Feb 22, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.gui;

import java.awt.Component;
import java.awt.Insets;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.lang.reflect.Constructor;
import java.net.URL;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import javax.swing.Action;
import javax.swing.Box;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JToolBar;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MenuFrame extends JFrame implements WindowListener {

	private final String MENUBAR_SUFFIX = "Menubar";
	private final String TOOLBAR_SUFFIX = "Toolbar";
	private final String DEFAULT_STREAMMILL_ICON = "StreamMillIcon";
	private static final String RESOURCE_PATH = "atlas/resources/properties.";
	 
	// Define Suffix Used In Resource File
	protected static final String IMAGE_SUFFIX = "Image";
	protected static final String LABEDL_SUFFIX = "Label";
	protected static final String ACTION_SUFFIX = "Action";
	protected static final String TOOLTIP_SUFFIX = "Tooltip";
	
	private ResourceBundle _resources;
	private	String _resourceFile;
	
	/**
	 * 
	 * @param title Title of the frame
	 * @param resrcFile Resource file name
	 */
	public MenuFrame(String title, String resrcFile) {
		super(title);
		
		_resourceFile = resrcFile;
		
		setIcon();
		
        addWindowListener(this);
	}
	
	final private void setIcon() {
		URL imageSource = getResource(DEFAULT_STREAMMILL_ICON);
	
		if (imageSource != null){
			this.setIconImage((new ImageIcon(imageSource)).getImage());
		}
	}

	final protected JMenuBar createMenubar(){
		//not used...
		JMenuItem item;

		JMenuBar bar = new JMenuBar();

		String[] menuKeys = getResourceStrings(getClass().getName() + MENUBAR_SUFFIX);
		
		for (int i=0; i<menuKeys.length; i++){
			JMenu menu = createMenu(menuKeys[i]);
			if (menu != null){
				bar.add(menu);
			}
		}
		return bar;
	}
    
	final protected ResourceBundle getResource() {
    	
    	if( _resources == null) {
	    	try
			{
	    		//System.out.println(RESOURCE_PATH + _resourceFile);
	    		
				_resources = ResourceBundle.getBundle(RESOURCE_PATH + _resourceFile, Locale.getDefault());
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

		// Setup _menuItems

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
			//ex.printStackTrace();
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
	
	public void windowClosing(WindowEvent e) {}
	public void windowClosed(WindowEvent e) {}
	public void windowOpened(WindowEvent e) {}
	public void windowIconified(WindowEvent e) {}
	public void windowDeiconified(WindowEvent e) {}
	public void windowActivated(WindowEvent e) {}	
	public void windowDeactivated(WindowEvent e) {}
}
