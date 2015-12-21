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


import atlas.monitoring.library.MBinCounter;
import atlas.monitoring.library.MColorMgr;
import atlas.monitoring.library.MGlobal;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentHistogramHorizontal extends MComponentHistogram {

	private HistogramArea _histogramArea;
	private int _bufferSize = 100;
	private boolean _isAutoCaptureHistogram = false;
	private String _capFileNameHistogram;
	private MBinCounter _binCnt;
	private boolean _isDrawOK = false;
	private boolean _isAutoCapture = false;
	private Thread _drawingThread;
	private String _capturePath;
	private int _capCounter = 0;
	private int _curWindowIdx = 0;

	public MComponentHistogramHorizontal(Object[] arguments) {
		super("MComponentHorizontalHistogram");
		
		try {
			if(arguments!=null && arguments[0]!=null) {
				_isAutoCapture = true;
				_capturePath = (String)arguments[0];
			}
		}
		catch(Exception ex) {
			_isAutoCapture = false;
		}
		
		_histogramArea = new HistogramArea();
		_drawingThread = new Thread( new DrawingThread() );
		_drawingThread.start();
	}
	
	/**
	 * 	Good habit to release the resource 
	 */
	public void CleanUp() {
		if(_drawingThread!=null)
			_drawingThread.stop();
	}

	public void AddValue(int x, int y, int bin, int windowIdx) {
		if(_binCnt == null || _curWindowIdx<windowIdx) {
			
			if(_binCnt != null && _isAutoCapture) {
				new Thread(new CaptureThread(_capturePath + (_capCounter++) + ".jpg", _histogramArea )).start();
			}
			
			_binCnt = new MBinCounter(_bufferSize);
			_binCnt.SetDynamicResize(true);
			_curWindowIdx = windowIdx;
			
			
		}
		_binCnt.Inc(bin);
		setDrawStatus(true);
	}
	
  public void AddValue(int cnt, int bin, int windowIdx) {
    if(_binCnt == null || _curWindowIdx<windowIdx) {
      
      if(_binCnt != null && _isAutoCapture) {
        new Thread(new CaptureThread(_capturePath + (_capCounter++) + ".jpg", _histogramArea )).start();
      }
      
      _binCnt = new MBinCounter(_bufferSize);
      _binCnt.SetDynamicResize(true);
      _curWindowIdx = windowIdx;
      
      
    }
    _binCnt.add(bin, cnt);
    setDrawStatus(true);
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

	public JComponent GetHistogramArea() {
		return _histogramArea;
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
	// Drawing Thread
	//************************************	
	private class DrawingThread implements Runnable {
		
		private boolean _shouldIwait = true;
		
		public void run() {
			
			boolean isAllFadedIn = false;
			boolean isAllFadedOut = false;
			boolean ispaint = false;
			
			while(true) {
				waitForDraw();
				
				_histogramArea.repaint();
				
				
				
				try {
					Thread.sleep(100);
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
	
	/**
	 *
	 * @author treetree
	 *
	 * Historgram area
	 */
	protected class HistogramArea extends JComponent {
		
		protected final Color COLOR_BACKGROUND = Color.WHITE;
		protected final Color COLOR_TEXT = Color.DARK_GRAY;
		protected final Font FONT_BOLD = new Font("Arial",Font.BOLD,12);
		protected final Font FONT_PLAIN = new Font("Arial",Font.PLAIN,12);
		
		//*******************************
		//NEW
		//*******************************
		protected int GAP_RIGHT_WIDTH = 20;
		protected int GAP_TOP_DIS = 10;
		protected int BIN_TOP_DIS = 10;
		
		protected int TEXT_HEIGHT = 10;
		protected int TEXT_LEFT_WIDTH = 5;
		protected int LINE_LEFT_WIDTH = 30;
		
		protected int LABEL_WIDTH = 10;
		protected int LABEL_WIDTH_GAP = 2;
		
		
		protected double _pixel_per_unit;
		protected Dimension _dim;
		protected Dimension _prevDim = new Dimension(0,0);
		
		protected int _numOfBins;
		protected int _maxBinsValue;
		protected int _maxBinsIndex;
		
		protected int counter = 0;
		
		protected int _bin_curr_y;
		protected double _bin_height;
		protected double _bin_height_two;
		protected int _bin_height_half;
		
		protected void handleScale() {
			
			int in_height;
			int in_width;
			
			_dim = getSize();
			
			in_width = _dim.width - LINE_LEFT_WIDTH - GAP_RIGHT_WIDTH;
			
			_bin_curr_y = GAP_TOP_DIS + BIN_TOP_DIS;
			in_height = _dim.height - (_bin_curr_y << 1);
			
			_maxBinsValue = _binCnt.GetMaxBinValue();
			_numOfBins = _binCnt.GetNumOfActiveBins() ;
			_maxBinsIndex = _binCnt.GetMaxBinIndex();
			
			/**
			 * since we consider outline as active bin even it is zero
			 */
			_numOfBins += _binCnt.GetBins()[0] == 0 ? 1 : 0;
			
			_bin_height = ((double)in_height / (double)(( _numOfBins << 1) - 1));
			_bin_height_two = _bin_height * 2;
			_bin_height_half = (int)(_bin_height / 2);
			
			_pixel_per_unit = (double)(in_width-LABEL_WIDTH) / (double)_maxBinsValue;
			
			//save for later use
			_prevDim = _dim;
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
			
			g.setFont(FONT_PLAIN);
			
//			System.out.println("repaint");
			
			//********************************
			//* If the fadeinbuffer is not ready
			//********************************
			if(_binCnt==null||_binCnt.GetNumOfActiveBins()==0) {
				g.setColor(Color.WHITE);
				g.fillRect(0, 0, getWidth(), getHeight());
				g.setColor(Color.GRAY);
				g.drawString("Waiting for data...", TEXT_LEFT_WIDTH, getHeight() - TEXT_HEIGHT);
				return;
			}
			
			//System.out.println("paint histogram = " + counter++);
			
			handleScale();
			
			g.setColor(Color.BLACK);
			g.drawLine(LINE_LEFT_WIDTH,0,LINE_LEFT_WIDTH,getHeight());
			
			//********************************
			//* Draw the histogram
			//********************************
			int[] bins;
			int value;
			int width;
			int x;
			
			Color color;
			
			bins = _binCnt.GetBins();
			//_maxBinsIndex
			
			//***********************
			//draw the outliner even it is zero
			//***********************
			int label_ex = LABEL_WIDTH_GAP + LINE_LEFT_WIDTH;
			int text_y;
			
			value = bins[0];

			width = (int)((double)value * _pixel_per_unit);
			
			x = LINE_LEFT_WIDTH + width;
			
			g.setColor(MColorMgr.GetInstance().GetColor(0,MGlobal.FADE_IDX_MAX));
			
			text_y = _bin_curr_y + _bin_height_half;
			
			g.fillRect(LINE_LEFT_WIDTH, _bin_curr_y, width, (int)_bin_height );
			g.drawString(value+"",width + label_ex,text_y);
			g.drawString("0",TEXT_LEFT_WIDTH,text_y);

			_bin_curr_y += (int)_bin_height_two;
			
			//***********************
			//* draw the rest
			//***********************
			
			for( int idx=1; idx<=_binCnt.GetMaxBinIndex(); idx++) {
				
				value = bins[idx];

				if(value>0) {
					
					width = (int)((double)value * _pixel_per_unit);
					
					x = LINE_LEFT_WIDTH + width;
					
					g.setColor(MColorMgr.GetInstance().GetColor(idx,MGlobal.FADE_IDX_MAX));
					g.fillRect(LINE_LEFT_WIDTH, _bin_curr_y, width, (int)_bin_height );
					
					text_y = _bin_curr_y + _bin_height_half;
					
					g.drawString(value+"",width + label_ex,text_y);
					g.drawString(idx+"",TEXT_LEFT_WIDTH,text_y);
					
					_bin_curr_y += (int)_bin_height_two;
				}
			}
			bins = null;
		}
	}
}
