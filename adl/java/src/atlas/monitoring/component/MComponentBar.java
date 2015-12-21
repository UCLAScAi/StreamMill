/*
 * Created on Feb 27, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;

import atlas.event.*;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentBar extends MComponent 
	implements IRefreshEventListener, IValueChangeEventListener {
	//
	// Preference
	//
	private int _max_vertical_value = 50;
	private int _outputValue = 0;
	private int _numOfBar = 0;
	private int _numOfOutputBar = 40;	
	// 
	// GUI
	//	
	private int _LABEL_HEIGHT = 17;
	private int _GAP_SIDE = 7;
	private int _GAP_TOP = 7;
	private Font _LABEL_FONT;
	private int _LABEL_FONTSize = 12;
	private boolean _hasValue = false;
	//
	// bar preference
	//
	private int _BAR_WIDTH = 0;
	private int _BAR_HEIGHT = 0;
	private int _BAR_GAP = 2;
	
	private Color DEFAULT_COLOR_BAR_BACKGROUND = Color.GRAY; //Color.getHSBColor(0.32f,0.73f,0.51f);
	private Color DEFAULT_COLOR_BAR_OUTPUT = Color.GREEN;
	private Color DEFAULT_COLOR_BACKGROUND = Color.BLACK;
	private Color DEFAULT_COLOR_NO_VALUE_OUTPUT = Color.RED;
	
	private Color _color_bar_background = DEFAULT_COLOR_BAR_BACKGROUND;
	private Color _color_bar_output = DEFAULT_COLOR_BAR_OUTPUT;
	private Color _color_background = DEFAULT_COLOR_BACKGROUND;
	private Color _color_no_value_output = DEFAULT_COLOR_NO_VALUE_OUTPUT;
	
	//
	// Method
	//
	public MComponentBar() {
		super("MComponentBar");
		
		setOpaque(true);
		
		setBackground(DEFAULT_COLOR_BACKGROUND);
		
		_LABEL_FONT = new Font("Arial", Font.PLAIN, _LABEL_FONTSize);
	}
	
	public void CleanUp() {
		//do nothing
	}
	
	public int GetMaxValue() {
		return this._max_vertical_value;
	}
	
	public void AddValue(String textValue) {
		try {
			if(textValue==null)
				throw new NumberFormatException();
			
			_outputValue = Integer.parseInt(textValue);
			
			_hasValue = true;
			
			repaint();
		}
		catch(NumberFormatException e) {
			throw e;
		}
		catch(Exception e) {
			e.printStackTrace();
		}
	}
	
	public void AddValue(int intValue) {
		
		_outputValue = intValue;
		
		_hasValue = true;
		
		repaint();
	}
	
	public void SkipValue() {
		
		_hasValue = false;
		_outputValue = 0;
		
		repaint();
	}
	
	public void RefreshEventOccurred(RefreshEvent e) {
	
		if(e.IsHasValue())
			AddValue(e.GetValue());
		else
			SkipValue();
	}
	
	public void ValueChangeEventOccurred(ValueChangeEvent e) {
		
		if(e.GetName().equalsIgnoreCase(MComponentWaveform.MAX_VERTICAL_VALUE )) {
			this._max_vertical_value = (int)e.GetValue();
		}
	}
	
	//
	// Get method
	//
	public int Get_NumOfBar() {
		return _numOfBar;
	}
	
	public int Get_NumOfOutputBar() {
		return _numOfOutputBar;
	}
	
	public int Get_MaximumVerticalValue() {
		return this._max_vertical_value;
	}
	
	//
	// Set method
	//
	public void Set_MaximumVerticalValue(int max) {
		_max_vertical_value = max;
	}
	
	public void Set_Color_Bar_Output(Color color) {
		_color_bar_output = color;
	}
	
	public void Set_Color_Background(Color color) {
		_color_background = color;
	}
	
	public Color Get_Color_Bar_Output() {
		return _color_bar_output;
	}
	
	public Color Get_Color_Background() {
		return _color_background;
	}
	
	public void UpdateColorPreference() {
		repaint();
	}
	
	/**
	 * Component paint
	 */
	protected void paintComponent(Graphics g) {
	//Paint background if we're opaque.
		if (isOpaque()) {
			g.setColor(_color_background); //getBackground());
			g.fillRect(0, 0, getWidth(), getHeight());
		}
		
		Dimension dim = getSize();
		
		int _bar_gap_x_2 = _BAR_GAP << 1;
		int _gap_side_div_2 = _GAP_SIDE << 1;
		int height_minus_label_height_minus_gap_top = dim.height - _LABEL_HEIGHT - _GAP_TOP;
		
		_BAR_HEIGHT = height_minus_label_height_minus_gap_top;
		_BAR_WIDTH = dim.width - _gap_side_div_2;
		
		_numOfBar = height_minus_label_height_minus_gap_top / _bar_gap_x_2;
		
		//remember to do the multiple first
		_numOfOutputBar = (_numOfBar * _outputValue) / _max_vertical_value; 
		
		
		//draw output bar
		int y = 0;
		
		g.setColor(_color_bar_output);
		
		int count = 0;
		int cntdown = _numOfOutputBar;
		
		for( y=_BAR_HEIGHT; y > _GAP_TOP + _BAR_GAP; y -= _bar_gap_x_2 ) {
			
			count++;
			
			if(cntdown <= 0)  {
				g.setColor(_color_bar_background);
			}
			else {
				cntdown--;
			}
			
			g.fillRect(_GAP_SIDE,y,_BAR_WIDTH,_BAR_GAP);
		}

		if(y > _GAP_TOP ) {
			g.fillRect(_GAP_SIDE,y,_BAR_WIDTH,y-_GAP_TOP);
			count++;
		}
		
		// draw output value
		if(_hasValue)		
			g.setColor(_color_bar_output);
		else 
			g.setColor(_color_no_value_output);
			
		g.setFont(_LABEL_FONT);
		
		String value = String.valueOf(_outputValue);
		
		FontMetrics metrics = g.getFontMetrics();
		
		g.drawString(value, (dim.width - metrics.stringWidth( value ))>> 1, dim.height - (metrics.getHeight() >> 1) );
	}
}
