/*
 * Created on Mar 5, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.tabbedPane;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Toolkit;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.File;
import java.lang.reflect.*;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDesktopPane;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFormattedTextField;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JTextField;

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
public class MTabbedPaneQuery extends MTabbedPaneSimpleLists {

	private final String POPUP_MENU_ACTIVATE = "Activate";
	private final String POPUP_MENU_UNACTIVATE = "Unactivate";
	private final String POPUP_MENU_SHOW = "Data output";
	private final String POPUP_MENUITEM_WAVEFORM = "Waveform viewer";
	private final String POPUP_MENUITEM_TEXT = "Text viewer";
	private final String POPUP_MENUITEM_CLUSTER = "Cluster viewer";
	private final String POPUP_MENUITEM_HISTOGRAM_HORIZONAL = "Histogram (Horizontal)";
	private final String POPUP_MENUITEM_HISTOGRAM_VERTICAL = "Histogram (Vertical)";
	//
	// Popup Menu
	//
	private JPopupMenu _popupMenu;
	private JMenu _activateMenu;
	private JMenu _performanceMenu;
	private JMenu _bufferSizeMenu;
	private JMenu _delayMenu;
	private JMenu _processingTimeMenu;
	
	private JMenuItem _unactivateMenuItem;
	private JMenuItem _textMenuItem;
	private JMenuItem _waveformMenuItem;
	private JMenuItem _clusterMenuItem;
	private JMenuItem _histogramVerticalMenuItem;
	private JMenuItem _histogramHorizontalMenuItem;
    
    
	/**
	 * Constructor
	 * @param mc {@link MCategory}
	 * @param desktop {@link JDesktopPane}
	 * @param proxy {@link MProxy}
	 */
	public MTabbedPaneQuery(MCategory mc, JDesktopPane desktop, MProxy proxy) {
		super(mc,desktop,proxy);
		
		_popupMenu = createPopupMenu();
		_activatedList.addMouseListener( new QueryListMouseListener(POPUP_MENU_SHOW,true) );
		_unactivatedList.addMouseListener( new QueryListMouseListener(POPUP_MENU_ACTIVATE,false) );

	}
	
	//************************************************
	// public class
	//************************************************
    
	/**
	 * 
	 * @author treetree
	 *
	 * TODO To change the template for this generated type comment go to
	 * Window - Preferences - Java - Code Style - Code Templates
	 */
    private class QueryListMouseListener extends SimpleListMouseListener implements MouseListener {
		
		QueryListMouseListener(String name, boolean isEnable) {
    		super(name,isEnable);
    	}
		
		protected void customAction(MouseEvent e) {
			_activateMenu.setText(_name);
			_performanceMenu.setEnabled(_isEnable);
			_unactivateMenuItem.setEnabled(_isEnable);
			_popupMenu.show(e.getComponent(),e.getX(),e.getY());
		}
    }
    
	/**
	 * create the pop up menu
	 * @return
	 */
    private JPopupMenu createPopupMenu() {
    	JPopupMenu popupMenu = new JPopupMenu();
    	
    	_performanceMenu = new JMenu("Performance");
    	_activateMenu = new JMenu(POPUP_MENU_ACTIVATE);
    	_unactivateMenuItem = new JMenuItem(POPUP_MENU_UNACTIVATE);
    	
    	
    	_bufferSizeMenu = new JMenu("Buffer size");
    	_delayMenu = new JMenu("Delay");
    	_processingTimeMenu = new JMenu("Processing time");
    	
        _textMenuItem = new JMenuItem(POPUP_MENUITEM_TEXT);
        _waveformMenuItem = new JMenuItem(POPUP_MENUITEM_WAVEFORM);
        _clusterMenuItem = new JMenuItem(POPUP_MENUITEM_CLUSTER);
        _histogramHorizontalMenuItem = new JMenuItem(POPUP_MENUITEM_HISTOGRAM_HORIZONAL);
        _histogramVerticalMenuItem = new JMenuItem(POPUP_MENUITEM_HISTOGRAM_VERTICAL);
        
        
        _activateMenu.add(_textMenuItem);
        _activateMenu.add(_waveformMenuItem);
        _activateMenu.add(_clusterMenuItem);
        _activateMenu.add(_histogramHorizontalMenuItem);
        _activateMenu.add(_histogramVerticalMenuItem);
        
        _bufferSizeMenu.add(createPerformancePopupMenuItem(POPUP_MENUITEM_TEXT, MONITORING_COMPONENT_FRAME_TEXT, MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_BUFFER_SIZE, false));
        _bufferSizeMenu.add(createPerformancePopupMenuItem(POPUP_MENUITEM_WAVEFORM, MONITORING_COMPONENT_FRAME_WAVEFORM, MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_BUFFER_SIZE, false));
        _processingTimeMenu.add(createPerformancePopupMenuItem(POPUP_MENUITEM_TEXT, MONITORING_COMPONENT_FRAME_TEXT, MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_PRO_TIME, false));
        _processingTimeMenu.add(createPerformancePopupMenuItem(POPUP_MENUITEM_WAVEFORM, MONITORING_COMPONENT_FRAME_WAVEFORM, MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_PRO_TIME, false));
        _delayMenu.add(createPerformancePopupMenuItem(POPUP_MENUITEM_TEXT, MONITORING_COMPONENT_FRAME_TEXT, MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_DELAY, false));
        _delayMenu.add(createPerformancePopupMenuItem(POPUP_MENUITEM_WAVEFORM, MONITORING_COMPONENT_FRAME_WAVEFORM, MGlobal.DATA_TYPE_IDX_QUERY_PERFORMANCE_DELAY, false));
        
        _performanceMenu.add(_bufferSizeMenu);
        _performanceMenu.add(_delayMenu);
        _performanceMenu.add(_processingTimeMenu);
        
        popupMenu.add(this._activateMenu);
        popupMenu.add(this._performanceMenu);
        popupMenu.addSeparator();
        popupMenu.add(this._unactivateMenuItem);
        
        _unactivateMenuItem.addActionListener( new PopuUnactiveActionListener() );
        _textMenuItem.addActionListener( new PopupActivateActionListener(MONITORING_COMPONENT_FRAME_TEXT, MGlobal.DATA_TYPE_IDX_QUERY_OUTPUT));
        _waveformMenuItem.addActionListener( new PopupActivateActionListener(MONITORING_COMPONENT_FRAME_WAVEFORM, MGlobal.DATA_TYPE_IDX_QUERY_OUTPUT));
        _clusterMenuItem.addActionListener( new PopupActivateActionListener(MONITORING_COMPONENT_FRAME_CLUSTER, MGlobal.DATA_TYPE_IDX_QUERY_OUTPUT));
        _histogramHorizontalMenuItem.addActionListener( new PopupActivateActionListener(MONITORING_COMPONENT_FRAME_HORIZONTAL_HISTOGRAM, MGlobal.DATA_TYPE_IDX_QUERY_OUTPUT));
        _histogramVerticalMenuItem.addActionListener( new PopupActivateActionListener(MONITORING_COMPONENT_FRAME_VERTICAL_HISTOGRAM, MGlobal.DATA_TYPE_IDX_QUERY_OUTPUT));
        
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
	    	
	    	// Display the dialog and ask for which field to display
	    	if(_comFrameName.equals("Waveform") && _isShowFieldDialog) {
	    		selectedField = ShowGetFieldDialog();
	    		
	    		if(selectedField==0)
	    			return;
	    		
	    		arguments = new Object[1];
	    		arguments[0] = new Integer(selectedField);
	    	}
	    	else if(_comFrameName.equals("Cluster") && _isShowFieldDialog) {
	    		
	    		ClusterPereferenceDialog dialog = new ClusterPereferenceDialog();
	    		
	    		Object[] ret = dialog.ShowDialog();
	    		
	    		if(ret==null)
	    			return;
	    		
	    		arguments = new Object[4];
	    		
	    		arguments[0] = ret[0];
	    		arguments[1] = ret[1];
	    		arguments[2] = ret[2];
	    		
	    		if(ret[3]!=null)
	    			arguments[3] = ret[3];
	    		
	    	}
//	    	else if(_comFrameName.equals(MONITORING_COMPONENT_FRAME_VERTICAL_HISTOGRAM) && _isShowFieldDialog) {
//	    		
//	    		HistogramPereferenceDialog dialog = new HistogramPereferenceDialog();
//	    		
//	    		Object[] ret = dialog.ShowDialog();
//	    		
//	    		if(ret==null)
//	    			return;
//	    		
//	    		arguments = new Object[1];
//	    		
//	    		//auto capture path
//	    		if(ret[0]!=null)
//	    			arguments[0] = ret[0];
//	    		
//	    	}
	    	
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
    		
    		if(comFrame.GetDataType() != MGlobal.DATA_TYPE_IDX_QUERY_OUTPUT) {
    			needSendStartMonitoringMessage = !_dispatcher.IsFrameExist(comFrame);
    		}
    		
    		/**
    		 * Check this part, why 
    		 */
    		_dispatcher.RegisterFrame(comFrame);
	    	
    		String[] param = new String[1];
    		param[0] = MGlobal.GetDataTypeKey(_dataType);
    		
	    	//wait for everything ready first
	    	if(needSendStartMonitoringMessage)  {
	    		_proxy.RequestMonitoringAction(_monitoringCategory.GetStartMonitoringCommand(),parameter_id,param);
	    	}
		}
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
    * Show the dialog to select the monitoring field from the output
    * @return
    */
   public int ShowGetFieldDialog() {
		 Object[] possibleValues = { "1", "2", "3", "4", "5", "6","7", "8", "9", "10" };
	        Object selectedValue = JOptionPane.showInputDialog(
	        		this, "Choose field for monitoring", "Input",
	        		JOptionPane.QUESTION_MESSAGE, null,
					possibleValues, possibleValues[0]);
	        
	        if(selectedValue==null)
	        	return 0;
	        else
	            return Integer.parseInt((String)selectedValue);		
	}
   
   /**
    * 
    * @author treetree
    *
    * TODO To change the template for this generated type comment go to
    * Window - Preferences - Java - Code Style - Code Templates
    */
   private class ClusterPereferenceDialog extends JDialog {
   	
   		private final int MAX_BUFFER_SIZE = 10000;
   		private final int MIN_BUFFER_SIZE = 10;
   		private final int MIN_RADIUS = 0;
   		private JFormattedTextField _textField_bufferSize;
   		private JFormattedTextField _textField_radius;
   		private JTextField _textfield_path;
   		private JTextField _textfield_prefix;
   		private JCheckBox _checkCluster;
   		private JComboBox _comboSpeed;
   		
   		private Object[] _ret;
   		private boolean _ok = false;;
   	
   		public ClusterPereferenceDialog() {	
   			
   			setTitle("Cluster setting");
   			
   			JPanel topPanel = new JPanel();
   			JPanel mainPanel = new JPanel();
   			topPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
   			mainPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
   			mainPanel.setLayout(new BorderLayout());
   			topPanel.add(makeScalePanel());
   			topPanel.add(makeAutoCapturePanel());
   			
   			mainPanel.add(topPanel, BorderLayout.CENTER);
	        mainPanel.add(makeButtonPanel(), BorderLayout.SOUTH);
   			
   			getContentPane().add(mainPanel, BorderLayout.CENTER);
   			
   			setSize(380,360);
	        setResizable(false);
			setModal(true);
			
			Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
	        screen.width -= getWidth();
	        screen.height -= getHeight();
	        setLocation((int)screen.width/2,(int)screen.height/2);
   		}
   		
   		protected JPanel makeAutoCapturePanel() {
  
   			int textWidth = 225;
   			int textHeight = 25;
   			int labelWidth = 240;
   			int buttonWidth = 80;
   			
   			JPanel panelMain  = new JPanel(new GridLayout(3,1,5,5));
   			panelMain.setBorder(BorderFactory.createCompoundBorder(
		            BorderFactory.createTitledBorder("Auto Capture Setting (optional)"), 
		            BorderFactory.createEmptyBorder(8,8,8,8)));
   			
   			JPanel panelCheckBoxCluster = new JPanel(new BorderLayout());
   			JPanel panelPathChooser = new JPanel(new BorderLayout());
   			JPanel panelPrefix = new JPanel(new BorderLayout());
   			JLabel labelPrefix = new JLabel("File prefix");
   			JButton buttonPath = new JButton("Browse");
   			
   			
   			panelPathChooser.setBorder(BorderFactory.createCompoundBorder(
		            null,BorderFactory.createEmptyBorder(2,2,2,2)));
   			panelPrefix.setBorder(BorderFactory.createCompoundBorder(
		            null,BorderFactory.createEmptyBorder(2,2,2,2)));
   			
   			
   			_textfield_path = new JTextField();
   			_textfield_path.setEditable(false);
   			_textfield_prefix = new JTextField("clu_");
   			
   			_checkCluster = new JCheckBox("Activated automatic screen capture");
 
   			_checkCluster.setPreferredSize(new Dimension(labelWidth, textHeight));
 			buttonPath.setPreferredSize(new Dimension(buttonWidth, textHeight));
   			labelPrefix.setPreferredSize(new Dimension(buttonWidth, textHeight));
   			_textfield_path.setPreferredSize(new Dimension(textWidth, textHeight));
   			_textfield_prefix.setPreferredSize(new Dimension(textWidth, textHeight));
   			
   			buttonPath.addActionListener(
   				new ActionListener() {
   					public void actionPerformed(ActionEvent ev){
   						JFrame frame = new JFrame();
   						JFileChooser filechooser = new JFileChooser();
   						filechooser.setDialogTitle("Choose directory");
   						filechooser.setDialogType(JFileChooser.DIRECTORIES_ONLY);
   						filechooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);

 
   				        if(filechooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION)
   				        {
   				         _textfield_path.setText(filechooser.getSelectedFile().toString());
   				        }

   					    
   					   
   					}
   				}
   			);
			
   			panelCheckBoxCluster.add(_checkCluster,BorderLayout.WEST);
   			panelPathChooser.add(buttonPath,BorderLayout.WEST);
   			panelPathChooser.add(_textfield_path,BorderLayout.CENTER);
   			panelPrefix.add(labelPrefix,BorderLayout.WEST);
   			panelPrefix.add(_textfield_prefix,BorderLayout.CENTER);
   			
   			panelMain.add(panelCheckBoxCluster);
   			panelMain.add(panelPathChooser);
   			panelMain.add(panelPrefix);
   			
   			return panelMain;
   		}

   		protected JPanel makeScalePanel() {

   			int textWidth = 30;
   			int textHeight = 25;
   			int labelWidth = 230;
   			
   			String[] speed = {"Slow", "Average", "Fast", "Very fast" };
   			
   			JPanel panelMain  = new JPanel(new GridLayout(3,1,5,5));
   			panelMain.setBorder(BorderFactory.createCompoundBorder(
		            BorderFactory.createTitledBorder("Cluster Setting"), 
		            BorderFactory.createEmptyBorder(8,8,8,8)));
   			
   			JPanel panelSpeed = new JPanel(new BorderLayout()); 
   			JPanel panelBufferSize = new JPanel(new BorderLayout());
   			JPanel panelRadius = new JPanel(new BorderLayout());
   			JLabel labelBufferSize = new JLabel("Window size (" + MIN_BUFFER_SIZE + " - " + MAX_BUFFER_SIZE +") (Required)");
   			JLabel labelradius = new JLabel("Radius >= " + MIN_RADIUS +" (optional)");
   			JLabel labelSpeed = new JLabel("Fading speed (optional)");
   			
   			_comboSpeed = new JComboBox(speed);
   			_comboSpeed.setSelectedIndex(1);
   			_textField_bufferSize = new JFormattedTextField();
   			_textField_radius = new JFormattedTextField();
   			
   			_textField_bufferSize.setValue(new Integer(100));
   			_textField_bufferSize.setColumns(10);
   			
   			_textField_radius.setValue(new Integer(0));
   			_textField_radius.setColumns(10);
   			
   			labelBufferSize.setPreferredSize(new Dimension(labelWidth,textHeight));
   			labelradius.setPreferredSize(new Dimension(labelWidth,textHeight));
   			labelSpeed.setPreferredSize(new Dimension(labelWidth,textHeight));
   			_textField_bufferSize.setPreferredSize(new Dimension(textWidth,textHeight));
   			_textField_radius.setPreferredSize(new Dimension(textWidth,textHeight));
   			_comboSpeed.setPreferredSize(new Dimension(textWidth,textHeight));
   			
   			panelBufferSize.add(labelBufferSize,BorderLayout.WEST);
   			panelRadius.add(labelradius,BorderLayout.WEST);
   			panelSpeed.add(labelSpeed,BorderLayout.WEST);
   			
   			panelBufferSize.add(_textField_bufferSize,BorderLayout.CENTER);
   			panelRadius.add(_textField_radius,BorderLayout.CENTER);
   			panelSpeed.add(_comboSpeed,BorderLayout.CENTER);
   			
   			
   			panelMain.add(panelBufferSize);
   			panelMain.add(panelRadius);
   			panelMain.add(panelSpeed);
   			
   			return panelMain;
   		}
   		
		protected JPanel makeButtonPanel() {

			JPanel mainPanel = new JPanel(new BorderLayout() );
			JPanel buttonPanel = new JPanel();
			JButton okButton = new JButton("OK");
			JButton cancelButton = new JButton("Cancel");
			
			buttonPanel.setLayout(new GridLayout(1,2,5,5));
			buttonPanel.add(okButton);
			buttonPanel.add(cancelButton);
			
			mainPanel.add(buttonPanel,BorderLayout.EAST);
			
			cancelButton.addActionListener(
				new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						setVisible(false);
					}
				}
			);
			
			okButton.addActionListener(
					new ActionListener() {
						public void actionPerformed(ActionEvent e) {
							
							StringBuffer buf = new StringBuffer();
							
							int tmp0 = ((Number)_textField_bufferSize.getValue()).intValue();
							int tmp1 = ((Number)_textField_radius.getValue()).intValue();
							
							if(tmp0 < MIN_BUFFER_SIZE || MAX_BUFFER_SIZE < tmp0 ) {
								buf.append("Buffer size");
							}
							
							if(tmp1 < MIN_RADIUS) {
								
								if(buf.capacity()>0)
									buf.append(" and radius");
								else
									buf.append("Radius");
							}
							
							if(buf.toString().length()>0) {
								buf.append( " out of range !\n");
							}
							
							if(_checkCluster.isSelected() ) {
								
								if(_textfield_path.getText().length()<=0) {
									buf.append( "Missing directory!\n" );
								}
								
								if(_textfield_prefix.getText().length()<=0) {
									buf.append( "Missig prefix!\n");
								}
							}
							
							if(buf.toString().length()>0) {
								JOptionPane.showMessageDialog(null, buf.toString(), "Warning", JOptionPane.WARNING_MESSAGE);
							}
							else {
								
								// 0 - buffer size
								// 1 - radius
								// 2 - fading speed
								// 3 - cluster auto capture
								// 4 - histogram auto capture
								
								_ret = new Object[5];
								_ret[0] = new Integer(tmp0);
								_ret[1] = new Integer(tmp1);
								_ret[2] = new Integer(_comboSpeed.getSelectedIndex());
								
								if(_checkCluster.isSelected())
									_ret[3] = _textfield_path.getText() + File.separator + _textfield_prefix.getText();
								
								_ok = true;
								setVisible(false);
							}
						}
					}
				);
			
			
			return mainPanel;
		}
		
   		
   		public Object[] ShowDialog() {
   			show();
   			
   			if(_ok)
   				return _ret;
   			else
   				return null;
   		}
   }

   
   
   
   
   
   
