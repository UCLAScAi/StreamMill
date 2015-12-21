/*
 * Created on Feb 14, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.util.Date;

import javax.swing.BoxLayout;
import javax.swing.JComponent;

import atlas.event.*;
import atlas.monitoring.library.MCircularBuffer;
/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentWaveform extends MComponent {

	//share
	public final static String MAX_VERTICAL_VALUE = "maxVerticalValue";
	public final static String REFRESH_RATE_VALUE = "refreshrate";
	public final static String GRID_SCALE_VALUE = "gridscale";
	private Thread _testThread;
	
	//**********************************
	//* Variable
	//**********************************
	//
	// Constant
	//
	private static final int PREFERENCE_CHANGE_NONE = 0;
	private static final int PREFERENCE_CHANGE_COLOR = 1;
	private static final int PREFERENCE_CHANGE_SCALE = 2;
	private static final int PREFERENCE_CHANGE_AUTO_SCALE = 4;
	public static final int MINIMUM_REFRESH_RATE = 100;		// milli-second
	private static final int MINIMUM_PIXEL_PER_UNIT = 1;
	public static final double MINIMUM_VERTICAL_VALUE_PER_GRID = 0.000001;		//avoid divided by zero
	public static final int MINIMUM_MAXIMUM_VERTICAL_UNIT = 1;
	private final int UNIT_PER_GRID = 5;
	//
	// Default
	//
	private int DEFAULT_BUFFER_SIZE = 1024;
	private int DEFAULT_REFRESH_RATE = 1000;		//1 second
	private int DEFAULT_PIXEL_PER_UNIT = 2;
	private double DEFAULT_VERTICAL_VALUE_PER_GRID = 1;
	private Color DEFAULT_COLOR_GRID = Color.getHSBColor(0.32f,0.73f,0.51f);
	private Color DEFAULT_COLOR_WAVEFORM = Color.GREEN;
	private Color DEFAULT_COLOR_METER_BACKGROUND = Color.BLACK;
	public static int MIN_NUM_OF_DELAY = 1;
	private int _numOfDelay = MIN_NUM_OF_DELAY;
	//
	// User Preference Variable
	//
	protected Color _color_grid;
	protected Color _color_waveform;
	protected Color _color_meter_background;
	protected int _refresh_rate;
	protected int _pixel_per_unit;
	protected double _vertical_value_per_grid;
	protected boolean _isAutoScale = false;
	protected boolean _isAutoResizeToMax = true;
	//
	// Internal variable
	//
	private int _max_vertical_value_meterArea;
	private MeterArea _meterArea;
	private Thread _refreshThread;
	private boolean _hasWriten;
	private boolean _isRefreshing = false;
	private int _preferenceChangeIndex = PREFERENCE_CHANGE_NONE;
	protected MCircularBuffer _buffer;
	protected boolean _suspendRefreshing = false;
	//
	// Event
	//
	protected javax.swing.event.EventListenerList _refreshListenerList = new javax.swing.event.EventListenerList();
	protected javax.swing.event.EventListenerList _valuChangeListenerList = new javax.swing.event.EventListenerList();
	
	//**********************************
	//* Main methods
	//**********************************	
			
	/**
	 * Constructor
	 * @param refresh_rate
	 */
	public MComponentWaveform() {
		
		super("MComponentWaveform");
		
		this._preferredSize = new Dimension(600,150);
		this._prev_dimension = this._preferredSize;
		
		this._buffer = new MCircularBuffer(DEFAULT_BUFFER_SIZE);
		
		this.Set_Scale_PixelPerUnit(this.DEFAULT_PIXEL_PER_UNIT);
		this.Set_Scale_VerticalValuePerGrid(this.DEFAULT_VERTICAL_VALUE_PER_GRID);
		this.Set_RefreshRate(this.DEFAULT_REFRESH_RATE);
		this.Set_Color_Grid(this.DEFAULT_COLOR_GRID);
		this.Set_Color_Waveform(this.DEFAULT_COLOR_WAVEFORM);
		this.Set_Color_MeterBackground(this.DEFAULT_COLOR_METER_BACKGROUND);
		
		this._meterArea = new MeterArea(this);
		this._meterArea.setAlignmentX(Component.LEFT_ALIGNMENT);
        this.setLayout(new BoxLayout(this,BoxLayout.PAGE_AXIS));
		this.add(this._meterArea);
		
		this._refreshThread = new Thread(new RefreshThread(this), "RefreshThread" );
 	}
	
	public void ClearBuffer() {
		this._suspendRefreshing = true;
		this._buffer.ClearAll();
		this._suspendRefreshing = false;
	}
	
	public synchronized void SuspendRefreshing() {
		this._suspendRefreshing = true;
	}
	
	public synchronized void ResumeRefreshing() {
		this._suspendRefreshing = false;
		//this._refreshThread.notify();
	}
	
	protected void finalize() {
		this.CleanUp();	
	}
	
	public void AddValue(String textValue) {
		try {
			double value = Double.parseDouble(textValue);
			
			addValue(value);
		}
		catch(NumberFormatException ex) {
			
			//throw ex;
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
	}
	
	/**
	 * Add the value to the performance meter
	 * @param value
	 */
	private synchronized void addValue( double value ) {
		while(this._isRefreshing) {
			try {
				wait();
			}catch(InterruptedException e) {
			}
		}
		
		//todo: handle the negative value
		this._buffer.Put( value );
		this._hasWriten = true;
	}

	/**
	 * Switch on the performance meter
	 *
	 */
	public void Start() {
		if(!this._refreshThread.isAlive())
		{
			this._refreshThread.start();
		}
	}
	
	/**
	 * Switch off the performance meter
	 *
	 */
	public void CleanUp() {
		if(this._refreshThread.isAlive())
		{
			this._refreshThread.stop();
		}
		
		if( _refreshListenerList != null ) {
			
			Object[] listeners = _refreshListenerList.getListenerList();
	        // Each listener occupies two elements - the first is the listener class
	        // and the second is the listener instance
	        for (int i=0; i<listeners.length; i+=2 ) {
	        	if (listeners[i]==IRefreshEventListener.class) {
	        		_refreshListenerList.remove(IRefreshEventListener.class,(IRefreshEventListener)listeners[i+1]);
	        	}
	        }
		}
		
		if( _valuChangeListenerList != null ) {
			Object[] listeners = _valuChangeListenerList.getListenerList();
	        // Each listener occupies two elements - the first is the listener class
	        // and the second is the listener instance
	        for (int i=0; i<listeners.length; i+=2) {
	        	if (listeners[i]==IValueChangeEventListener.class) {
	        		_valuChangeListenerList.remove(IValueChangeEventListener.class,(IValueChangeEventListener)listeners[i+1]);
	        	}
	        }
		}
	}
	
	private Dimension _prev_dimension;
	
	//
	// Getting Preference variable
	//
	public int Get_Max_Vertical_Value() 				{	return this._max_vertical_value_meterArea;	}
	public Color Get_Color_Grid()			 			{	return this._color_grid;		}
	public Color Get_Color_Waveform() 					{	return this._color_waveform;	}
	public Color Get_Color_MeterBackground()			{	return this._color_meter_background;	}
	public int Get_RefreshRate()						{	return this._refresh_rate;				}
	public int Get_Scale_PixelPerUnit()					{	return this._pixel_per_unit;	}
	public double Get_Scale_VerticalValuePerGrid()		{	return this._vertical_value_per_grid;	}
	public Color Get_Default_Color_Grid()				{	return this.DEFAULT_COLOR_GRID;			}
	public Color Get_Default_Color_Waveform()			{	return this.DEFAULT_COLOR_WAVEFORM;		}
	public Color Get_Default_Color_MeterBackground()	{	return this.DEFAULT_COLOR_METER_BACKGROUND;	}
	public int Set_Default_RefreshRate()				{	return this.DEFAULT_REFRESH_RATE;	}
	public int Get_Default_PixelPerUnit()				{	return this.DEFAULT_PIXEL_PER_UNIT;	}
	public double Get_Default_VerticalValuePerGrid()	{	return this.DEFAULT_VERTICAL_VALUE_PER_GRID;	}
	public int Get_Num_Of_Delay()						{	return this._numOfDelay;	}

	
	public void Set_NumOfDelay(int delay) {
		if(delay<MIN_NUM_OF_DELAY) {
			_numOfDelay = MIN_NUM_OF_DELAY;
		}
		else {
			_numOfDelay = delay;
		}
	}
	
	//
	// Setting Preference variable
	//	
	public void Set_Max_Vertical_Value(int max_value) {
		
		set_max_vertical_value(max_value < MINIMUM_MAXIMUM_VERTICAL_UNIT ? MINIMUM_MAXIMUM_VERTICAL_UNIT : max_value);
		this._prev_dimension = this._meterArea.getSize(); 
		//this._vertical_value_per_grid = (double)(max_value * this.UNIT_PER_GRID * this._pixel_per_unit)/ (double)this._prev_dimension.getHeight();   
		
		Dimension dim = getSize();
		
		set_scale_verticalvalueGrid( ((double)(max_value * this.UNIT_PER_GRID * this._pixel_per_unit)/ (double)this._prev_dimension.getHeight()));
		
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_SCALE; 
	}
	
	public void set_scale_verticalvalueGrid(double vvaluePerGrid) {
		this._vertical_value_per_grid = vvaluePerGrid < MINIMUM_VERTICAL_VALUE_PER_GRID ? MINIMUM_VERTICAL_VALUE_PER_GRID : vvaluePerGrid;
		
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_SCALE;
		
		fireValueChangeEvent(new ValueChangeEvent(this,GRID_SCALE_VALUE,_vertical_value_per_grid));
	}
	
	public void Set_Scale_VerticalValuePerGrid(double vvaluePerGrid) {
		
		this._vertical_value_per_grid = vvaluePerGrid < MINIMUM_VERTICAL_VALUE_PER_GRID ? MINIMUM_VERTICAL_VALUE_PER_GRID : vvaluePerGrid;
		
		if( this._isAutoScale) {
			recalc_maxVerticalValueMeterArea();		
		}
		
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_SCALE;
		
		fireValueChangeEvent(new ValueChangeEvent(this,GRID_SCALE_VALUE,_vertical_value_per_grid));
	}
	
	private void set_max_vertical_value(int max) {
		this._max_vertical_value_meterArea = max;
		fireValueChangeEvent(new ValueChangeEvent(this,MAX_VERTICAL_VALUE,max));
	}
	
	public void Set_Scale_PixelPerUnit(int pixel_per_unit) {
		this._pixel_per_unit = pixel_per_unit < MINIMUM_PIXEL_PER_UNIT ? MINIMUM_PIXEL_PER_UNIT : pixel_per_unit;
		
		if( this._isAutoScale) {
			recalc_maxVerticalValueMeterArea();		
		}
		
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_SCALE;
	}
	
	
	public void AutoScale(boolean choice) {
		if(choice && !this._isAutoScale) {
			recalc_maxVerticalValueMeterArea();
		}
		this._isAutoScale = choice;
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_SCALE; 
	}
	
	public void AutoResizeToMax(boolean choice) {
		this._isAutoResizeToMax = choice;
	}
	
	public boolean isAutoResizeToMax() {
		return this._isAutoResizeToMax;
	}
	
	public boolean isAutoScale() {
		return this._isAutoScale;
	}
	
	//Helper method
	private void recalc_maxVerticalValueMeterArea() {
		Dimension dim = this.getSize();
		
		if( dim.width > 0 && dim.height > 0)
			this._prev_dimension = dim; 
		double tmp = (double)(this._prev_dimension.getHeight() * this._vertical_value_per_grid) / (double)(this._pixel_per_unit * this.UNIT_PER_GRID) ;
		set_max_vertical_value( (int)( tmp < MINIMUM_MAXIMUM_VERTICAL_UNIT ? MINIMUM_MAXIMUM_VERTICAL_UNIT : tmp) );
	}
	
	public void Set_Color_Grid(Color color) {
		this._color_grid = color;
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_COLOR;
	}
	
	public void Set_Color_Waveform(Color color) {
		this._color_waveform = color;
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_COLOR;
	}
	
	public void Set_Color_MeterBackground(Color color) {
		this._color_meter_background = color;
		this._preferenceChangeIndex |= PREFERENCE_CHANGE_COLOR;
	}
	
	public void Set_RefreshRate(int refresh_rate) {
		this._refresh_rate = refresh_rate < MINIMUM_REFRESH_RATE ? MINIMUM_REFRESH_RATE : refresh_rate;
		
		fireValueChangeEvent(new ValueChangeEvent(this,REFRESH_RATE_VALUE,_refresh_rate));
	}
	
	public void Set_Default_Color_Grid(Color color) {
		this.DEFAULT_COLOR_GRID = color;
	}
	
	public void Set_Default_Color_Waveform(Color color) {
		this.DEFAULT_COLOR_WAVEFORM = color;
	}
	
	public void Set_Default_Color_MeterBackground(Color color) {
		this.DEFAULT_COLOR_METER_BACKGROUND = color;
	}
	
	public void Set_Default_RefreshRate(int refresh_rate) {
		this.DEFAULT_REFRESH_RATE = refresh_rate < MINIMUM_REFRESH_RATE ? MINIMUM_REFRESH_RATE : refresh_rate;
	}
	
	public void Set_Default_PixelPerUnit(int pixel_per_unit) {
		this.DEFAULT_PIXEL_PER_UNIT = pixel_per_unit < MINIMUM_PIXEL_PER_UNIT ? MINIMUM_PIXEL_PER_UNIT : pixel_per_unit;
	}
	
	public void Set_Default_VerticalValuePerGrid(int vvaluePerGrid) {
		this.DEFAULT_VERTICAL_VALUE_PER_GRID = vvaluePerGrid < MINIMUM_VERTICAL_VALUE_PER_GRID ? MINIMUM_VERTICAL_VALUE_PER_GRID : vvaluePerGrid;
	}
	
	public void ResetColor() {
		this.Set_Color_Grid(this.DEFAULT_COLOR_GRID);
		this.Set_Color_MeterBackground(this.DEFAULT_COLOR_METER_BACKGROUND);
		this.Set_Color_Waveform(this.DEFAULT_COLOR_WAVEFORM);
	}
	
	public void ResetScale() {
		this.Set_Scale_PixelPerUnit(this.DEFAULT_PIXEL_PER_UNIT);
		this.Set_Scale_VerticalValuePerGrid(this.DEFAULT_VERTICAL_VALUE_PER_GRID);
	}
	
	public void UpdateColorPreference() {
		SuspendRefreshing();
		this._meterArea.UpdateColorPreference();
		this._preferenceChangeIndex ^= PREFERENCE_CHANGE_COLOR;
		ResumeRefreshing();
	}
	
	public void UpdateScalePreference() {
		SuspendRefreshing();
		this._meterArea.UpdateMeterScalePreference();
		this._preferenceChangeIndex ^= PREFERENCE_CHANGE_SCALE;
		ResumeRefreshing();
	}
	
	//***************************************
	//* protected methods
	//***************************************
	
	protected synchronized void Refresh() {
		
		if(this._isAutoScale) {
			Dimension dim = this.getSize();
			
			if( dim.width > 0 && dim.height > 0 ) {
				if(this._prev_dimension.width != dim.width || this._prev_dimension.height != dim.height ) {
					this.Set_Max_Vertical_Value(this._max_vertical_value_meterArea);
					this._prev_dimension = dim;
				}
			}
		}
		else {
			Dimension dim = this.getSize();
			
			if( dim.width > 0 && dim.height > 0 ) {
				if(this._prev_dimension.width != dim.width || this._prev_dimension.height != dim.height ) {
					set_max_vertical_value((int)(dim.height * _vertical_value_per_grid) / (this.UNIT_PER_GRID * this._pixel_per_unit));
					this._prev_dimension = dim;
				}
			}
		}
		
		//handle preference change
		if( _preferenceChangeIndex != 0){
			if((_preferenceChangeIndex & PREFERENCE_CHANGE_COLOR) == PREFERENCE_CHANGE_COLOR)	UpdateColorPreference();	
			if((_preferenceChangeIndex & PREFERENCE_CHANGE_SCALE) == PREFERENCE_CHANGE_SCALE)	UpdateScalePreference();
		}
		
		//MeterArea refreshing
		this._isRefreshing = true;
		
		if(!_hasWriten) {
			this._buffer.Skip();
		}
		else {
			this._buffer.Increment();
		}
		
		this._meterArea.Refresh();
		
		this._hasWriten = false;
		this._isRefreshing = false;
		notifyAll();		
	}
	
	/**
	 * Take the snapshot of current waveform
	 * @return
	 */
	public RenderedImage SnapShot(){
		
		boolean isSuspended = _suspendRefreshing;
		
		if(!isSuspended)
			SuspendRefreshing();
		
		RenderedImage image = _meterArea.RenderWaveformImage();
		
		if(!isSuspended)
			ResumeRefreshing();
		
		return image;
	}
	
	//
	// Event handling
	//
	public void addRefreshEventListener(IRefreshEventListener listener) {
        _refreshListenerList.add(IRefreshEventListener.class, listener);
    }

    // This methods allows classes to unregister for MyEvents
    public void removeRefreshEventListener(IRefreshEventListener listener) {
        _refreshListenerList.remove(IRefreshEventListener.class, listener);
    }

    // This private class is used to fire MyEvents
    private void fireRefreshEvent(RefreshEvent evt) {
        Object[] listeners = _refreshListenerList.getListenerList();
        // Each listener occupies two elements - the first is the listener class
        // and the second is the listener instance
        for (int i=0; i<listeners.length; i+=2) {
            if (listeners[i]==IRefreshEventListener.class) {
                ((IRefreshEventListener)listeners[i+1]).RefreshEventOccurred(evt);
            }
        }
    }
    
	public void addValueChangeEventListener(IValueChangeEventListener listener) {
        _valuChangeListenerList.add(IValueChangeEventListener.class, listener);
    }

    // This methods allows classes to unregister for MyEvents
    public void removeValueChangeListener(IValueChangeEventListener listener) {
    	_valuChangeListenerList.remove(IValueChangeEventListener.class, listener);
    }

    // This private class is used to fire MyEvents
    private void fireValueChangeEvent(ValueChangeEvent evt) {
        Object[] listeners = _valuChangeListenerList.getListenerList();
        // Each listener occupies two elements - the first is the listener class
        // and the second is the listener instance
        for (int i=0; i<listeners.length; i+=2) {
            if (listeners[i]==IValueChangeEventListener.class) {
                ((IValueChangeEventListener)listeners[i+1]).ValueChangeEventOccurred(evt);
            }
        }
    }
    
	/**
	 * 
	 * @author treetree
	 *
	 * TODO To change the template for this generated type comment go to
	 * Window - Preferences - Java - Code Style - Code Templates
	 */
	public static class RefreshThread implements Runnable {
		
		private MComponentWaveform _parent;
		private long prevTime;
		private Date _stamp;
		
		public RefreshThread( MComponentWaveform parent ) {
			this._parent = parent;
			this._stamp = new Date();
		}
		
		public void run() {
			
			long curTime;
			long sleepTime;
			
			while(true) {
				
				try
				{
					prevTime = _stamp.getTime();
					
					synchronized(this) {
						while(this._parent._suspendRefreshing) {
							Thread.sleep(this._parent._refresh_rate);
						}
					}
				}
				catch(InterruptedException e){}
				
				this._parent.Refresh();

			 	try {
			 		
			 		curTime = _stamp.getTime();
			 		
			 		sleepTime = this._parent._refresh_rate - (curTime - prevTime);
			 		
			 		if(sleepTime <0)
			 			continue;
			 		
			 		Thread.sleep(sleepTime);
			    }
			    catch(InterruptedException e) {
			    	return;
			    }
		    }
		}
	}
	
	/**
	 * 
	 * @author treetree
	 *
	 * JComponet: MeterArea - customer draw for the performance meter
	 */
    public class MeterArea extends JComponent {
    	
		//CoordinatesDemo controller;
    	private MComponentWaveform _parent;
		private Color _gridColor;
		private int _offset = 0;
		private int _unit_per_grid;
		private int _pixel_per_grid;
		private double _vertical_pixel_per_value;
		//private int _max_vertical_value_meterArea;
		private boolean _isAutoScale;
		private Dimension _prev_Dimension;
		
		private Color _color_grid;
		private Color _color_waveform;
		private int _pixel_per_unit;
		private double _vertical_value_per_grid;
		private Object[] _parameterObj = new Object[2];
		
		/**
		 * Construct
		 * @param parent
		 */
		public MeterArea(MComponentWaveform parent) {
			setOpaque(true);
			this._parent = parent;
			this._unit_per_grid = this._parent.UNIT_PER_GRID;
		}
				
		public void UpdateMeterScalePreference() {
			this._pixel_per_unit = this._parent._pixel_per_unit;
			this._vertical_value_per_grid = this._parent._vertical_value_per_grid;
			this._pixel_per_grid = this._pixel_per_unit * this._unit_per_grid;
			this._vertical_pixel_per_value = (double)this._pixel_per_grid / (double)this._vertical_value_per_grid;
			repaint();
		}
		
		public void UpdateColorPreference() {
			this._color_grid = this._parent._color_grid;
			this._color_waveform = this._parent._color_waveform;
			this.setBackground(this._parent._color_meter_background);
			repaint();
		}
		
		/**
		 * Get the PrefrredSize
		 */
		public Dimension getPreferredSize() {
			return _parent._preferredSize;
		}
		
		/**
		 * Refresh the meter
		 *
		 */
		public void Refresh() {
			this._offset += this._pixel_per_unit;
			
			if( this._offset >= this._pixel_per_grid )
				this._offset = 0;
		
			repaint();
		}
		
		/**
		 * Component paint
		 */
		protected void paintComponent(Graphics g) {
		//Paint background if we're opaque.
			if (isOpaque()) {
				g.setColor(getBackground());
				g.fillRect(0, 0, getWidth(), getHeight());
			}
		
			Dimension dim = getSize();
			
			//Paint 20x20 grid.
			g.setColor(this._color_grid);
			drawGrid(g, this._pixel_per_grid, _offset, dim);
			
			g.setColor(this._color_waveform);
			drawWaveform(g, dim);
		}
		
		/**
		 * Draw the grid 
		 * @param g
		 * @param gridSpace
		 * @param x_offset
		 * @param dim
		 */
		//Draws a 20x20 grid using the current color.
		private void drawGrid(Graphics g, int gridSpace, int x_offset, Dimension dim ) {
			
			int firstX = 0;
			int firstY = 0;
			int lastX = dim.width;
			int lastY = dim.height;
					
			//Draw vertical lines.
			int x = lastX - x_offset;
			while (x >= firstX) {
				g.drawLine(x, firstY, x, lastY);
				x -= gridSpace;
			}
			
			//Draw horizontal lines.
			int y = lastY;
			while (y >= firstY) {
				g.drawLine(firstX, y, lastX, y);
				y -= gridSpace;
			}
		}
		
		/**
		 * Draw the waveform
		 * @param g
		 * @param dim
		 */
		private boolean hasPreviousValue = false;
		
		private void drawWaveform(Graphics g, Dimension dim ) {
			
			int oldmax = this._parent.Get_Max_Vertical_Value();
			int max = 0;
			
			int point_start_x = 0;
			int point_start_y = 0;
			int point_end_x = 0;
			int point_end_y = 0;
			
			//coordinate
			int lastX = dim.width;
			int lastY = dim.height;			
			
			//initialize to read the buffer
			this._parent._buffer.initRead();
			
			//check empty buffer
			if( !this._parent._buffer.hasPrevious())
			{
				return;
			}
			
			boolean hasPreviousValue = true;
			boolean isDraw = true;
			int countDown = this._parent._numOfDelay;
			
			boolean hasValue = this._parent._buffer.HasValue();
			
			int value = (int)this._parent._buffer.GetValue(); 

			if(hasValue)
				max = value;
			
			this._parent.fireRefreshEvent( new RefreshEvent(this._parent,hasValue,value));
			
			
			//todo: if there is no value
			point_start_y = lastY - (int)( (double)(hasValue ? (int)value : 0) * (double)_vertical_pixel_per_value );
			point_start_x = lastX;
			
			point_end_y = point_start_y;
			point_end_x = lastX;
			
			hasPreviousValue = hasValue;
			isDraw = hasValue;
			
			if(isDraw)
				g.drawLine( point_start_x, point_start_y, point_end_x, point_end_y );
			
			int previousValue = value;

						
			while(this._parent._buffer.hasPrevious()) {
				point_start_x = point_end_x;
				point_start_y = point_end_y;
				
				if(this._parent._buffer.HasValue() ) {
					countDown = this._parent._numOfDelay;
					value = (int)this._parent._buffer.GetValue();
					previousValue = value;
					isDraw = hasPreviousValue;
					hasPreviousValue = true;
				}
				else {

					if(countDown>0) {
						value = previousValue;
						countDown--;						
					}
					else {
						hasPreviousValue = false;
						isDraw = false;
					}
				}
				
				//value = (this._parent._buffer.HasValue() ?  (int)this._parent._buffer.GetValue()  : 0);
				
				if(max < value)
					max = value;
				
				point_end_y = lastY - (int)( (double)value * (double)_vertical_pixel_per_value);
				point_end_x -= this._parent._pixel_per_unit;
				
				if(isDraw) {
					g.drawLine( point_start_x, point_start_y, point_end_x, point_end_y );
				}
				isDraw = true;
			}
			
			if((max > oldmax) && this._parent.isAutoResizeToMax()) {
				this._parent.Set_Max_Vertical_Value(max + 10);
			}
			
			
		}
		
		//********************************************************
		//*		Render the waveform image
		//********************************************************
		public RenderedImage RenderWaveformImage() {
	        
			try
			{
				
				int label_height = 50;
				int label_fir_start = 10;
				int label_sec_start = 220;
				int label_first_height_gap = 20;
				int text_height = 10;
				
				Dimension dim = getSize();
//				 Create a buffered image in which to draw
		        BufferedImage bufferedImage = new BufferedImage(dim.width, dim.height + label_height, BufferedImage.TYPE_INT_RGB);
		    
		        		        
		        // Create a graphics contents on the buffered image
		        Graphics2D g2d = bufferedImage.createGraphics();
				
				//*****************************************************************
		        //Paint 20x20 grid.
		        //*****************************************************************
		        g2d.setColor(this._parent._color_meter_background);
				g2d.fillRect(0, 0, dim.width, dim.height + label_height);
		        
				g2d.setColor(this._color_grid);
				
								
				int firstX = 0;
				int firstY = 0;
				int lastX = dim.width;
				int lastY = dim.height;
				int gridSpace = this._pixel_per_grid;
				int x_offset = _offset;
						
				//Draw vertical lines.
				int x = lastX - x_offset;
				while (x >= firstX) {
					g2d.drawLine(x, firstY, x, lastY);
					x -= gridSpace;
				}
				
				//Draw horizontal lines.
				int y = lastY;
				while (y >= firstY) {
					g2d.drawLine(firstX, y, lastX, y);
					y -= gridSpace;
				}
				
				int width = 800;
		        int height = 600;
		    
		        
		        //*****************************************************************
		        //Paint waveform
		        //*****************************************************************
		        int oldmax = this._parent.Get_Max_Vertical_Value();
				int max = 0;
				
				int point_start_x = 0;
				int point_start_y = 0;
				int point_end_x = 0;
				int point_end_y = 0;
				
				//coordinate
				lastX = dim.width;
				lastY = dim.height;		
				
				
				g2d.setColor(this._color_waveform);
				//initialize to read the buffer
				this._parent._buffer.initRead();
				
				//check empty buffer
				if( !this._parent._buffer.hasPrevious())
				{
					return null;
				}
				
				boolean hasPreviousValue = true;
				boolean isDraw = true;
				int countDown = this._parent._numOfDelay;
				
				boolean hasValue = this._parent._buffer.HasValue();
				
				int value = (int)this._parent._buffer.GetValue(); 

				if(hasValue)
					max = value;
				
				//todo: if there is no value
				point_start_y = lastY - (int)( (double)(hasValue ? (int)value : 0) * (double)_vertical_pixel_per_value );
				point_start_x = lastX;
				
				point_end_y = point_start_y;
				point_end_x = lastX;
				
				hasPreviousValue = hasValue;
				isDraw = hasValue;
				
				if(isDraw)
					g2d.drawLine( point_start_x, point_start_y, point_end_x, point_end_y );
				
				int previousValue = value;
							
				while(this._parent._buffer.hasPrevious()) {
					point_start_x = point_end_x;
					point_start_y = point_end_y;
					
					if(this._parent._buffer.HasValue() ) {
						countDown = this._parent._numOfDelay;
						value = (int)this._parent._buffer.GetValue();
						previousValue = value;
						isDraw = hasPreviousValue;
						hasPreviousValue = true;
					}
					else {
						if(countDown>0) {
							value = previousValue;
							countDown--;						
						}
						else {
							hasPreviousValue = false;
							isDraw = false;
						}
					}
					
					//value = (this._parent._buffer.HasValue() ?  (int)this._parent._buffer.GetValue()  : 0);
					
					if(max < value)
						max = value;
					
					point_end_y = lastY - (int)( (double)value * (double)_vertical_pixel_per_value);
					point_end_x -= this._parent._pixel_per_unit;
					
					if(isDraw) {
						g2d.drawLine( point_start_x, point_start_y, point_end_x, point_end_y );
					}
					isDraw = true;
				}
				
				int curHeight = dim.height + label_first_height_gap;
				
				g2d.drawString("Max grid value = " + _parent._max_vertical_value_meterArea, label_fir_start, curHeight);
				g2d.drawString("Refresh Rate = " + _parent._refresh_rate, label_sec_start, curHeight);
				
				curHeight += label_first_height_gap;
				g2d.drawString("Min grid value = 0", label_fir_start, curHeight);
				g2d.drawString("Grid scale = " + _parent.Get_Scale_VerticalValuePerGrid(), label_sec_start, curHeight);
				
	
		        // Graphics context no longer needed so dispose it
		        g2d.dispose();
		        
		        return bufferedImage;
			}
			catch(Exception ex) {
				ex.printStackTrace();
				return null;
			}
	        
	    }
	}	
}
