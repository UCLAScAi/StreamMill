/*
 * Created on Mar 10, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.tabbedPane;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.lang.reflect.Constructor;

import javax.swing.*;

import atlas.monitoring.category.MCategory;
import atlas.monitoring.componentFrame.MComponentFrame;
import atlas.monitoring.library.MGlobal;
import atlas.monitoring.proxy.MProxy;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MTabbedPaneSystem extends MTabbedPaneSimpleLists {

	//
	// Popup Menu
	//
	private final String POPUP_MENU_SHOW = "Performance output";
	protected final String POPUP_MENU_ACTIVATE = "Activate";
	protected final String POPUP_MENU_UNACTIVATE = "Unactivate";
	protected final String POPUP_MENUITEM_WAVEFORM = "Waveform viewer";
	protected final String POPUP_MENUITEM_TEXT = "Text viewer";
	protected JMenu _activateMenu;
	protected JMenuItem _unactivateMenuItem;
	private JPopupMenu _popupMenu;
	private JMenuItem _textMenuItem;
	private JMenuItem _waveformMenuItem;
	
	/**
	 * Constructor
	 * @param mc {@link MCategory}
	 * @param desktop {@link JDesktopPane}
	 * @param proxy {@link MProxy}
	 */
	public MTabbedPaneSystem(MCategory mc, JDesktopPane desktop, MProxy proxy) {
		super(mc,desktop,proxy);
		
		// add the popup feature back to the simple list panel
		_popupMenu = createPopupMenu();
		_activatedList.addMouseListener( new SystemListMouseListener(POPUP_MENU_SHOW,true) );
		_unactivatedList.addMouseListener( new SystemListMouseListener(POPUP_MENU_ACTIVATE,false) );
	}
	

    
	//************************************************
	// Popup menu part
	//************************************************
	
	/**
	 * create the pop up menu
	 * @return
	 */
    private JPopupMenu createPopupMenu() {
    	JPopupMenu popupMenu = new JPopupMenu();
    	
    	_activateMenu = new JMenu(POPUP_MENU_ACTIVATE);
    	_unactivateMenuItem = new JMenuItem(POPUP_MENU_UNACTIVATE);
    	
        _textMenuItem = new JMenuItem(POPUP_MENUITEM_TEXT);
        _waveformMenuItem = new JMenuItem(POPUP_MENUITEM_WAVEFORM);
        
        _activateMenu.add(_textMenuItem);
        _activateMenu.add(_waveformMenuItem);
        
        popupMenu.add(this._activateMenu);
        popupMenu.addSeparator();
        popupMenu.add(this._unactivateMenuItem);
        
        _unactivateMenuItem.addActionListener( new PopuUnactiveActionListener() );
        _textMenuItem.addActionListener( new PopupActivateActionListener(MONITORING_COMPONENT_FRAME_TEXT, MGlobal.DATA_TYPE_IDX_SYSTEM_PERFORMANCE));
        _waveformMenuItem.addActionListener( new PopupActivateActionListener(MONITORING_COMPONENT_FRAME_WAVEFORM, MGlobal.DATA_TYPE_IDX_SYSTEM_PERFORMANCE));
       
        return popupMenu;
    }
    
    /**
     * create the performance pop up menu item for text and waveform
     * @param name
     * @param frameType
     * @param dataType
     * @param isShowFieldDialog
     * @return
     */
    private JMenuItem createPerformancePopupMenuItem(String name, String frameType, int dataType, boolean isShowFieldDialog) {
    	JMenuItem menuItem = new JMenuItem(name);
    	menuItem.addActionListener( new PopupActivateActionListener(frameType, dataType, isShowFieldDialog));
    	return menuItem;
    }
    
    /**
     * 
     * @author treetree
     *
     * unactivate the query
     */
    private class PopuUnactiveActionListener implements ActionListener{
		public void actionPerformed(ActionEvent arg0) {
	
			String parameter_id = (String)_activatedDefaultListModel.getElementAt(_activatedList.getSelectedIndex());
	    	
			_dispatcher.UnregisterFrameByParameterId(parameter_id);
			
			setMonitorElementMonitoringState(parameter_id,false);
			
			_proxy.RequestMonitoringAction(_monitoringCategory.GetStopMonitoringCommand(),parameter_id);
		}
	}    
    
    /**
	 * 
	 * @author treetree
	 *
	 * TODO To change the template for this generated type comment go to
	 * Window - Preferences - Java - Code Style - Code Templates
	 */
    private class PopupActivateActionListener implements ActionListener {
    	private String _comFrameName;
    	private int _dataType;
    	private boolean _isShowFieldDialog;
    	
    	/**
    	 * Constructor
    	 * @param componentFrameName
    	 */
    	public PopupActivateActionListener(String componentFrameName, int dataType) {
    		this._comFrameName = componentFrameName;
    		this._dataType = dataType;
    		this._isShowFieldDialog = true;
       	}
    	
    	/**
    	 * Constructor
    	 * @param componentFrameName
    	 * @param dataType
    	 * @param isShowFieldDialog
    	 */
    	public PopupActivateActionListener(String componentFrameName, int dataType, boolean isShowFieldDialog) {
    		this._comFrameName = componentFrameName;
    		this._dataType = dataType;
    		this._isShowFieldDialog = isShowFieldDialog;
       	}
    	
    	/**
    	 * Action performed
    	 */
    	public void actionPerformed(ActionEvent arg0) {
    		
    		Object[] arguments = null;
    		
	    	boolean needSendStartMonitoringMessage = false;
	    	
	    	MComponentFrame comFrame = null;
	    		    	
	    	JList list = null;
	    	String parameter_id = null;
	    	
	    	int selectedField = 1;
	    	
	    	//if this is an activate action
	    	if(_activateMenu.getText().equals(POPUP_MENU_ACTIVATE)) {
	    		list = _unactivatedList;
	    		parameter_id = (String)_unactivatedDefaultListModel.getElementAt(list.getSelectedIndex());
	    		setMonitorElementMonitoringState(parameter_id,true);
	    		needSendStartMonitoringMessage = true;
	    	}
	    	else if(_activateMenu.getText().equals(POPUP_MENU_SHOW)){
	    		list = _activatedList;
	    		parameter_id = (String)_activatedDefaultListModel.getElementAt(list.getSelectedIndex());
	    	}
	    	else {
	    		return;
	    	}
	    	
    		try {
    			Class cl = Class.forName(MONITORING_COMPONENT_FRAME_PACKAGE_PREFIX + "." + MONITORING_COMPONENT_FRAME_PREFIX + _comFrameName);
    			
    			Constructor co = cl.getConstructor(new Class[] {String.class, Integer.class, Object[].class }); 
    			
				Object objectParm[] = new Object[3];
				
				objectParm[0] = (String)parameter_id;
				objectParm[1] = new Integer(_dataType);
				objectParm[2] = arguments;
				
				comFrame = (MComponentFrame)co.newInstance(objectParm);
			}
			catch(Exception ex) {
				ex.printStackTrace();
				needSendStartMonitoringMessage = false;
				return;
			}
	    	
    		_desktop.add(comFrame);
    		comFrame.moveToFront();
    		
    		/**
    		 * Check this part, why 
    		 */
    		_dispatcher.RegisterFrame(comFrame);
	    	
    		String[] param = new String[1];
    		param[0] = MGlobal.GetDataTypeKey(_dataType);
    		
	    	//wait for everything ready first
	    	if(needSendStartMonitoringMessage) {
	    		_proxy.RequestMonitoringAction(_monitoringCategory.GetStartMonitoringCommand(),parameter_id,param);
	    	}
		}
    }
    
	//************************************************
	// Customized List listener
	//************************************************
    
	/**
	 * 
	 * @author treetree
	 *
	 * TODO To change the template for this generated type comment go to
	 * Window - Preferences - Java - Code Style - Code Templates
	 */
    protected class SystemListMouseListener extends SimpleListMouseListener implements MouseListener {

    	SystemListMouseListener(String name, boolean isEnable) {
    		super(name,isEnable);
    	}
		
		protected void customAction(MouseEvent e) {
			_activateMenu.setText(_name);
			_unactivateMenuItem.setEnabled(_isEnable);
			_popupMenu.show(e.getComponent(),e.getX(),e.getY());
		}
    }
}
