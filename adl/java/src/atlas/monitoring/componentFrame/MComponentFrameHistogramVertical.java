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
import atlas.monitoring.component.MComponentHistogramVertical;


/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentFrameHistogramVertical extends MComponentFrameHistogram {

	
	public MComponentFrameHistogramVertical(String parameter_id, Integer dataType, Object[] objs) {
		super(new MComponentHistogramVertical(objs), "MComponentFrameHistogramVertical", parameter_id, dataType.intValue(), null);
		
		Container container = getContentPane();

		
		_mainPanel = new JPanel();
		
		_mainPanel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createTitledBorder("Vertical Histogram"), 
                BorderFactory.createEmptyBorder(0,1,0,1)));
		_mainPanel.setLayout(new BorderLayout());
		_mainPanel.add(((MComponentHistogramVertical)_monitoringComponent).GetHistogramArea(), BorderLayout.CENTER);
		_mainPanel.setPreferredSize(new Dimension(600,400));
		
		this._statusBox = createBox();
		
        container.add(_mainPanel, BorderLayout.CENTER);
        container.add(this._statusBox, BorderLayout.PAGE_END);
        setBorder(BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        //setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
        
	    pack();
	    show();
	}
	
	public class SaveAction extends AbstractAction implements Serializable {
	    
		public SaveAction() {
	      super("SaveAction");
	    }
		
		public void actionPerformed(ActionEvent e) {
			
			try {
		    	RenderedImage rendImage = null;
		    	
		    	rendImage = ((MComponentHistogramVertical)_monitoringComponent).SnapShotHistogram();
		    	
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

