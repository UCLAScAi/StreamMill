/*
 * Created on Mar 12, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import java.awt.*;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.io.File;

import javax.imageio.ImageIO;
import javax.swing.JComponent;


import atlas.monitoring.library.MColorMgr;
import atlas.monitoring.library.MGlobal;
import atlas.monitoring.library.MStreamingBuffer;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentClusterHistogramPaint extends MComponent {

	private MStreamingBuffer _mainBuffer;
	private MClusterElementBuffer _fadeInBuffer;
	private MClusterElementBuffer _fadeOutBuffer;
	private ClusterArea _clusterArea;
	private HistogramArea _histogramArea;
	private int _bufferSize = 1000;
	private Thread _dispatchThread;
	private Thread _fadingThread;
	private boolean _isDrawOK = false;
	private boolean _isAutoCaptureCluster = false;
	private boolean _isAutoCaptureHistogram = false;
	private String _capFileNameCluster;
	private String _capFileNameHistogram;
	private int _capCounter = 0;
	
	
	public static final int FADING_SPEED_VERTFAST = 50;
	public static final int FADING_SPEED_FAST = 150;
	public static final int FADING_SPEED_MIDDLE = 500;
	public static final int FADING_SPEED_SLOW = 1000;
	public static final int[] FADING_SPEEDS = {FADING_SPEED_SLOW,FADING_SPEED_MIDDLE,FADING_SPEED_FAST,FADING_SPEED_VERTFAST};
	
	private int _fadingSpeed = 0;
	private boolean _displayOutliner = true;
	private boolean _displayAxis = true;
	
	
	public MComponentClusterHistogramPaint(Object[] arguments) {
		super("MComponentClusterHistogramPaint");
		
		int radius = 0;
		
		try {
			_bufferSize = ((Integer)arguments[0]).intValue();
		}
		catch( Exception ex) {
			_bufferSize = 1000;
		}
		
		try {
			radius = ((Integer)arguments[1]).intValue();
		}
		catch( Exception ex) {}
		
		_fadingSpeed = FADING_SPEED_MIDDLE;
		
		try {
			int speed = ((Integer)arguments[2]).intValue();
			
			if(0<=speed&&speed<FADING_SPEEDS.length) {
				_fadingSpeed = FADING_SPEEDS[speed];
			}
		}
		catch(Exception e) {
			
		}
		
		//cluster go first
		try {
			
			if(arguments[3]!=null) {
				_capFileNameCluster = (String)arguments[3];
				_isAutoCaptureCluster = (_capFileNameCluster.length() > 0);
			}
		}
		catch(Exception ex) {}
		
		//then is histogram
		try {
			if(arguments[4]!=null) {
				_capFileNameHistogram = (String)arguments[4];			
				_isAutoCaptureHistogram = (_capFileNameHistogram.length() > 0);
			}
		}
		catch(Exception ex) {}
		
		
		_mainBuffer = new MStreamingBuffer(_bufferSize << 1);
		_clusterArea = new ClusterArea(radius);
		_histogramArea = new HistogramArea();
		_dispatchThread = new Thread( new DispatchingThread() );
		_fadingThread = new Thread( new FadingThread() );
		_dispatchThread.start();
		_fadingThread.start();
	}
	
	/**
	 * 	Good habit to release the resource 
	 */
	public void CleanUp() {
		if(_dispatchThread!=null)
			_dispatchThread.stop();
		if(_fadingThread!=null)
			_fadingThread.stop();
	}

	/**
	 * Set up the fading speed
	 * @param speed
	 */ 
	public void SetFadingSpeed(int speed) {
		switch(speed) {
			case FADING_SPEED_VERTFAST:		_fadingSpeed = FADING_SPEED_VERTFAST; 	break;
			case FADING_SPEED_FAST:			_fadingSpeed = FADING_SPEED_FAST; 		break;
			case FADING_SPEED_MIDDLE:		_fadingSpeed = FADING_SPEED_MIDDLE; 	break;
			case FADING_SPEED_SLOW:			_fadingSpeed = FADING_SPEED_SLOW; 		break;
			default:		
		}
	}
	
	/**
	 * Should I display the outliner
	 * @param choice
	 */
	public void SetDisplayOutliner(boolean choice) {
		_displayOutliner = choice;
		_clusterArea.repaint();
	}
	
	public void SetDisplayAxis(boolean choice) {
		_displayAxis = choice;
		_clusterArea.repaint();
	}
	
	/**
	 * Add the new cluster element value into the buffer
	 * @param x
	 * @param y
	 * @param clusterId
	 */
	public void AddClusterElementValue(int x, int y, int clusterId, int slideWindowIdx) {
		_mainBuffer.Add( new MClusterElement(x,y,clusterId,MGlobal.FADE_IDX_MIN, slideWindowIdx));
	}
	
	/**
	 * No use
	 */
	public void AddValue(String value) {}
	
	/**
	 * Take the snapshot of current cluster
	 * @return
	 */
	public RenderedImage SnapShotCluster(){
        BufferedImage bufferedImage = new BufferedImage(_clusterArea.getWidth(), _clusterArea.getHeight(), BufferedImage.TYPE_INT_RGB);
        
		// Create a graphics contents on the buffered image
		Graphics2D g2d = bufferedImage.createGraphics();
		
		_clusterArea.paint(g2d);
		
		return bufferedImage;
	}
	
	/**
	 * Take the snapshot of current histogram
	 * @return
	 */
	public RenderedImage SnapShotHistogram(){
        BufferedImage bufferedImage = new BufferedImage(_histogramArea.getWidth(), _histogramArea.getHeight(), BufferedImage.TYPE_INT_RGB);
	        		        
        // Create a graphics contents on the buffered image
        Graphics2D g2d = bufferedImage.createGraphics();
		
        _histogramArea.paint(g2d);
        
		return bufferedImage;
	}

	//************************************
	// Capture Thread
	//************************************	
	private class CaptureThread implements Runnable {
		
		private String _name;
		private JComponent _com;
		
		public CaptureThread(String filename, JComponent com) {
			_name = filename;
			_com = com;
		}
		
		public void run() {
			
			try
			{
				BufferedImage bufferedImage = new BufferedImage(_com.getWidth(), _com.getHeight(), BufferedImage.TYPE_INT_RGB);
		        
				// Create a graphics contents on the buffered image
				Graphics2D g2d = bufferedImage.createGraphics();
				
				_com.paint(g2d);
				
				File file = new File(_name);
		        ImageIO.write(bufferedImage, "jpg", file);
			}
			catch(Exception ex) {}
		}
	}
	
	
	//************************************
	// Dispatch Thread
	//************************************	

	/**
	 * 
	 * @author treetree
	 *
	 * Dispatch the cluster elements to fade in buffer and also do the switch
	 */
	private class DispatchingThread implements Runnable {
		
		public void run() {
			while(true) {
				MClusterElement element = (MClusterElement)_mainBuffer.Get();
				
				//TODO: this is not correct
				// we have to chceck the slideWindowIdx , not the id
				if(_fadeInBuffer==null || _fadeInBuffer.GetClusterIndex() < element.GetClusterIndex()) {
					
					//System.out.println("do switch");
					
					setDrawStatus(false);
					
					_fadeOutBuffer = null;
					_fadeOutBuffer = _fadeInBuffer;
					_fadeInBuffer = new MClusterElementBuffer(element.GetClusterIndex(), _bufferSize);
					_fadeInBuffer.AddElement(element);
					
					setDrawStatus(true);
				}
				else if(element.GetClusterIndex() == _fadeInBuffer.GetClusterIndex()){
					
					//System.out.println("pure add");
					_fadeInBuffer.AddElement(element);
					setDrawStatus(true);
				}
				
				//_clusterArea.repaint();
			}
		}
	}
	
	//************************************
	// Drawing Thread
	//************************************	
	private class FadingThread implements Runnable {
		
		private boolean _shouldIwait = true;
		
		public void run() {
			
			boolean isAllFadedIn = false;
			boolean isAllFadedOut = false;
			boolean ispaint = false;
			
			while(true) {
				if(_shouldIwait) {
					//System.out.print("wait for draw...");
					waitForDraw();
					//System.out.println("..done");
				}
				
				if(_fadeInBuffer!=null) {
					_fadeInBuffer.IncFadeIdxToAllElement();
					_shouldIwait = _fadeInBuffer.IsAllFadedIn();
				}
			
				if(_fadeOutBuffer!=null) {
					_fadeOutBuffer.DecFadeIdxToAllElement();
					_shouldIwait &= _fadeOutBuffer.IsAllFadeout();

				}
	
				_clusterArea.repaint();
				_histogramArea.repaint();
				
				/*
				 *Auto caputre function
				 */
				if(_isAutoCaptureCluster) {
					new Thread(new CaptureThread(_capFileNameCluster + _capCounter + ".jpg", _clusterArea )).start();
				}
				
				if(_isAutoCaptureHistogram) {
					new Thread(new CaptureThread(_capFileNameHistogram + _capCounter + ".jpg", _histogramArea )).start();
				}
				
				_capCounter++;

				
				try {
					Thread.sleep(_fadingSpeed);
				}
				catch(Exception ex) {
					
				}
			}
		}
	}
	
	private synchronized void waitForDraw() {
		while(!_isDrawOK) {
			try {
				wait();
				
			}
			catch(InterruptedException ex) {
				/**
				 * TODO: remove
				 */
				ex.printStackTrace();
			}
		}
		_isDrawOK = false;
	}
	
	private synchronized void setDrawStatus(boolean status) {
		_isDrawOK = status;
		
		if(_isDrawOK)
			notify();
	}
	
	//************************************
	// Get the component and paint
	//************************************
	public JComponent GetClusterArea() {
		return _clusterArea;
	}
	
	public JComponent GetHistogramArea() {
		return _histogramArea;
	}
	
	/**
	 *
	 * @author treetree
	 *
	 * Historgram area
	 */
	private class HistogramArea extends JComponent {
		
		private final Color COLOR_BACKGROUND = Color.WHITE;
		private final Color COLOR_TEXT = Color.DARK_GRAY;
		private final Font FONT_BOLD = new Font("Arial",Font.BOLD,10);
		private final Font FONT_PLAIN = new Font("Arial",Font.PLAIN,10);
		
		private int GAP_TOP_HEIGHT = 20;
		private int GAP_SIDE_WIDTH = 10;
		private int BIN_SIDE_WIDTH = 10;
		
		private int LINE_HEIGHT = 30;
		private int AXIS_HEIGHT = 15;
		private int TEXT_HEIGHT = 10;
		private int TEXT_START_X = 50;
		private int LABEL_HEIGHT = 5;
		
		private int _abs_line_height;
		private int _abs_axis_height;
		private int _abs_text_height;
		private int _abs_text_start_x;
		
		private int _bin_cur_x;
		private double _bin_width;
		private double _bin_width_two;
		private double _pixel_per_unit;
		private Dimension _dim;
		private Dimension _prevDim = new Dimension(0,0);
		
		private int _numOfBins;
		private int _maxBinsValue;
		private int _maxBinsIndex;
		
		private int counter = 0;
		
		
		protected void paintComponent(Graphics g) {
			
			if (isOpaque()) {
				g.setColor(Color.GRAY);
				g.fillRect(0, 0, getWidth(), getHeight());
			}
			else {
				g.setColor(COLOR_BACKGROUND);
				g.fillRect(0, 0, getWidth(), getHeight());
			}
			
			g.setFont(FONT_PLAIN);
			
//			System.out.println("repaint");
			
			//********************************
			//* If the fadeinbuffer is not ready
			//********************************
			if(_fadeInBuffer==null) {
				g.setColor(Color.WHITE);
				g.fillRect(0, 0, getWidth(), getHeight());
				g.setColor(Color.GRAY);
				g.drawLine(GAP_SIDE_WIDTH,getHeight() - LINE_HEIGHT,(getWidth()-GAP_SIDE_WIDTH),getHeight() - LINE_HEIGHT);
				g.drawString("Waiting for data...", TEXT_START_X, getHeight() - TEXT_HEIGHT);
				return;
			}
			
			//System.out.println("paint histogram = " + counter++);
			
			handleScale();
			
			g.setColor(Color.BLACK);
			g.drawLine(GAP_SIDE_WIDTH,_abs_line_height,(_dim.width-GAP_SIDE_WIDTH),_abs_line_height);
			
			//********************************
			//* Draw the histogram
			//********************************
			int[] bins;
			int value;
			int height;
			int y;
			
			Color color;
			
			bins = _fadeInBuffer.GetBins();
			//_maxBinsIndex
			
			//***********************
			//draw the outlier even if it is zero
			//***********************
			value = bins[0];

			height = (int)((double)value * _pixel_per_unit);
			
			y = _abs_line_height - height;
			
			g.setColor(MColorMgr.GetInstance().GetColor(0,MGlobal.FADE_IDX_MAX));
			g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
			g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
			g.drawString("0",(int)_bin_cur_x,_abs_axis_height);

			_bin_cur_x += (int)_bin_width_two;
			
			//***********************
			//* draw the rest
			//***********************
			
			for( int idx=1; idx<=_fadeInBuffer.GetMaxBinIndex(); idx++) {
				
				value = bins[idx];

				if(value>0) {
					
					height = (int)((double)value * _pixel_per_unit);
					
					y = _abs_line_height - height;
					
					g.setColor(MColorMgr.GetInstance().GetColor(idx,MGlobal.FADE_IDX_MAX));
					g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
					g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
					g.drawString(idx+"",(int)_bin_cur_x,_abs_axis_height);
					
					_bin_cur_x += (int)_bin_width_two;
				}
			}
			bins = null;
		}
		
		private void handleScale() {
			
			int in_height;
			int in_width;
			
			_dim = getSize();
			
//			if((_prevDim.getWidth()== _dim.getWidth()) && (_prevDim.getHeight() == _dim.getHeight())) 
//				return;
			
			in_height = _dim.height - LINE_HEIGHT - GAP_TOP_HEIGHT;
			
			_bin_cur_x = GAP_SIDE_WIDTH + BIN_SIDE_WIDTH;
			in_width = _dim.width - (_bin_cur_x << 1);
			
			_maxBinsValue = _fadeInBuffer.GetMaxBinValue();
			_numOfBins = _fadeInBuffer.GetNumActiveBin() ;
			_maxBinsIndex = _fadeInBuffer.GetMaxBinIndex();
			
			
			/**
			 * since we consider outline as active bin even it is zero
			 */
			_numOfBins += _fadeInBuffer.GetBins()[0] == 0 ? 1 : 0;
			
//			System.out.println("Number of bins = " + _numOfBins);
			
			_bin_width = ((double)in_width / (double)(( _numOfBins << 1) - 1));
			_bin_width_two = _bin_width * 2;
			
//			System.out.println(String.valueOf((double)(in_width / (( _numOfBins << 1) - 1))));
//			System.out.println(_bin_width);
//			System.out.println(_bin_width_two);
//			System.out.println(_numOfBins + " bins");
			
			_pixel_per_unit = (double)(in_height-LABEL_HEIGHT) / (double)_maxBinsValue;
			
			_abs_line_height = _dim.height - LINE_HEIGHT;
			_abs_axis_height = _dim.height - AXIS_HEIGHT;
			_abs_text_height = _dim.height - TEXT_HEIGHT;
			
			//save for later use
			_prevDim = _dim;
		}
	}
	
	/**
	 * 
	 * @author treetree
	 *
	 * Cluster Area
	 */
	private class ClusterArea extends JComponent {
		
		private final int POINT_RADIUS = 5;
		private final int LABEL_HEIGHT = 25;
		private final int LABEL_SENTENCE_HEIGHT = 8;
		private final int LABEL_START_WIDTH = 10;
		private final int LABEL_GAP_WIDTH = 20;
		private final int LABEL_SEPERATE_WIDTH = 120;
		private final int LABEL_BETWEEN_WIDTH = 180;
		
		private final Color COLOR_BACKGROUND = Color.WHITE;
		private final Color COLOR_TEXT = Color.DARK_GRAY;
		private final Font FONT_BOLD = new Font("Arial",Font.BOLD,12);
		private final Font FONT_PLAIN = new Font("Arial",Font.PLAIN,12);
		
		private int _old_max_x;
		private int _old_min_x;
		private int _old_max_y;
		private int _old_min_y;
		
		private int _cur_max_x;
		private int _cur_min_x;
		private int _cur_max_y;
		private int _cur_min_y;
		private double _ex_gap_ratio = 0.01;
		
		private int _tmp;
		private int _range_x;
		private int _range_y;
		private double _pixel_per_unit_x;
		private double _pixel_per_unit_y;
		
		private int _rang_radius; 
		
		public ClusterArea( int ring_radius ) {
			_rang_radius = ring_radius;
		}
		
		protected void paintComponent(Graphics g) {
			
			if (isOpaque()) {
				g.setColor(Color.GRAY);
				g.fillRect(0, 0, getWidth(), getHeight());
			}
			else {
				g.setColor(COLOR_BACKGROUND);
				g.fillRect(0, 0, getWidth(), getHeight());
			}
			
			//********************************************************
			//* Handle the scale issue
			//********************************************************
			Dimension dim = getSize();
			
			int height = dim.height - LABEL_HEIGHT;
			int width = dim.width;
			int diameter;
			int tmph = dim.height - LABEL_SENTENCE_HEIGHT;
			int tmpw = LABEL_START_WIDTH;
			
			
			g.setFont(FONT_PLAIN);
			if(_fadeInBuffer==null&&_fadeOutBuffer==null) {
				g.setColor(Color.WHITE);
				g.fillRect(0, 0, getWidth(), getHeight());
				g.setColor(Color.GRAY);
				g.drawLine(0,height,width,height);
				g.drawString("Waiting for data...", tmpw, tmph);
				return;
			}
			
			g.setColor(COLOR_TEXT);
			g.drawLine(0,height,width,height);
			
			fixMaxMin();
			_pixel_per_unit_y = (double)height / (double)_range_y;
			_pixel_per_unit_x = (double)width / (double)_range_x;
		
			
			//********************************************************
			//* set up the diameter of the point
			//********************************************************
			
			diameter = (int)_pixel_per_unit_y;
			
			if(_pixel_per_unit_x < diameter)
				diameter = (int)_pixel_per_unit_x;
			
			if(diameter<POINT_RADIUS)
				diameter = POINT_RADIUS;
			
			/**
			 * x_cor = ( x + (-1)*_min_x ) * _pixel_per_unit_x
			 * x_cor = ( x + neg_min_x ) * _pixel_per_unit_x
			 * 
			 * y_cor = [ (_range_y - (-1)*min_y) - y ] *  _pixel_per_unit_y
			 * y_cor = [ (sub_y) - y ] *  _pixel_per_unit_y
			 */
			
			int neg_min_x = (-1) * _cur_min_x;
			int sub_y = _range_y - ((-1) * _cur_min_y);
			
			//*****************************************************
			//*	Start to draw (Text)
			//*****************************************************
			g.setFont(FONT_BOLD);
			g.drawString("x :", tmpw, tmph);
			
			g.setFont(FONT_PLAIN);
			tmpw += LABEL_GAP_WIDTH;
			g.drawString("min = " + _cur_min_x, tmpw, tmph );
			
			tmpw += LABEL_SEPERATE_WIDTH;
			g.drawString("max = " + _cur_max_x, tmpw, tmph );
						
			tmpw += LABEL_BETWEEN_WIDTH;
			g.setFont(FONT_BOLD);
			g.drawString("y :", tmpw, tmph);
			
			g.setFont(FONT_PLAIN);
			tmpw += LABEL_GAP_WIDTH;
			g.drawString("min = " + _cur_min_y, tmpw, tmph );
			
			tmpw += LABEL_SEPERATE_WIDTH;
			g.drawString("max = " + _cur_max_y, tmpw, tmph );
			
			
			
			//*****************************************************
			//*	Start to draw fadeout
			//*****************************************************
			MClusterElement element = null;
			int x_coor;
			int y_coor;
			int x_draw_coor;
			int y_draw_coor;
			int id;

			int radius = diameter >> 1;
			int ring_x_radius = (int)(_rang_radius * _pixel_per_unit_x);
			int ring_y_radisu= (int)(_rang_radius* _pixel_per_unit_y);
			int ring_x_diameter = ring_x_radius << 1;
			int ring_y_diameter= ring_y_radisu << 1;
			
			if(_fadeOutBuffer!=null && !_fadeOutBuffer.IsAllFadeout()) {
				
				for( int i=0; i<_fadeOutBuffer.GetNumOfElements(); i++) {
					
					element = _fadeOutBuffer.GetElementAt(i);
					
					id = element.GetClusterId();
					
					if(id==0 && !_displayOutliner)
						continue;
					
					
					
					x_coor = (int) ((double)(element.GetX() + neg_min_x) * _pixel_per_unit_x);
					y_coor = (int) ((double)(sub_y - element.GetY()) * _pixel_per_unit_y);
					
					if(_rang_radius>0) {
						x_draw_coor = x_coor - ring_x_radius;
						y_draw_coor = y_coor - ring_y_radisu;
						
						g.setColor(MColorMgr.GetInstance().GetColor(0,element.GetFadeIdx()));
						g.drawOval(x_draw_coor,y_draw_coor,ring_x_diameter,ring_y_diameter);
					}
					
					g.setColor(MColorMgr.GetInstance().GetColor(id,element.GetFadeIdx()));
					
					x_draw_coor = x_coor - radius;
					y_draw_coor = y_coor - radius;
					
					//System.out.println(element.GetX() + "," + element.GetY());
					
					g.fillOval(x_draw_coor,y_draw_coor,diameter,diameter);
					
					
				}
			}

			//*****************************************************
			//*	Start to draw fadeIn
			//*****************************************************
			
			if(_fadeInBuffer!=null) {
				
				//System.out.println("print cluter = " + counter++);
				
				for( int i=0; i<_fadeInBuffer.GetNumOfElements(); i++) {
					
					element = _fadeInBuffer.GetElementAt(i);
					
					id = element.GetClusterId();
					
					if(id==0 && !_displayOutliner)
						continue;
					
					
					
					x_coor = (int) ((double)(element.GetX() + neg_min_x) * _pixel_per_unit_x);
					y_coor = (int) ((double)(sub_y - element.GetY()) * _pixel_per_unit_y);
					
					if(_rang_radius>0) {
						x_draw_coor = x_coor - ring_x_radius;
						y_draw_coor = y_coor - ring_y_radisu;
						
						g.setColor(MColorMgr.GetInstance().GetColor(0,element.GetFadeIdx()));
						g.drawOval(x_draw_coor,y_draw_coor,ring_x_diameter,ring_y_diameter);
					}
					
					g.setColor(MColorMgr.GetInstance().GetColor(id,element.GetFadeIdx()));
					x_draw_coor = x_coor - radius;
					y_draw_coor = y_coor - radius;
					
					g.fillOval(x_draw_coor,y_draw_coor,diameter,diameter);
					
					
				}
			}

			//*****************************************************
			//*	Start to draw axis
			//*****************************************************
			if(_displayAxis) {
				
				g.setColor(Color.LIGHT_GRAY);
				
				if(_cur_min_x < 0 && 0 < _cur_max_x) {
					x_coor = (int) ((double)(neg_min_x) * _pixel_per_unit_x);
					g.drawLine(x_coor,height,x_coor,0);
				}
				
				if(_cur_min_y < 0 && 0 < _cur_max_y ) {
					y_coor = (int) ((double)(sub_y - 0) * _pixel_per_unit_y);
					g.drawLine(0,y_coor,width,y_coor);
				}
			}
		}
		
		/**
		 * find the max and min of (x,y) for the graph
		 */
		private void fixMaxMin() {
			
			//*****************************
			//* find the ragne of graph
			//*****************************
			if(_fadeOutBuffer!=null ) { //&& !_fadeOutBuffer.IsAllFadeout()) {		//this way looks better
				
				_cur_max_x = _fadeOutBuffer.GetMaxX();
				_cur_max_y = _fadeOutBuffer.GetMaxY();
				_cur_min_x = _fadeOutBuffer.GetMinX();
				_cur_min_y = _fadeOutBuffer.GetMinY();
				
				_tmp = _fadeInBuffer.GetMaxX();
				_cur_max_x = _tmp > _cur_max_x ? _tmp : _cur_max_x;
				
				_tmp = _fadeInBuffer.GetMaxY();
				_cur_max_y = _tmp > _cur_max_y ? _tmp : _cur_max_y;
				
				_tmp = _fadeInBuffer.GetMinX();
				_cur_min_x = _tmp < _cur_min_x ? _tmp : _cur_min_x;
				
				_tmp = _fadeInBuffer.GetMinY();
				_cur_min_y = _tmp < _cur_min_y ? _tmp : _cur_min_y;
			}
			else {
				_cur_max_x = _fadeInBuffer.GetMaxX();
				_cur_max_y = _fadeInBuffer.GetMaxY();
				_cur_min_x = _fadeInBuffer.GetMinX();
				_cur_min_y = _fadeInBuffer.GetMinY();
			}

			
			//the graph can be a line but no a point
			if( (_cur_max_x == _cur_min_x) && (_cur_max_y == _cur_min_y))
				return;
			
			//x-coordinate change
			if((_old_max_x != _cur_max_x)||(_old_min_x!=_cur_min_x)) {
				_range_x = _cur_max_x - _cur_min_x;
				
				_tmp = (int)Math.ceil(_range_x * _ex_gap_ratio);
				
				_cur_max_x += _tmp;
				_cur_min_x -= _tmp;
				
				_range_x = _cur_max_x - _cur_min_x;
				
				_old_max_x = _cur_max_x;
				_old_min_x = _cur_min_x;
			}
			
			//y-coordinate change
			if((_old_max_y != _cur_max_y)||(_old_min_y!=_cur_min_y)) {
				_range_y = _cur_max_y - _cur_min_y;
				
				_tmp = (int)Math.ceil(_range_y * _ex_gap_ratio);
				
				_cur_max_y += _tmp;
				_cur_min_y -= _tmp;
				
				_range_y = _cur_max_y - _cur_min_y;
				
				_old_max_y = _cur_max_y;
				_old_min_y = _cur_min_y;
			}			
		}
	}
}