//   public int ShowGetClusterBufferSizeDialog() {
//   	
//   		int num = -1;
//   	
//   		try {
//   	
//   			String input = JOptionPane.showInputDialog("Enter the data size (10 - 10000)");
//   			
//   			num = Integer.parseInt(input);
//   			
//   			if(num < 10 || 10000 < num) {
//   				JOptionPane.showMessageDialog(null, "Number out of range", "Warning", JOptionPane.WARNING_MESSAGE);
//   	   			return -1;
//   			}
//   			return num;
//   		}
//   		catch(NumberFormatException ex) {
//   			JOptionPane.showMessageDialog(null, "Invalid number format", "Warning", JOptionPane.WARNING_MESSAGE);
//   			return -1;
//   		}
//   }
   
//   private class ClusterHistogramPereferenceDialog extends JDialog {
//   	
//   		private final int MAX_BUFFER_SIZE = 10000;
//   		private final int MIN_BUFFER_SIZE = 10;
//   		private final int MIN_RADIUS = 0;
//   		private JFormattedTextField _textField_bufferSize;
//   		private JFormattedTextField _textField_radius;
//   		private JTextField _textfield_path;
//   		private JTextField _textfield_prefix;
//   		private JCheckBox _checkCluster;
//   		private JCheckBox _checkHistogram;
//   		private JComboBox _comboSpeed;
//   		
//   		private Object[] _ret;
//   		private boolean _ok = false;;
//   	
//   		public ClusterHistogramPereferenceDialog() {	
//   			
//   			setTitle("Cluster setting");
//   			
//   			JPanel topPanel = new JPanel();
//   			JPanel mainPanel = new JPanel();
//   			topPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
//   			mainPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
//   			mainPanel.setLayout(new BorderLayout());
//   			topPanel.add(makeScalePanel());
//   			topPanel.add(makeAutoCapturePanel());
//   			
//   			mainPanel.add(topPanel, BorderLayout.CENTER);
//	        mainPanel.add(makeButtonPanel(), BorderLayout.SOUTH);
//   			
//   			getContentPane().add(mainPanel, BorderLayout.CENTER);
//   			
//   			setSize(380,400);
//	        setResizable(false);
//			setModal(true);
//			
//			Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
//	        screen.width -= getWidth();
//	        screen.height -= getHeight();
//	        setLocation((int)screen.width/2,(int)screen.height/2);
//   		}
//   		
//   		protected JPanel makeAutoCapturePanel() {
//  
//   			int textWidth = 225;
//   			int textHeight = 25;
//   			int labelWidth = 120;
//   			int buttonWidth = 80;
//   			
//   			JPanel panelMain  = new JPanel(new GridLayout(4,1,5,5));
//   			panelMain.setBorder(BorderFactory.createCompoundBorder(
//		            BorderFactory.createTitledBorder("Auto Capture Setting"), 
//		            BorderFactory.createEmptyBorder(8,8,8,8)));
//   			
//   			JPanel panelCheckBoxCluster = new JPanel(new BorderLayout());
//   			JPanel panelCheckBoxHistogram = new JPanel(new BorderLayout());
//   			JPanel panelPathChooser = new JPanel(new BorderLayout());
//   			JPanel panelPrefix = new JPanel(new BorderLayout());
//   			JLabel labelPrefix = new JLabel("File prefix");
//   			JButton buttonPath = new JButton("Browse");
//   			
//   			
//   			panelPathChooser.setBorder(BorderFactory.createCompoundBorder(
//		            null,BorderFactory.createEmptyBorder(2,2,2,2)));
//   			panelPrefix.setBorder(BorderFactory.createCompoundBorder(
//		            null,BorderFactory.createEmptyBorder(2,2,2,2)));
//   			
//   			
//   			_textfield_path = new JTextField();
//   			_textfield_path.setEditable(false);
//   			_textfield_prefix = new JTextField("img_");
//   			
//   			_checkCluster = new JCheckBox("Cluster");
//   			_checkHistogram = new JCheckBox("Histogram");
// 
//   			_checkCluster.setPreferredSize(new Dimension(labelWidth, textHeight));
//   			_checkHistogram.setPreferredSize(new Dimension(labelWidth, textHeight));
// 			buttonPath.setPreferredSize(new Dimension(buttonWidth, textHeight));
//   			labelPrefix.setPreferredSize(new Dimension(buttonWidth, textHeight));
//   			_textfield_path.setPreferredSize(new Dimension(textWidth, textHeight));
//   			_textfield_prefix.setPreferredSize(new Dimension(textWidth, textHeight));
//   			
//   			buttonPath.addActionListener(
//   				new ActionListener() {
//   					public void actionPerformed(ActionEvent ev){
//   						JFrame frame = new JFrame();
//   						JFileChooser filechooser = new JFileChooser();
//   						filechooser.setDialogTitle("Choose directory");
//   						filechooser.setDialogType(JFileChooser.DIRECTORIES_ONLY);
//   						filechooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
//
// 
//   				        if(filechooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION)
//   				        {
//   				         _textfield_path.setText(filechooser.getSelectedFile().toString());
//   				        }
//
//   					    
//   					   
//   					}
//   				}
//   			);
//			
//   			panelCheckBoxCluster.add(_checkCluster,BorderLayout.WEST);
//   			panelCheckBoxHistogram.add(_checkHistogram,BorderLayout.WEST);
//   			panelPathChooser.add(buttonPath,BorderLayout.WEST);
//   			panelPathChooser.add(_textfield_path,BorderLayout.CENTER);
//   			panelPrefix.add(labelPrefix,BorderLayout.WEST);
//   			panelPrefix.add(_textfield_prefix,BorderLayout.CENTER);
//   			
//   			panelMain.add(panelCheckBoxCluster);
//   			panelMain.add(panelCheckBoxHistogram);
//   			panelMain.add(panelPathChooser);
//   			panelMain.add(panelPrefix);
//   			
//   			return panelMain;
//   		}
//
//   		protected JPanel makeScalePanel() {
//
//   			int textWidth = 60;
//   			int textHeight = 25;
//   			int labelWidth = 200;
//   			
//   			String[] speed = {"Slow", "Average", "Fast", "Very fast" };
//   			
//   			JPanel panelMain  = new JPanel(new GridLayout(3,1,5,5));
//   			panelMain.setBorder(BorderFactory.createCompoundBorder(
//		            BorderFactory.createTitledBorder("Cluster Setting"), 
//		            BorderFactory.createEmptyBorder(8,8,8,8)));
//   			
//   			JPanel panelSpeed = new JPanel(new BorderLayout()); 
//   			JPanel panelBufferSize = new JPanel(new BorderLayout());
//   			JPanel panelRadius = new JPanel(new BorderLayout());
//   			JLabel labelBufferSize = new JLabel("Data size (" + MIN_BUFFER_SIZE + " - " + MAX_BUFFER_SIZE +")");
//   			JLabel labelradius = new JLabel("Radius ( >= " + MIN_RADIUS +" )");
//   			JLabel labelSpeed = new JLabel("Fading speed");
//   			
//   			_comboSpeed = new JComboBox(speed);
//   			_comboSpeed.setSelectedIndex(1);
//   			_textField_bufferSize = new JFormattedTextField();
//   			_textField_radius = new JFormattedTextField();
//   			
//   			_textField_bufferSize.setValue(new Integer(100));
//   			_textField_bufferSize.setColumns(10);
//   			
//   			_textField_radius.setValue(new Integer(0));
//   			_textField_radius.setColumns(10);
//   			
//   			labelBufferSize.setPreferredSize(new Dimension(labelWidth,textHeight));
//   			labelradius.setPreferredSize(new Dimension(labelWidth,textHeight));
//   			labelSpeed.setPreferredSize(new Dimension(labelWidth,textHeight));
//   			_textField_bufferSize.setPreferredSize(new Dimension(textWidth,textHeight));
//   			_textField_radius.setPreferredSize(new Dimension(textWidth,textHeight));
//   			_comboSpeed.setPreferredSize(new Dimension(textWidth,textHeight));
//   			
//   			panelBufferSize.add(labelBufferSize,BorderLayout.WEST);
//   			panelRadius.add(labelradius,BorderLayout.WEST);
//   			panelSpeed.add(labelSpeed,BorderLayout.WEST);
//   			
//   			panelBufferSize.add(_textField_bufferSize,BorderLayout.CENTER);
//   			panelRadius.add(_textField_radius,BorderLayout.CENTER);
//   			panelSpeed.add(_comboSpeed,BorderLayout.CENTER);
//   			
//   			
//   			panelMain.add(panelBufferSize);
//   			panelMain.add(panelRadius);
//   			panelMain.add(panelSpeed);
//   			
//   			return panelMain;
//   		}
//   		
//		protected JPanel makeButtonPanel() {
//
//			JPanel mainPanel = new JPanel(new BorderLayout() );
//			JPanel buttonPanel = new JPanel();
//			JButton okButton = new JButton("OK");
//			JButton cancelButton = new JButton("Cancel");
//			
//			buttonPanel.setLayout(new GridLayout(1,2,5,5));
//			buttonPanel.add(okButton);
//			buttonPanel.add(cancelButton);
//			
//			mainPanel.add(buttonPanel,BorderLayout.EAST);
//			
//			cancelButton.addActionListener(
//				new ActionListener() {
//					public void actionPerformed(ActionEvent e) {
//						setVisible(false);
//					}
//				}
//			);
//			
//			okButton.addActionListener(
//					new ActionListener() {
//						public void actionPerformed(ActionEvent e) {
//							
//							StringBuffer buf = new StringBuffer();
//							
//							int tmp0 = ((Number)_textField_bufferSize.getValue()).intValue();
//							int tmp1 = ((Number)_textField_radius.getValue()).intValue();
//							
//							if(tmp0 < MIN_BUFFER_SIZE || MAX_BUFFER_SIZE < tmp0 ) {
//								buf.append("Buffer size");
//							}
//							
//							if(tmp1 < MIN_RADIUS) {
//								
//								if(buf.capacity()>0)
//									buf.append(" and radius");
//								else
//									buf.append("Radius");
//							}
//							
//							if(buf.toString().length()>0) {
//								buf.append( " out of range !\n");
//							}
//							
//							if(_checkCluster.isSelected() || _checkHistogram.isSelected()) {
//								
//								if(_textfield_path.getText().length()<=0) {
//									buf.append( "Missing directory!\n" );
//								}
//								
//								if(_textfield_prefix.getText().length()<=0) {
//									buf.append( "Missig prefix!\n");
//								}
//							}
//							
//							if(buf.toString().length()>0) {
//								JOptionPane.showMessageDialog(null, buf.toString(), "Warning", JOptionPane.WARNING_MESSAGE);
//							}
//							else {
//								
//								// 0 - buffer size
//								// 1 - radius
//								// 2 - fading speed
//								// 3 - cluster auto capture
//								// 4 - histogram auto capture
//								
//								_ret = new Object[5];
//								_ret[0] = new Integer(tmp0);
//								_ret[1] = new Integer(tmp1);
//								_ret[2] = new Integer(_comboSpeed.getSelectedIndex());
//								
//								if(_checkCluster.isSelected())
//									_ret[3] = _textfield_path.getText() + File.separator + _textfield_prefix.getText();
//								
//								if(_checkHistogram.isSelected()) 
//									_ret[4] = _textfield_path.getText() + File.separator + _textfield_prefix.getText();
//								
//								_ok = true;
//								setVisible(false);
//							}
//						}
//					}
//				);
//			
//			
//			return mainPanel;
//		}
//		
//   		
//   		public Object[] ShowDialog() {
//   			show();
//   			
//   			if(_ok)
//   				return _ret;
//   			else
//   				return null;
//   		}
//   }
   

   
//   /**
//    * 
//    * @author treetree
//    *
//    * TODO To change the template for this generated type comment go to
//    * Window - Preferences - Java - Code Style - Code Templates
//    */
//   private class HistogramPereferenceDialog extends JDialog {
//   	
//   		private final int MAX_BUFFER_SIZE = 10000;
//   		private final int MIN_BUFFER_SIZE = 10;
//   		private final int MIN_RADIUS = 0;
//   		private JTextField _textfield_path;
//   		private JTextField _textfield_prefix;
//   		private JCheckBox _checkHistogram;
//   		private JButton _buttonOk;
//   		
//   		private Object[] _ret;
//   		private boolean _ok = false;;
//   	
//   		public HistogramPereferenceDialog() {	
//   			
//   			setTitle("Histogram setting");
//   			
//   			JPanel topPanel = new JPanel();
//   			JPanel mainPanel = new JPanel();
//   			topPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
//   			mainPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
//   			mainPanel.setLayout(new BorderLayout());
//   			topPanel.add(makeAutoCapturePanel());
//   			
//   			mainPanel.add(topPanel, BorderLayout.CENTER);
//	        mainPanel.add(makeButtonPanel(), BorderLayout.SOUTH);
//   			
//   			getContentPane().add(mainPanel, BorderLayout.CENTER);
//   			
//   			setSize(360,220);
//	        setResizable(false);
//			setModal(true);
//			
//			Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
//	        screen.width -= getWidth();
//	        screen.height -= getHeight();
//	        setLocation((int)screen.width/2,(int)screen.height/2);
//  		}
//   		
//   		protected JPanel makeAutoCapturePanel() {
//  
//   			int textWidth = 225;
//   			int textHeight = 25;
//   			int labelWidth = 240;
//   			int buttonWidth = 80;
//   			
//   			JPanel panelMain  = new JPanel(new GridLayout(3,1,5,5));
//   			panelMain.setBorder(BorderFactory.createCompoundBorder(
//		            BorderFactory.createTitledBorder("Auto Capture Setting (optional)"), 
//		            BorderFactory.createEmptyBorder(8,8,8,8)));
//   			
//   			JPanel panelCheckBoxHistogram = new JPanel(new BorderLayout());
//   			JPanel panelPathChooser = new JPanel(new BorderLayout());
//   			JPanel panelPrefix = new JPanel(new BorderLayout());
//   			JLabel labelPrefix = new JLabel("File prefix");
//   			JButton buttonPath = new JButton("Browse");
//   			
//   			
//   			panelPathChooser.setBorder(BorderFactory.createCompoundBorder(
//		            null,BorderFactory.createEmptyBorder(2,2,2,2)));
//   			panelPrefix.setBorder(BorderFactory.createCompoundBorder(
//		            null,BorderFactory.createEmptyBorder(2,2,2,2)));
//   			
//   			
//   			_textfield_path = new JTextField();
//   			_textfield_path.setEditable(false);
//   			_textfield_prefix = new JTextField("his_");
//   			
//   			_checkHistogram = new JCheckBox("Automatic snapshot when window shift");
// 
//   			_checkHistogram.setPreferredSize(new Dimension(labelWidth, textHeight));
// 			buttonPath.setPreferredSize(new Dimension(buttonWidth, textHeight));
//   			labelPrefix.setPreferredSize(new Dimension(buttonWidth, textHeight));
//   			_textfield_path.setPreferredSize(new Dimension(textWidth, textHeight));
//   			_textfield_prefix.setPreferredSize(new Dimension(textWidth, textHeight));
//   			
//
//   			buttonPath.addActionListener(
//   				new ActionListener() {
//   					public void actionPerformed(ActionEvent ev){
//   						JFrame frame = new JFrame();
//   						JFileChooser filechooser = new JFileChooser();
//   						filechooser.setDialogTitle("Choose directory");
//   						filechooser.setDialogType(JFileChooser.DIRECTORIES_ONLY);
//   						filechooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
//
// 
//   				        if(filechooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION)
//   				        {
//   				         _textfield_path.setText(filechooser.getSelectedFile().toString());
//   				        }
//   					}
//   				}
//   			);
//			
//   			panelCheckBoxHistogram.add(_checkHistogram,BorderLayout.WEST);
//   			panelPathChooser.add(buttonPath,BorderLayout.WEST);
//   			panelPathChooser.add(_textfield_path,BorderLayout.CENTER);
//   			panelPrefix.add(labelPrefix,BorderLayout.WEST);
//   			panelPrefix.add(_textfield_prefix,BorderLayout.CENTER);
//   			
//   			panelMain.add(panelCheckBoxHistogram);
//   			panelMain.add(panelPathChooser);
//   			panelMain.add(panelPrefix);
//   			
//   			return panelMain;
//   		}
//
//   		
//		protected JPanel makeButtonPanel() {
//
//			JPanel mainPanel = new JPanel(new BorderLayout() );
//			JPanel buttonPanel = new JPanel();
//			_buttonOk = new JButton("OK");
//			JButton cancelButton = new JButton("Cancel");
//			
//			buttonPanel.setLayout(new GridLayout(1,2,5,5));
//			buttonPanel.add(_buttonOk);
//			buttonPanel.add(cancelButton);
//			
//			mainPanel.add(buttonPanel,BorderLayout.EAST);
//			
//			cancelButton.addActionListener(
//				new ActionListener() {
//					public void actionPerformed(ActionEvent e) {
//						setVisible(false);
//					}
//				}
//			);
//			
//			_buttonOk.addActionListener(
//					new ActionListener() {
//						public void actionPerformed(ActionEvent e) {
//							
//							StringBuffer buf = new StringBuffer();
//							
//						
//							if( _checkHistogram.isSelected()) {
//								
//								if(_textfield_path.getText().length()<=0) {
//									buf.append( "Missing directory!\n" );
//								}
//								
//								if(_textfield_prefix.getText().length()<=0) {
//									buf.append( "Missig prefix!\n");
//								}
//							}
//							
//							if(buf.toString().length()>0) {
//								JOptionPane.showMessageDialog(null, buf.toString(), "Warning", JOptionPane.WARNING_MESSAGE);
//							}
//							else {
//								
//								// 0 - histogram auto capture
//								
//								_ret = new Object[1];
//								
//								if(_checkHistogram.isSelected()) 
//									_ret[0] = _textfield_path.getText() + File.separator + _textfield_prefix.getText();
//								
//								_ok = true;
//								setVisible(false);
//							}
//						}
//					}
//				);
//			
//			
//			return mainPanel;
//		}
//		
//   		
//   		public Object[] ShowDialog() {
//   			
//   			show();
//   			
//   			if(_ok)
//   				return _ret;
//   			else
//   				return null;
//   		}
//   }
}
