/*
 * Created on Mar 14, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.componentFrame;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.image.RenderedImage;
import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;

import javax.imageio.ImageIO;
import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;

import atlas.gui.CustomFileFilter;
import atlas.monitoring.component.MComponentCluster;


/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentFrameCluster extends MComponentFrame {

	private JPanel _mainPanel;
	private Calendar _cal = new GregorianCalendar();
	private DecimalFormat _twoDigitFormat = new DecimalFormat();
    private Box _statusBox;
	private JTextField _statusTextField;				// Very bottom of the interface
	private JTextField _countTextField;
	private int _numOfItem = 0;
	private int _prevClusterIndex = -99;
	private Object[] _arguments;
	
	public MComponentFrameCluster(String parameter_id, Integer dataType, Object[] objs) {
		super(new MComponentCluster(objs), "MComponentFrameCluster", parameter_id, dataType.intValue(), null);
		
		Container container = getContentPane();
        
		_arguments = objs;
		
		_mainPanel = new JPanel();
		
		_mainPanel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createTitledBorder("Cluster"), 
                BorderFactory.createEmptyBorder(0,1,0,1)));
		_mainPanel.setLayout(new BorderLayout());
		_mainPanel.add(((MComponentCluster)_monitoringComponent).GetClusterArea(), BorderLayout.CENTER);
		_mainPanel.setPreferredSize(new Dimension(600,400));
		
		this._statusBox = createBox();
		
        container.add(_mainPanel, BorderLayout.CENTER);
        container.add(this._statusBox, BorderLayout.PAGE_END);
        //setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
        setBorder(BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        
	    pack();
	    show();
	}
	
	protected Box createBox() {
    	Box myBox = new Box(BoxLayout.X_AXIS);
        this._statusTextField = new JTextField("", 30);
        this._countTextField = new JTextField("Cluster : None",5);
        this._countTextField.setEnabled(false);
		this._statusTextField.setEnabled(false);
		myBox.add(this._statusTextField);
		myBox.add(this._countTextField);
		
		this._statusTextField.setText( "Start up: " + getDateTimeString() );
		
		return myBox;
    }

	
	public void AddValue(String[] strings) {
		
		if(strings==null || strings.length != 4 ) {
			this._statusTextField.setText( getDateTimeString() + " - Receive invalid clustering data.");
			return;
		}
		
		try {
			double x = Double.parseDouble(strings[2]);
			double y = Double.parseDouble(strings[3]);
			int clusterId = Integer.parseInt(strings[1]);
			int slideWindowIdx = Integer.parseInt(strings[0]);
			
			if(clusterId < 0) {
				
				this._statusTextField.setText( getDateTimeString() + " - Receive invalid clustering data.");
				return;
			}
			
			((MComponentCluster)_monitoringComponent).AddClusterElementValue((int)x,(int)y,clusterId,slideWindowIdx);
			
			this._statusTextField.setText( "Last update: " + getDateTimeString() );
			
			if(_prevClusterIndex == -99)
			{
				_prevClusterIndex = slideWindowIdx;
				_numOfItem++;
			}
			else if (slideWindowIdx > _prevClusterIndex) {
				_numOfItem++;
				_prevClusterIndex = slideWindowIdx;
			}
			else {return;}
			
			this._countTextField.setText("Cluster: " + String.valueOf(_numOfItem));
		}
		catch(Exception ex) {
			
			ex.printStackTrace();
			
			this._statusTextField.setText( getDateTimeString() + " - Receive invalid clustering data.");
		}
	}
	
	private String getDateTimeString() {
		_cal = new GregorianCalendar();
		
		return _cal.get(Calendar.MONTH) + "/" +
		_cal.get(Calendar.DAY_OF_MONTH) + "/" +
		_cal.get(Calendar.YEAR) + "   " +
		_twoDigitFormat.format(_cal.get(Calendar.HOUR)) + ":" +
		_twoDigitFormat.format(_cal.get(Calendar.MINUTE)) + ":" +
		_twoDigitFormat.format(_cal.get(Calendar.SECOND)) + " " +
		(_cal.get(Calendar.AM) == Calendar.AM ? "AM" : "PM" );
	}
	
	//**************************
	//* Menu
	//***************************
	
	public class SaveAction extends AbstractAction implements Serializable {
	    
		public SaveAction() {
	      super("SaveAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			
			try {
		    	RenderedImage rendImage = null;
		    	
		    	rendImage = ((MComponentCluster)_monitoringComponent).SnapShotCluster();
		    	
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

