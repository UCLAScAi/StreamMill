/*
 * Created on Mar 23, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.componentFrame;

import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JPanel;
import javax.swing.JTextField;

import atlas.monitoring.component.MComponent;
import atlas.monitoring.component.MComponentHistogram;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MComponentFrameHistogram extends MComponentFrame {
	
	

	protected JPanel _mainPanel;
	protected Calendar _cal = new GregorianCalendar();
	protected DecimalFormat _twoDigitFormat = new DecimalFormat();
	protected Box _statusBox;
	protected JTextField _statusTextField;				// Very bottom of the interface
	protected int _prevClusterIndex = -99;
	
	public MComponentFrameHistogram(MComponent com, String frameName, String parameter_id, int dataType, Object[] objs ) {
		super(com, frameName, parameter_id, dataType, objs);
	}
	
	
	protected Box createBox() {
    	Box myBox = new Box(BoxLayout.X_AXIS);
        this._statusTextField = new JTextField("", 18);
		this._statusTextField.setEnabled(false);
		myBox.add(this._statusTextField);
		
		this._statusTextField.setText( "Start up: " + getDateTimeString() );
		
		return myBox;
    }

	
	public void AddValue(String[] strings) {
		
		if(strings==null || (strings.length != 4  && strings.length != 3)) {
			this._statusTextField.setText( getDateTimeString() + " - Receive invalid histogram data.");
			return;
		}
		
		try {

      int slideWindowIdx = Integer.parseInt(strings[0]);
      int clusterId = Integer.parseInt(strings[1]);
      if(strings.length == 4) {
        int x = (int)Double.parseDouble(strings[2]);
        int y = (int)Double.parseDouble(strings[3]);
      
			  if(clusterId < 0) {
				
				  this._statusTextField.setText( getDateTimeString() + " - Receive invalid histogram data.");
				  return;
			  }
			
			  ((MComponentHistogram)_monitoringComponent).AddValue(x, y, clusterId, slideWindowIdx);
      }
      else if (strings.length == 3) {
        int cnt = Integer.parseInt(strings[2]);
        
        if(clusterId < 0) {
          this._statusTextField.setText( getDateTimeString() + " - Receive invalid histogram data.");
          return;
        }
      
        ((MComponentHistogram)_monitoringComponent).AddValue(cnt, clusterId, slideWindowIdx);
      }
			
			this._statusTextField.setText( "Last update: " + getDateTimeString() );
			
			if(_prevClusterIndex == -99)
			{
				_prevClusterIndex = slideWindowIdx;
			}
			else if (slideWindowIdx > _prevClusterIndex) {
				_prevClusterIndex = slideWindowIdx;
			}
			else {return;}
		}
		catch(Exception ex) {
			
			ex.printStackTrace();
			
			this._statusTextField.setText( getDateTimeString() + " - Receive invalid clustering data.");
		}
	}
	
	protected String getDateTimeString() {
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
	

}
