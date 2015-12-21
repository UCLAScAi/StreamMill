/*
 * Created on Feb 20, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.gui;
import java.awt.*;
import java.awt.Container;
import java.awt.Image;

import java.net.URL;

import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.ImageIcon;
import javax.swing.JPanel;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class LogoFrame extends JFrame {

	public LogoFrame (URL imageURL) {
		
		try {
			
			ImageIcon icon = new ImageIcon(imageURL);
			int height = icon.getIconHeight();
			int width = icon.getIconWidth();
			Image image = icon.getImage();
			
			LogoPanel logo = new LogoPanel(image);
	        
			
			Container container = getContentPane();
	        container.setLayout(new BoxLayout(container,BoxLayout.PAGE_AXIS));
	        container.add(logo);	    
	         
	        
	        
	        logo.setPreferredSize(new Dimension(width,height));
	        
	        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
	        screen.width -= width;
	        screen.height -= height;
	        
	        
	        
	        setLocation((int)screen.width/2,(int)screen.height/2);
	        
	        setUndecorated(true);
	        
	        logo.repaint();
	        //setModal(true);
		    pack();
		    show();
		    
		    
			
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public class LogoPanel extends JPanel {
		
		Image _image = null;
		
		public LogoPanel(Image image) {
			this._image = image;
		}
		
		protected void paintComponent(Graphics g) {
	        
			if (isOpaque()) { //paint background
	            g.setColor(getBackground());
	            g.fillRect(0, 0, getWidth(), getHeight());
	        }

	        if (this._image != null) {
	            g.drawImage(this._image,0,0,this);
	        }
	    }

	}
	
}
