/*
 * Created on Feb 18, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.componentFrame;

import java.awt.*;
import java.awt.Component;
import java.awt.Container;
import java.awt.event.*;
import java.awt.event.ActionEvent;
import java.awt.image.*;
import java.io.Serializable;
import java.io.*;
import javax.imageio.ImageIO;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.*;



import javax.swing.JPanel;


import atlas.event.IValueChangeEventListener;

import atlas.event.ValueChangeEvent;
import atlas.gui.CustomFileFilter;
import atlas.monitoring.component.MComponentBar;
import atlas.monitoring.component.MComponentWaveform;
import atlas.monitoring.library.MGlobal;


/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentFrameWaveform extends MComponentFrame 
	implements IValueChangeEventListener {

	private Thread _refreshThread;
	private Thread updateThread;
	private Thread _listenerThread1;
	
    private JPanel _guiPanel;
    private JPanel _waveformPanel;
    private JPanel _barPanel;
    private JPanel _parameterPanel;

    private MComponentBar _bar;
	private int _selectedField;
	private String _title;
	
	private JLabel _refreshRateValueLabel;
	private JLabel _maxVerticalValueLabel;
	private JLabel _minVerticalValueLabel;
	private JLabel _gridScaleValueLabel; 

	private PreferenceDialog _preferenceDialog;
	
	//
	// Constructer
	//
	public MComponentFrameWaveform(String parameter_id, Integer dataType, Object[] obj)  {
		super(new MComponentWaveform(), "MComponentFrameWaveform", parameter_id, dataType.intValue(), null);
		
		try {
			_fieldIndex = ((Integer)obj[0]).intValue();
		}
		catch(Exception ex) {
			_fieldIndex = 1;
		}
		
		_title = "<" + MGlobal.getDescription(dataType.intValue()) + "> " + parameter_id + " (Field " + _fieldIndex + ")";
		setTitle(_title);

        //Create the phase selection and display panels.
        _waveformPanel = new JPanel();
        _barPanel = new JPanel();
        _parameterPanel = new JPanel();
        _guiPanel = new JPanel();
	    
        addGuiWidgets();
        addParameterWidgets();
        
        //Gui panel
        _guiPanel.setLayout(new BorderLayout());
        //_guiPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
        _guiPanel.add(_barPanel,BorderLayout.LINE_START);
        _guiPanel.add(_waveformPanel,BorderLayout.CENTER);
         
        Container container = getContentPane();
        container.setLayout(new BorderLayout());	//new BoxLayout(container,BoxLayout.PAGE_AXIS));

        JPanel mainPanel = new JPanel();
        mainPanel.setBorder(BorderFactory.createCompoundBorder(null,BorderFactory.createEmptyBorder(5,5,5,5)));
        mainPanel.setLayout(new BorderLayout());
        mainPanel.add(_parameterPanel, BorderLayout.SOUTH);
        mainPanel.add(_guiPanel, BorderLayout.CENTER);	 

        container.add(mainPanel, BorderLayout.CENTER);
                
	    //Start the meterThread
        ((MComponentWaveform)_monitoringComponent).Start();
        
        setMinimumSize(new Dimension(410,200));
        
        setBorder(BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
         
	    pack();
	    show();
	}
	
	public void ShowPreferenceDialog() {
        _preferenceDialog = new PreferenceDialog(this);
        _preferenceDialog.show();
        _preferenceDialog = null;
	}
	
	public class PreferenceDialog extends JDialog {
		
		private JTabbedPane _tabbedPane;
		private MComponentFrameWaveform _owner;
		private JButton _gridColorButton;
		private JButton _waveformColorButton;
		private JButton _backgroundColorButton;
		
		public PreferenceDialog(MComponentFrameWaveform owner) {
			
			if(owner==null)
				return;
			
			_owner = owner;
			
			this.setSize(400,330);
	        _gridColorButton = new JButton();
			_waveformColorButton = new JButton();
			_backgroundColorButton = new JButton();			
			_tabbedPane = createTabbedPanelUI();
			
			 //Make sure we have nice window decorations.
	        setDefaultLookAndFeelDecorated(true);
	        setTitle("Monitor Property");

	        JPanel mainPanel = new JPanel();
	        JPanel buttonPanel = makeButtonPanel();
	        JPanel settingPanel = new JPanel();
	        
	        settingPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	        settingPanel.setLayout(new BorderLayout());
	        settingPanel.add(_tabbedPane, BorderLayout.CENTER);
	        
	        mainPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	        mainPanel.setLayout(new BorderLayout());
	        mainPanel.add(_tabbedPane, BorderLayout.CENTER);
	        mainPanel.add(buttonPanel, BorderLayout.SOUTH);
	        
	        getContentPane().add(mainPanel, BorderLayout.CENTER);
	        
	        setResizable(false);
			setModal(true);
		}
	
		protected JTabbedPane createTabbedPanelUI() {
	    	JTabbedPane tabbedPane = new JTabbedPane();
	        tabbedPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
	    	
	        JComponent panel1 = makeMonitorPreferencePanel();
	        tabbedPane.addTab("Monitor", null, panel1,"Monitoring Preference settings");
	        tabbedPane.setMnemonicAt(0, KeyEvent.VK_M);

	        JComponent panel2 = makeColorPreferencePanel();
	        tabbedPane.addTab("Color", null, panel2,"Color settings");
	        tabbedPane.setMnemonicAt(1, KeyEvent.VK_C);
	    	
	        return tabbedPane;
	    }
		
		protected JPanel makeButtonPanel() {
			JPanel panel = new JPanel();
			JPanel buttonPanel = new JPanel();
			JButton okButton = new JButton("OK");
			JButton cancelButton = new JButton("Cancel");
			JButton applyButton = new JButton("Apply");
			JButton resetButton = new JButton("Reset");
			
			buttonPanel.setLayout(new GridLayout(1,3,5,5));
			buttonPanel.add(okButton);
			buttonPanel.add(cancelButton);
			buttonPanel.add(applyButton);
			
			panel.setLayout(new BorderLayout());
			panel.setBorder(BorderFactory.createEmptyBorder(5,5,3,3));
			panel.add(buttonPanel, BorderLayout.EAST);
			panel.add(resetButton,BorderLayout.WEST);

			applyButton.addActionListener(
				new ActionListener(){
					public void actionPerformed(ActionEvent e) {
						if(_tabbedPane.getSelectedIndex() == 1) {
							applyColorPreference();
						}
						else if (_tabbedPane.getSelectedIndex() == 0) { 
							applyScalePreference();
						}
					}
				}
			);
			
			cancelButton.addActionListener(
				new ActionListener(){
					public void actionPerformed(ActionEvent e) {
						_preferenceDialog.setVisible(false);
					}
				}
			);
			
			okButton.addActionListener(
					new ActionListener(){
						public void actionPerformed(ActionEvent e) {
							
							if(!applyScalePreference())
								return;
							
							applyColorPreference();
							
							_preferenceDialog.setVisible(false);	
						}
					}
				);
			
			resetButton.addActionListener(
					new ActionListener(){
						public void actionPerformed(ActionEvent e) {
							if(_tabbedPane.getSelectedIndex() == 1) {
								_gridColorButton.setBackground(((MComponentWaveform)_owner._monitoringComponent).Get_Default_Color_Grid());
								_waveformColorButton.setBackground(((MComponentWaveform)_owner._monitoringComponent).Get_Default_Color_Waveform());
								_backgroundColorButton.setBackground(((MComponentWaveform)_owner._monitoringComponent).Get_Default_Color_MeterBackground());
							}
							else if(_tabbedPane.getSelectedIndex() == 0) {
								_refreshTextField.setValue(new Integer(((MComponentWaveform)_monitoringComponent).Get_RefreshRate()));
								_numOfDelayTextField.setValue(new Integer(((MComponentWaveform)_monitoringComponent).Get_Num_Of_Delay()));
								_maxValueTextField.setValue(new Integer(((MComponentWaveform)_monitoringComponent).Get_Max_Vertical_Value()));
								_gridScaleTextField.setValue(new Double(((MComponentWaveform)_monitoringComponent).Get_Scale_VerticalValuePerGrid()));
								_autoScaleCheckBox.setSelected(((MComponentWaveform)_monitoringComponent).isAutoScale());
								_autoResizeToMaxCheckBox.setSelected(((MComponentWaveform)_monitoringComponent).isAutoResizeToMax());
							}
						}
					}
				);
			
			return panel;
		}
		
		protected boolean applyScalePreference() {
			
			
			boolean pass = true;
			int cnt = 0;
			StringBuffer buf = new StringBuffer();
			
			boolean isMaxSelected = _maxValueRadioButton.isSelected();
			
			try {

			int refreshrate = ((Number)_refreshTextField.getValue()).intValue();
			int numOfDelay =  ((Number)_numOfDelayTextField.getValue()).intValue();
			int maxValue = (isMaxSelected ? ((Number)_maxValueTextField.getValue()).intValue() : 0); 
			double gridscale = (!isMaxSelected ?((Number)_gridScaleTextField.getValue()).intValue() : 0 );
			
			if(refreshrate < MComponentWaveform.MINIMUM_REFRESH_RATE) {
				buf.append("Refresh rate");
				pass = false;
				cnt++;
			}
			
			if(numOfDelay < MComponentWaveform.MIN_NUM_OF_DELAY) {
				buf.append("Data delay");
				pass = false;
				cnt++;
			}
			
			if(isMaxSelected && maxValue < MComponentWaveform.MINIMUM_MAXIMUM_VERTICAL_UNIT) {
				buf.append((!pass ? ", " : "") + "Max grid value");
				pass = false;
				cnt++;
			}
			
			if( !isMaxSelected && gridscale < MComponentWaveform.MINIMUM_VERTICAL_VALUE_PER_GRID) {
				buf.append((!pass ? ", " : "") + "Grid scale");
				pass = false;
				cnt++;
			}
			
			if(!pass) {
				
				JFrame frame = new JFrame();
				
				buf.append( " " + (cnt > 1 ? "are" : "is") + " out of range!" );
				
				JOptionPane.showMessageDialog(frame,buf.toString());
			}
			else {
				((MComponentWaveform)_owner._monitoringComponent).Set_RefreshRate(refreshrate);
				((MComponentWaveform)_owner._monitoringComponent).Set_NumOfDelay(numOfDelay);
				
				if(isMaxSelected)
					((MComponentWaveform)_owner._monitoringComponent).Set_Max_Vertical_Value(maxValue);
				else
					((MComponentWaveform)_owner._monitoringComponent).Set_Scale_VerticalValuePerGrid(gridscale);
				
				
				((MComponentWaveform)_owner._monitoringComponent).AutoScale(_autoScaleCheckBox.isSelected());
				((MComponentWaveform)_owner._monitoringComponent).AutoResizeToMax(_autoResizeToMaxCheckBox.isSelected());
				((MComponentWaveform)_owner._monitoringComponent).UpdateScalePreference();
			}
			}
			catch(Exception e) {
				e.printStackTrace();
			}
			
			return pass;
		}
		
		protected void applyColorPreference() {
			((MComponentWaveform)_owner._monitoringComponent).Set_Color_Grid(_gridColorButton.getBackground());
			((MComponentWaveform)_owner._monitoringComponent).Set_Color_Waveform(_waveformColorButton.getBackground());
			((MComponentWaveform)_owner._monitoringComponent).Set_Color_MeterBackground(_backgroundColorButton.getBackground());
			((MComponentWaveform)_owner._monitoringComponent).UpdateColorPreference();
		}
		
		protected JComponent makeColorPreferencePanel() {
			JPanel panel = new JPanel(new GridLayout(3,1,5,5));
			panel.setBorder(BorderFactory.createCompoundBorder(
		            BorderFactory.createTitledBorder("Color Settings"), 
		            BorderFactory.createEmptyBorder(8,8,8,8)));
			
			int width_1 = 130;
			int width_2 = 200;
			int height = 20;
			int height_2 = 50;
			
			JPanel gridColorPanel = new JPanel(new BorderLayout());
			JPanel waveformColorPanel = new JPanel(new BorderLayout());
			JPanel backgroundColorPanel = new JPanel(new BorderLayout());
						
			JLabel gridLabel = new JLabel("Grid color");
			JLabel waveformLabel = new JLabel("Waveform color");
			JLabel backgroundLabel = new JLabel("Background color");

			_gridColorButton.setBackground(((MComponentWaveform)_owner._monitoringComponent).Get_Color_Grid());
			_waveformColorButton.setBackground(((MComponentWaveform)_owner._monitoringComponent).Get_Color_Waveform());
			_backgroundColorButton.setBackground(((MComponentWaveform)_owner._monitoringComponent).Get_Color_MeterBackground());
					
			_gridColorButton.addActionListener(
				new ActionListener(){
					public void actionPerformed(ActionEvent e) {
						_gridColorButton.setBackground(JColorChooser.showDialog(null, "Grid", _gridColorButton.getBackground()));
					}
				}
			);
			
			_waveformColorButton.addActionListener(
				new ActionListener(){
					public void actionPerformed(ActionEvent e) {
						_waveformColorButton.setBackground(JColorChooser.showDialog(null, "Waveform", _waveformColorButton.getBackground()));
					}
				}
			);
			
			_backgroundColorButton.addActionListener(
				new ActionListener(){
					public void actionPerformed(ActionEvent e) {
						_backgroundColorButton.setBackground(JColorChooser.showDialog(null, "Background", _backgroundColorButton.getBackground()));
					}
				}
			);
			
			_gridColorButton.setPreferredSize(new Dimension(width_2, height_2));
			_waveformColorButton.setPreferredSize(new Dimension(width_2, height_2));
			_backgroundColorButton.setPreferredSize(new Dimension(width_2, height_2));
			
			gridLabel.setPreferredSize(new Dimension(width_1, height));
			waveformLabel.setPreferredSize(new Dimension(width_1, height));
			backgroundLabel.setPreferredSize(new Dimension(width_1, height));
			
			gridColorPanel.add(gridLabel,BorderLayout.WEST);
			gridColorPanel.add(_gridColorButton,BorderLayout.EAST);
			
			waveformColorPanel.add(waveformLabel,BorderLayout.WEST);
			waveformColorPanel.add(_waveformColorButton,BorderLayout.EAST);
			
			backgroundColorPanel.add(backgroundLabel,BorderLayout.WEST);
			backgroundColorPanel.add(_backgroundColorButton,BorderLayout.EAST);
			
			panel.add(gridColorPanel);
			panel.add(waveformColorPanel);
			panel.add(backgroundColorPanel);
			
			return panel;
		}
		
		private	JRadioButton _maxValueRadioButton;
		private JRadioButton _gridScaleRadioButton;
		
		private JCheckBox _autoResizeToMaxCheckBox;
		private JCheckBox _autoScaleCheckBox;
		private JFormattedTextField _maxValueTextField;
		private JFormattedTextField _gridScaleTextField;
		private JFormattedTextField _refreshTextField;
		private JFormattedTextField _numOfDelayTextField;
		
		protected JComponent makeMonitorPreferencePanel() {
			JPanel panel = new JPanel(new GridLayout(6,1,5,5));
			panel.setBorder(BorderFactory.createCompoundBorder(
		            BorderFactory.createTitledBorder("Scale Settings"), 
		            BorderFactory.createEmptyBorder(8,8,8,8)));
			
			JPanel refreshPanel = new JPanel(new BorderLayout());
			JPanel maxValuePanel = new JPanel(new BorderLayout());
			JPanel gridScalePanel = new JPanel(new BorderLayout());
			JPanel numOfDelayPanel = new JPanel(new BorderLayout());
			JPanel autoScalePanel = new JPanel(new BorderLayout());
			JPanel autoResizeToMaxPanel = new JPanel(new BorderLayout());
			
			
			JLabel refreshLabel = new JLabel("Refresh rate");
			JLabel numOfDelaylabel = new JLabel("Data delay ");
			
			_maxValueRadioButton = new JRadioButton("Max grid value");
			_gridScaleRadioButton = new JRadioButton("Grid scale");
			
			ButtonGroup buttonGroup = new ButtonGroup();
			buttonGroup.add(_maxValueRadioButton);
			buttonGroup.add(_gridScaleRadioButton);
			
			_autoScaleCheckBox = new JCheckBox("Automatic scale");
			_autoResizeToMaxCheckBox = new JCheckBox("Automatic resize to max");
			
			
			JLabel refreshTipsLabel = new JLabel( "   >= " + MComponentWaveform.MINIMUM_REFRESH_RATE + " ms" );
			JLabel numOfDelayTipsLabel  = new JLabel( "   >= " + MComponentWaveform.MIN_NUM_OF_DELAY + " refresh interval");
			JLabel maxValueTipsLabel = new JLabel( "   >= " + MComponentWaveform.MINIMUM_MAXIMUM_VERTICAL_UNIT);
			JLabel gridScaleTipsLabel = new JLabel("   >= " + MComponentWaveform.MINIMUM_VERTICAL_VALUE_PER_GRID );
			
			_maxValueTextField = new JFormattedTextField();
			_gridScaleTextField = new JFormattedTextField();
			_refreshTextField = new JFormattedTextField();
			_numOfDelayTextField  = new JFormattedTextField();
			
			_refreshTextField.setValue(new Integer(((MComponentWaveform)_monitoringComponent).Get_RefreshRate()));
			_refreshTextField.setColumns(10);
			
			_numOfDelayTextField.setValue(new Integer(((MComponentWaveform)_monitoringComponent).Get_Num_Of_Delay()));
			_numOfDelayTextField.setColumns(10);
			
			_maxValueTextField.setValue(new Integer(((MComponentWaveform)_monitoringComponent).Get_Max_Vertical_Value()));
			_maxValueTextField.setColumns(10);
			
			_gridScaleTextField.setValue(new Double(((MComponentWaveform)_monitoringComponent).Get_Scale_VerticalValuePerGrid()));
			_gridScaleTextField.setColumns(10);
			
			_autoScaleCheckBox.setSelected(((MComponentWaveform)_monitoringComponent).isAutoScale());
			_autoResizeToMaxCheckBox.setSelected(((MComponentWaveform)_monitoringComponent).isAutoResizeToMax());
			_maxValueRadioButton.setSelected(true);
			_gridScaleTextField.setEnabled(false);
			
			_maxValueRadioButton.addItemListener(
				new ItemListener() {
					public void itemStateChanged(ItemEvent e) {
						if(e.getStateChange() == ItemEvent.SELECTED) {
							_gridScaleRadioButton.setSelected(false);	
							_gridScaleTextField.setEnabled(false);
							_maxValueTextField.setEnabled(true);
						}
					}
				}
			);
			
			_gridScaleRadioButton.addItemListener(
					new ItemListener() {
						public void itemStateChanged(ItemEvent e) {
							if(e.getStateChange() == ItemEvent.SELECTED) {
								_maxValueRadioButton.setSelected(false);
								_maxValueTextField.setEnabled(false);
								_gridScaleTextField.setEnabled(true);
							}
						}
					}
				);
			
			int height = 30;
			int label_width = 130;
			int textfield_width = 50;
			int tips_width = 100;
			
			refreshLabel.setPreferredSize(new Dimension(label_width,height));
			numOfDelaylabel.setPreferredSize(new Dimension(label_width,height));
			_maxValueRadioButton.setPreferredSize(new Dimension(label_width,height));
			_gridScaleRadioButton.setPreferredSize(new Dimension(label_width,height));
			_autoScaleCheckBox.setPreferredSize(new Dimension(label_width,height));
			_autoResizeToMaxCheckBox.setPreferredSize(new Dimension(label_width,height));
			
			refreshTipsLabel.setPreferredSize(new Dimension(tips_width,height));
			numOfDelayTipsLabel.setPreferredSize(new Dimension(tips_width,height));
			maxValueTipsLabel.setPreferredSize(new Dimension(tips_width,height));
			gridScaleTipsLabel.setPreferredSize(new Dimension(tips_width,height));
			
			_refreshTextField.setPreferredSize(new Dimension(textfield_width,height));
			_numOfDelayTextField.setPreferredSize(new Dimension(textfield_width,height));
			_maxValueTextField.setPreferredSize(new Dimension(textfield_width,height));
			_gridScaleTextField.setPreferredSize(new Dimension(textfield_width,height));
			
			
			refreshPanel.add(refreshLabel,BorderLayout.WEST);
			numOfDelayPanel.add(numOfDelaylabel,BorderLayout.WEST);
			maxValuePanel.add(_maxValueRadioButton,BorderLayout.WEST);
			gridScalePanel.add(_gridScaleRadioButton,BorderLayout.WEST);
			autoScalePanel.add(_autoScaleCheckBox,BorderLayout.WEST);
			autoResizeToMaxPanel.add(_autoResizeToMaxCheckBox,BorderLayout.WEST);
			
			refreshPanel.add(_refreshTextField,BorderLayout.CENTER);
			numOfDelayPanel.add(_numOfDelayTextField,BorderLayout.CENTER);
			maxValuePanel.add(_maxValueTextField,BorderLayout.CENTER);
			gridScalePanel.add(_gridScaleTextField,BorderLayout.CENTER);
			
			refreshPanel.add(refreshTipsLabel,BorderLayout.EAST);
			numOfDelayPanel.add(numOfDelayTipsLabel,BorderLayout.EAST);
			maxValuePanel.add(maxValueTipsLabel,BorderLayout.EAST);
			gridScalePanel.add(gridScaleTipsLabel,BorderLayout.EAST);
			
			panel.add(refreshPanel);
			panel.add(numOfDelayPanel);
			panel.add(maxValuePanel);
			panel.add(gridScalePanel);
			panel.add(autoScalePanel);
			panel.add(autoResizeToMaxPanel);
			
			return panel;
		}
	}
	
	private void addParameterWidgets() {

		int labelHeight = 14;
		
		JLabel refreshRateLabel = new JLabel("Refresh Rate : ");
		JLabel maxVerticalLabel = new JLabel("Max grid value : ");
		JLabel minVerticalLabel = new JLabel("Min grid value : ");
		JLabel gridScaleCheckBox = new JLabel("Grid scale : ");
		
		refreshRateLabel.setPreferredSize(new Dimension(100, labelHeight));
		maxVerticalLabel.setPreferredSize(new Dimension(120, labelHeight));
		minVerticalLabel.setPreferredSize(new Dimension(120, labelHeight));
		gridScaleCheckBox.setPreferredSize(new Dimension(100, labelHeight));
		
		_refreshRateValueLabel = new JLabel(((MComponentWaveform)_monitoringComponent).Get_RefreshRate() + " ms");
		_maxVerticalValueLabel = new JLabel(((MComponentWaveform)_monitoringComponent).Get_Max_Vertical_Value() + "");
		_minVerticalValueLabel = new JLabel("0");
		_gridScaleValueLabel = new JLabel(((MComponentWaveform)_monitoringComponent).Get_Scale_VerticalValuePerGrid() + "");
		
		Font font = new Font("Arial", Font.PLAIN, 12);
		
		_refreshRateValueLabel.setFont(font);
		_maxVerticalValueLabel.setFont(font);
		_minVerticalValueLabel.setFont(font);
		_gridScaleValueLabel.setFont(font);
		
		_refreshRateValueLabel.setPreferredSize(new Dimension(50, labelHeight));
		_maxVerticalValueLabel.setPreferredSize(new Dimension(50, labelHeight));
		_minVerticalValueLabel.setPreferredSize(new Dimension(50, labelHeight));
		_gridScaleValueLabel.setPreferredSize(new Dimension(50, labelHeight));
		
		JPanel refreshRatePanel = new JPanel(new BorderLayout());
		JPanel maxVerticalPanel = new JPanel(new BorderLayout());
		JPanel minVerticalPanel = new JPanel(new BorderLayout());
		JPanel gridScalePanel = new JPanel(new BorderLayout());
				
		refreshRatePanel.add(refreshRateLabel, BorderLayout.WEST);
		refreshRatePanel.add(_refreshRateValueLabel, BorderLayout.CENTER);
		
		maxVerticalPanel.add(maxVerticalLabel, BorderLayout.WEST);
		maxVerticalPanel.add(_maxVerticalValueLabel, BorderLayout.CENTER);
		
		minVerticalPanel.add(minVerticalLabel, BorderLayout.WEST);
		minVerticalPanel.add(_minVerticalValueLabel, BorderLayout.CENTER);
		
		gridScalePanel.add(gridScaleCheckBox, BorderLayout.WEST);
		gridScalePanel.add(_gridScaleValueLabel, BorderLayout.CENTER);		
		
		_parameterPanel.setLayout(new BorderLayout() ); 
	
		JPanel inPanel = new JPanel();
		inPanel.setBorder(BorderFactory.createCompoundBorder(
	            BorderFactory.createTitledBorder("Settings"), 
	            BorderFactory.createEmptyBorder(5,5,5,5)));

		inPanel.setLayout(new GridLayout(2,3, 40, 10));
		
		inPanel.add(maxVerticalPanel);
		inPanel.add(refreshRatePanel);
		inPanel.add(minVerticalPanel);
		inPanel.add(gridScalePanel);
		
		_parameterPanel.add(inPanel,BorderLayout.WEST);
		
		((MComponentWaveform)_monitoringComponent).addValueChangeEventListener(this);
	}

    /*
     * Get the images and set up the widgets.
     */
    private void addGuiWidgets() {

    	_bar = new MComponentBar();
    	_bar.setPreferredSize(50,150);
    	
    	//Add a border around the display panel.
    	_barPanel.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createTitledBorder("Value"), 
            BorderFactory.createEmptyBorder(5,5,5,5)));
    	_barPanel.setLayout(new BoxLayout(_barPanel,BoxLayout.PAGE_AXIS));
        
    	//Add a border around the display panel.
        _waveformPanel.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createTitledBorder("History"), 
            BorderFactory.createEmptyBorder(5,5,5,5)));
        _waveformPanel.setLayout(new BoxLayout(_waveformPanel,BoxLayout.PAGE_AXIS));
        
        ((MComponentWaveform)_monitoringComponent).Set_RefreshRate(1000);
        ((MComponentWaveform)_monitoringComponent).Set_Scale_PixelPerUnit(2);
        ((MComponentWaveform)_monitoringComponent).Set_Scale_VerticalValuePerGrid(1);
        ((MComponentWaveform)_monitoringComponent).Set_Default_RefreshRate(1000);
        ((MComponentWaveform)_monitoringComponent).Set_Default_PixelPerUnit(2);
        ((MComponentWaveform)_monitoringComponent).Set_Default_VerticalValuePerGrid(1);
        
        ((MComponentWaveform)_monitoringComponent).AutoScale(true);
        
        ((MComponentWaveform)_monitoringComponent).setAlignmentX(Component.LEFT_ALIGNMENT);
        
        ((MComponentWaveform)_monitoringComponent).addRefreshEventListener(_bar);
        ((MComponentWaveform)_monitoringComponent).addValueChangeEventListener(_bar);
        
        _monitoringComponent.setPreferredSize(400,150);
        
        _barPanel.add(_bar);
        _waveformPanel.add(_monitoringComponent);
    }
    
    public void ValueChangeEventOccurred(ValueChangeEvent evt) {
    	if(evt.GetName().equalsIgnoreCase(MComponentWaveform.GRID_SCALE_VALUE)) {
    		_gridScaleValueLabel.setText(String.valueOf(evt.GetValue()));
    	}
    	else if(evt.GetName().equalsIgnoreCase(MComponentWaveform.MAX_VERTICAL_VALUE)) {
    		_maxVerticalValueLabel.setText(String.valueOf(evt.GetValue()));
    	}
    	else if(evt.GetName().equalsIgnoreCase(MComponentWaveform.REFRESH_RATE_VALUE)) {
    		_refreshRateValueLabel.setText(String.valueOf(evt.GetValue()) + " ms");
    	}
    }
    
    /**
     * Add Value
     */
    public void AddValue(String[] textValue) {
    	
    	if(textValue.length<_fieldIndex) {
    		this.setTitle(_title + " out of bound!");
    		((MComponentWaveform)_monitoringComponent).SuspendRefreshing();
    		return;
    	}
    	this.setTitle(_title);
     	this._monitoringComponent.AddValue(textValue[_fieldIndex-1]);
    }
    
    //***************************************
    // MenuBar Action
    //***************************************
    public class SaveAction extends AbstractAction implements Serializable {
	    
		public SaveAction() {
	      super("SaveAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
		    // Write generated image to a file
		    try {
		    	RenderedImage rendImage = ((MComponentWaveform)_monitoringComponent).SnapShot();
		    	
		    	if(rendImage==null) {
		    		JOptionPane.showMessageDialog(null, "Fail to render the image.", "Error", JOptionPane.WARNING_MESSAGE);
		    		return;
		    	}
		    	
		    	JFrame frame = new JFrame();
		    	JFileChooser filechooser = new JFileChooser();
				filechooser.setDialogTitle("Save waveform snapshot");
				filechooser.setDialogType(JFileChooser.SAVE_DIALOG);
				CustomFileFilter filter = new CustomFileFilter();
			    filter.addExtension("jpg");
			    filter.setDescription("jpg file");
			    filechooser.setFileFilter(filter);

			    if(filechooser.showSaveDialog(frame) == JFileChooser.CANCEL_OPTION )
			    	return;
			    
			    String fileName = filechooser.getSelectedFile().getAbsolutePath();
			    
			    if(!fileName.endsWith(".jpg")) {
			    	fileName = fileName.concat(".jpg");
			    }
		    	
		        // Save as jpg
		        File file = new File(fileName);
		        ImageIO.write(rendImage, "jpg", file);
		        
		    } catch (IOException ex) {
		    	JOptionPane.showMessageDialog(null, ex.getMessage(), "Error", JOptionPane.WARNING_MESSAGE);
		    }
	    
		}
		

	}
    
    public class ExitAction extends AbstractAction implements Serializable {
	    
		public ExitAction() {
	      super("ExitAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			doDefaultCloseAction();
		}
	}
    
    public class StartAction extends AbstractAction implements Serializable {
	    
		public StartAction() {
	      super("StartAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			((MComponentWaveform)_monitoringComponent).ResumeRefreshing();
		}
	}
    
    public class PauseAction extends AbstractAction implements Serializable {
	    
		public PauseAction() {
	      super("PauseAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			((MComponentWaveform)_monitoringComponent).SuspendRefreshing();
		}
	}
    
    public class PropertyAction extends AbstractAction implements Serializable {
	    
		public PropertyAction() {
	      super("PropertyAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			ShowPreferenceDialog();
		}
	}
    
    public class ClearAction extends AbstractAction implements Serializable {
	    
		public ClearAction() {
	      super("ClearAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			((MComponentWaveform)_monitoringComponent).ClearBuffer();
		}
	}
    
    //***************************************
    //Testing purpose
    //***************************************
    
    public void Test(int rate, int max) {
    	((MComponentWaveform)_monitoringComponent).AutoScale(true);
    	((MComponentWaveform)_monitoringComponent).Set_RefreshRate(rate);
    	((MComponentWaveform)_monitoringComponent).Set_Max_Vertical_Value(max);
    }
    
    public boolean TestMaxValueInBar(int max) {
    	return _bar.GetMaxValue() == max;
    }
    
    public boolean TestBarAndWaveformSychronized() {
    	boolean isSych = true;
    	
    	isSych = (_bar.GetMaxValue() == ((MComponentWaveform)_monitoringComponent).Get_Max_Vertical_Value());
     	
    	return isSych;
    }
    
    public void TestAutoScale(boolean autoScale) {
    	((MComponentWaveform)_monitoringComponent).AutoScale(autoScale);
    }
    
    public void TestResize(int width, int height) {
    	this.resize(width,height);
    }
    
    public void SetSelectedField(int field) {
    	if(field < 1)
    		field = 1;
    	
    	this._selectedField = field;
    }
}


