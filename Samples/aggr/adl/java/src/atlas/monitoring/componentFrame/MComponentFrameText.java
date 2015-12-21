/*
 * Created on Feb 21, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.componentFrame;

import java.awt.BorderLayout;
import java.awt.Container;

import java.awt.event.ActionEvent;

import java.io.Serializable;
import java.io.*;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.text.DecimalFormat;
import java.lang.StringBuffer;


import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;



import atlas.monitoring.component.MComponentText;
import atlas.gui.CustomFileFilter;
/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentFrameText extends MComponentFrame {

	private JPanel mainPanel;
	private Calendar _cal = new GregorianCalendar();
	private DecimalFormat _twoDigitFormat = new DecimalFormat();
	private boolean _isPause = false;
    //
    // Bottom
    //
    private Box _statusBox;
	private JTextField _statusTextField;				// Very bottom of the interface
	private JTextField _countTextField;
	private int _numOfItem;
		
	public MComponentFrameText(String parameter_id, Integer dataType, Object[] objs)  {
		super(new MComponentText(), "MComponentFrameText", parameter_id, dataType.intValue(), null);
		
		_twoDigitFormat.setMinimumIntegerDigits(2);
		_numOfItem = 0;
		
		mainPanel = new JPanel();
 
        mainPanel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createTitledBorder("History"), 
                BorderFactory.createEmptyBorder(5,5,5,5)));
        mainPanel.setLayout(new BoxLayout(mainPanel,BoxLayout.PAGE_AXIS));
        mainPanel.add(_monitoringComponent);
        
        //STATUS BOX
        this._statusBox = createBox();
        
        Container container = getContentPane();
        //container.setLayout(new BoxLayout(container,BoxLayout.PAGE_AXIS));
        
        container.add(mainPanel, BorderLayout.CENTER);
        container.add(this._statusBox, BorderLayout.PAGE_END);
        //setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
        setBorder(BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        
        setSize(200,600);
        
	    pack();
	    show();
	}
	
    protected Box createBox() {
    	Box myBox = new Box(BoxLayout.X_AXIS);
        this._statusTextField = new JTextField("Ready!", 15);
        this._countTextField = new JTextField("0",5);
        this._countTextField.setEnabled(false);
		this._statusTextField.setEnabled(false);
		myBox.add(this._statusTextField);
		myBox.add(this._countTextField);
		
		return myBox;
    }
    
    public void AddValue(String[] textValue) {
    	
    	if(_isPause)
    		return;
    	
    	_cal = new GregorianCalendar();
    	
    	StringBuffer buf = new StringBuffer();
    	
    	boolean isAddSeperator = false;
    	
    	for( int i=0; i<textValue.length; i++) {
    		
    		if(isAddSeperator) {
    			buf.append( ", ");
    		}
    		else {
    			isAddSeperator = true;
    		}
    		
    		buf.append(textValue[i]);
    	}
    		
    	
    	this._monitoringComponent.AddValue(buf.toString());
    	
    	this._statusTextField.setText( 
    			_cal.get(Calendar.MONTH) + 1 + "/" +
    			_cal.get(Calendar.DAY_OF_MONTH) + "/" +
    			_cal.get(Calendar.YEAR) + "   " +
    			_twoDigitFormat.format(_cal.get(Calendar.HOUR)) + ":" +
    			_twoDigitFormat.format(_cal.get(Calendar.MINUTE)) + ":" +
    			_twoDigitFormat.format(_cal.get(Calendar.SECOND)) + " " +
    			(_cal.get(Calendar.AM) == Calendar.AM ? "AM" : "PM" ) );
    	
    	_numOfItem++;
    	this._countTextField.setText(String.valueOf(_numOfItem));
    }
    
    public int GetNumberOfItem() {
    	return _numOfItem;
    }
    
	public void ClearOutput() {
		
		_numOfItem = 0;
		_statusTextField.setText("Reset");
		this._countTextField.setText("0");
		
		SetPause(true);
		
		((MComponentText)_monitoringComponent).ClearOutput();
		
		SetPause(false);		
	}
    
    public synchronized void SetPause(boolean pause) {
		this._isPause = pause;
	}
    
    private String GetOutputString() {
    	String str;
    	SetPause(true);
    	str = ((MComponentText)_monitoringComponent).GetOutputString();
    	SetPause(false);
    	return str;
    }
    
	//
	// Menu Action 
	//Start Pause - Clear
	public class StartAction extends AbstractAction implements Serializable {
	    
		public StartAction() {
	      super("StartAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			SetPause(false);
			
			_statusTextField.setText("Ready");
		}
	}
	
	public class PauseAction extends AbstractAction implements Serializable {
	    
		public PauseAction() {
	      super("PauseAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			SetPause(true);
			
			_statusTextField.setText("Pause: " + _statusTextField.getText());
		}
	}
	
	public class ClearAction extends AbstractAction implements Serializable {
	    
		public ClearAction() {
	      super("ClearAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			ClearOutput();
		}
	}
	
	public class SaveAction extends AbstractAction implements Serializable {
	    
		private final String newline = "\n"; 
		
		public SaveAction() {
	      super("SaveAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			try {
				
				JFrame frame = new JFrame();
				
				JFileChooser filechooser = new JFileChooser();
				filechooser.setDialogTitle("Save History");
				filechooser.setDialogType(JFileChooser.SAVE_DIALOG);
				CustomFileFilter filter = new CustomFileFilter();
			    filter.addExtension("txt");
			    filter.setDescription("text file");
			    filechooser.setFileFilter(filter);

			    if(filechooser.showSaveDialog(frame) == JFileChooser.CANCEL_OPTION )
			    	return;
			    
			    String fileName = filechooser.getSelectedFile().getAbsolutePath();
			    
			    if(!fileName.endsWith(".txt")) {
			    	fileName = fileName.concat(".txt");
			    }
//			    System.out.println(filechooser.getSelectedFile().getAbsolutePath());
//			    System.out.println(fileName);
			    
			    File outFile = new File(fileName);
				
				if(outFile.exists()) {
					int answer = JOptionPane.showConfirmDialog(frame, "Do you want to overwrite the file?");
				    if (answer != JOptionPane.YES_OPTION) 
				    	return;
				    
			    	boolean success = outFile.delete();
			        if (!success) {
			        	JOptionPane.showMessageDialog(frame, "Fail to delete the old file!");
						return;
			        }
			        
			        if(!outFile.createNewFile()){
						JOptionPane.showMessageDialog(frame, "Fail to create file!");
						return;
					}
				}
				else if(!outFile.createNewFile()){
					JOptionPane.showMessageDialog(frame, "Fail to create file!");
					return;
				}
				
				FileOutputStream fileOutStream = new FileOutputStream(outFile);
				
				StringBuffer buf = new StringBuffer();
				
				buf.append("##################################################################" + newline);
				buf.append(" id   : " + GetParameterID() + newline );
				buf.append(" Date : " + _cal.get(Calendar.MONTH) + "/" +
						    			_cal.get(Calendar.DAY_OF_MONTH) + "/" +
						    			_cal.get(Calendar.YEAR) + "   " +
						    			_twoDigitFormat.format(_cal.get(Calendar.HOUR)) + ":" +
						    			_twoDigitFormat.format(_cal.get(Calendar.MINUTE)) + ":" +
						    			_twoDigitFormat.format(_cal.get(Calendar.SECOND)) + " " +
						    			(_cal.get(Calendar.AM) == Calendar.AM ? "AM" : "PM" )  + newline);
				buf.append(" Number of Item : " + _numOfItem  + newline);
				buf.append("##################################################################" + newline);
				buf.append(GetOutputString());
				
				fileOutStream.write(buf.toString().getBytes());
				
			    fileOutStream.close(); 
			}
			catch(Exception ex) {
				ex.printStackTrace();
			}
		}

	}
	
	//public class TextFilter extends Filef
	
	public class ExitAction extends AbstractAction implements Serializable {
	    
		public ExitAction() {
	      super("ExitAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			doDefaultCloseAction();
		}
	}
}
