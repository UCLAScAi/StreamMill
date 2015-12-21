/*
 * Created on Mar 12, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.io.File;

import javax.imageio.ImageIO;
import javax.swing.JComponent;
import javax.swing.UIManager;

import atlas.monitoring.library.MBinSummary;
import atlas.monitoring.library.MBinSummaryUI;
import atlas.monitoring.library.MClusterSummary;
import atlas.monitoring.library.MClusterToolkit;
import atlas.monitoring.library.MColorMgr;
import atlas.monitoring.library.MGlobal;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentHistogramVertical extends MComponentHistogram {
	
	protected HistogramArea _histogramArea;
	protected int _bufferSize = 100;
	protected boolean _isAutoCaptureHistogram = false;
	protected String _capFileNameHistogram;
	protected MBinSummary _binCnt;
  protected MBinSummary _oldBinCnt;
	protected boolean _isDrawOK = false;
	protected boolean _isAutoCapture = false;
	protected Thread _drawingThread;
	protected String _capturePath;
	protected int _capCounter = 0;
	protected int _curWindowIdx = 0;

	
	/**
	 * 	Good habit to release the resource 
	 */
	public void CleanUp() {
		if(_drawingThread!=null)
			_drawingThread.stop();
	}

	public void AddValue(int x, int y, int bin, int windowIdx) {
    System.out.println("Adding value " + bin + ", " + windowIdx);
		if(_binCnt == null || _curWindowIdx<windowIdx) {
			
			if(_binCnt != null && _isAutoCapture) {
				new Thread(new CaptureThread(_capturePath + (_capCounter++) + ".jpg", _histogramArea )).start();
			}
			
      if(_binCnt != null) {
        _oldBinCnt = _binCnt;
      }
      
			_binCnt = new MBinSummary(_bufferSize);
			_binCnt.SetDynamicResize(true);
			_curWindowIdx = windowIdx;
		}
		_binCnt.Add(x, y, bin);
		setDrawStatus(true);
	}
	
  public void AddValue(int cnt, int bin, int windowIdx) {
    System.out.println("Setting cnt " + bin + ", " + windowIdx + ", " + cnt);
    if(_binCnt == null || _curWindowIdx<windowIdx) {
      
      if(_binCnt != null && _isAutoCapture) {
        new Thread(new CaptureThread(_capturePath + (_capCounter++) + ".jpg", _histogramArea )).start();
      }
      
      if(_binCnt != null) {
        System.out.println("Setting oldcnt");
        _oldBinCnt = _binCnt;
      }

      System.out.println("allocating new binCnt");
      _binCnt = new MBinSummary(_bufferSize);
      _binCnt.SetDynamicResize(true);
      _curWindowIdx = windowIdx;
    }
    _binCnt.Add(cnt, bin);
    setDrawStatus(true);
  }
  
	/**
	 * No use
	 */
	public void AddValue(String value) {}
	
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
	protected class CaptureThread implements Runnable {
		
		protected String _name;
		protected JComponent _com;
		
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
	protected class DrawingThread implements Runnable {
		
		protected boolean _shouldIwait = true;
		
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
	
	protected synchronized void waitForDraw() {
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
	
	protected synchronized void setDrawStatus(boolean status) {
		_isDrawOK = status;
		
		if(_isDrawOK)
			notify();
	}

	public MComponentHistogramVertical(Object[] arguments) {
		super("MComponentHistogramVertical");
		
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
	 *
	 * @author treetree
	 *
	 * Historgram area
	 */
	protected class HistogramArea extends JComponent {
		
		protected final Color COLOR_BACKGROUND = Color.WHITE;
		protected final Color COLOR_TEXT = Color.DARK_GRAY;
		protected final Font FONT_BOLD = new Font("Arial",Font.BOLD,10);
		protected final Font FONT_PLAIN = new Font("Arial",Font.PLAIN,10);
		
		protected int GAP_TOP_HEIGHT = 20;
		protected int GAP_SIDE_WIDTH = 10;
		protected int BIN_SIDE_WIDTH = 10;
		
		protected int LINE_HEIGHT = 30;
		protected int AXIS_HEIGHT = 15;
		protected int TEXT_HEIGHT = 10;
		protected int TEXT_START_X = 50;
		protected int LABEL_HEIGHT = 5;
		
		protected int _abs_line_height;
		protected int _abs_axis_height;
		protected int _abs_text_height;
		protected int _abs_text_start_x;
		
		protected int _bin_cur_x;
		protected double _bin_width;
		protected double _bin_width_two;
		protected double _pixel_per_unit;
		protected Dimension _dim;
		protected Dimension _prevDim = new Dimension(0,0);
		
		protected int _numOfBins;
		protected int _maxBinsValue;
		protected int _maxBinsIndex;
		
		protected int counter = 0;
		
		protected void drawHistogram(Graphics g, MBinSummary binCnt) {
      //    ********************************
      //* Draw the histogram
      //********************************
      MClusterSummary[] bins;
      int value;
      int height;
      int y;
  
      Color color;
      Font f = g.getFont();
      Font newFont= new Font(UIManager.getFont("Label.font").getName(), Font.PLAIN, 14);
      g.setFont(newFont);
  
      bins = binCnt.GetBins();
      //_maxBinsIndex
  
      //***********************
      //draw the outliner even it is zero
      //***********************
      value = bins[0].getCount();

      height = (int)((double)value * _pixel_per_unit);
  
      y = _abs_line_height - height;
  
      g.setColor(MColorMgr.GetInstance().GetColor(0,MGlobal.FADE_IDX_MAX));
      g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
      g.setColor(Color.BLACK);
      g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
      g.drawString("0",(int)_bin_cur_x,_abs_axis_height);

      _bin_cur_x += (int)_bin_width_two;
  
      //***********************
      //* draw the rest
      //***********************
  
      for( int idx=1; idx<=binCnt.GetMaxBinIndex(); idx++) {
    
        value = bins[idx].getCount();

        if(value>0) {
      
          height = (int)((double)value * _pixel_per_unit);
      
          y = _abs_line_height - height;
      
          g.setColor(MColorMgr.GetInstance().GetColor(idx,MGlobal.FADE_IDX_MAX));
          g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
          g.setColor(Color.BLACK);
          g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
          g.drawString(idx+"",(int)_bin_cur_x,_abs_axis_height);
      
          _bin_cur_x += (int)_bin_width_two;
        }
      }
      g.setFont(f);
      bins = null;
		}
     
    protected void drawHistogram(Graphics g, MBinSummary binCnt, MBinSummary oldCnt) {
      //    ********************************
      //* Draw the histogram
      //********************************
      MClusterSummary[] bins;
      MClusterSummary[] oldBins;
      int value;
      int height;
      int y;
  
      Color color;
      Font f = g.getFont();
      Font newFont= new Font(UIManager.getFont("Label.font").getName(), Font.PLAIN, 14);
      g.setFont(newFont);
  
      bins = binCnt.GetBins();
      oldBins = oldCnt.GetBins();
      //_maxBinsIndex
  
      //***********************
      //draw the outliner even it is zero
      //***********************
      value = bins[0].getCount();

      height = (int)((double)value * _pixel_per_unit);
  
      y = _abs_line_height - height;
  
      g.setColor(MColorMgr.GetInstance().GetColor(0,MGlobal.FADE_IDX_MAX));
      g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
      g.setColor(Color.BLACK);
      g.drawRect(_bin_cur_x-1, y, (int)_bin_width, height);
      g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
      g.drawString("0",(int)_bin_cur_x,_abs_axis_height);

      //only add one bin_width since matching old will be put next
      _bin_cur_x += (int)_bin_width;
      
      
      //***********************
      //draw the outliner even it is zero -- old
      //***********************
      value = oldBins[0].getCount();

      height = (int)((double)value * _pixel_per_unit);
  
      y = _abs_line_height - height;
  
      g.setColor(MColorMgr.GetInstance().GetColor(0,MGlobal.FADE_IDX_MAX));
      g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
      g.setColor(Color.BLACK);
      g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
      g.drawString("0",(int)_bin_cur_x,_abs_axis_height);

      _bin_cur_x += (int)_bin_width_two;
  
      //***********************
      //* draw the rest
      //***********************
  
      for( int idx=1; idx<=binCnt.GetMaxBinIndex(); idx++) {
    
        value = bins[idx].getCount();

        if(value>0) {
      
          height = (int)((double)value * _pixel_per_unit);
      
          y = _abs_line_height - height;
      
          g.setColor(MColorMgr.GetInstance().GetColor(idx,MGlobal.FADE_IDX_MAX));
          g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
          g.setColor(Color.BLACK);
          g.drawRect(_bin_cur_x-1, y, (int)_bin_width, height);
          g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
          g.drawString(idx+"",(int)_bin_cur_x,_abs_axis_height);
        }
        
        if(oldCnt.GetMaxBinIndex() >= idx && oldBins[idx].getCount()>0) {
          _bin_cur_x += (int)_bin_width;
          value = oldBins[idx].getCount();
          height = (int)((double)value * _pixel_per_unit);
      
          y = _abs_line_height - height;
      
          g.setColor(MColorMgr.GetInstance().GetColor(idx,MGlobal.FADE_IDX_MAX));
          g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
          g.setColor(Color.BLACK);
          g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
          g.drawString(idx+"",(int)_bin_cur_x,_abs_axis_height);

          _bin_cur_x += (int)_bin_width_two;
        }
        else {
          _bin_cur_x += (int)_bin_width_two;
        }
      }
      
      if(binCnt.GetMaxBinIndex() < oldCnt.GetMaxBinIndex()) {
        for(int idx=binCnt.GetMaxBinIndex()+1; idx<=oldCnt.GetMaxBinIndex(); idx++) {
          value = oldBins[idx].getCount();

          if(value>0) {
      
             height = (int)((double)value * _pixel_per_unit);
      
             y = _abs_line_height - height;
      
             g.setColor(MColorMgr.GetInstance().GetColor(idx,MGlobal.FADE_IDX_MAX));
             g.fillRect(_bin_cur_x, y, (int)_bin_width, height);
             g.setColor(Color.BLACK);
             g.drawString(value+"",(int)_bin_cur_x,y-LABEL_HEIGHT);
             g.drawString(idx+"",(int)_bin_cur_x,_abs_axis_height);

            _bin_cur_x += (int)_bin_width_two;
           }
        }
      }
      g.setFont(f);
      bins = null;
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
				g.drawLine(GAP_SIDE_WIDTH,getHeight() - LINE_HEIGHT,(getWidth()-GAP_SIDE_WIDTH),getHeight() - LINE_HEIGHT);
				g.drawString("Waiting for data...", TEXT_START_X, getHeight() - TEXT_HEIGHT);
				return;
			}
			
			//System.out.println("paint histogram = " + counter++);
			
			handleScale();
			
			g.setColor(Color.BLACK);
			g.drawLine(GAP_SIDE_WIDTH,_abs_line_height,(_dim.width-GAP_SIDE_WIDTH),_abs_line_height);
			
      if(_oldBinCnt == null) {
        drawHistogram(g, _binCnt);
      }
      else {
        drawHistogram(g, _binCnt, _oldBinCnt);
      }
		}
		
		protected void handleScale() {
			
			int in_height;
			int in_width;
			
			_dim = getSize();
			
			in_height = _dim.height - LINE_HEIGHT - GAP_TOP_HEIGHT;
			
			_bin_cur_x = GAP_SIDE_WIDTH + BIN_SIDE_WIDTH;
			in_width = _dim.width - (_bin_cur_x << 1);
			
			_maxBinsValue = _binCnt.GetMaxBinValue();
			_numOfBins = _binCnt.GetNumOfActiveBins() ;
			_maxBinsIndex = _binCnt.GetMaxBinIndex();
			
      if(_oldBinCnt != null) {
        if(_oldBinCnt.GetMaxBinValue() > _maxBinsValue) {
          _maxBinsValue = _oldBinCnt.GetMaxBinValue();
        }
        if(_oldBinCnt.GetMaxBinIndex() > _maxBinsIndex) {
          _maxBinsIndex = _oldBinCnt.GetMaxBinIndex();
        }
        _numOfBins = _numOfBins + _oldBinCnt.GetNumOfActiveBins();
        _numOfBins += _oldBinCnt.GetBins()[0].getCount() == 0 ? 1 : 0;
      }
      
			/**
			 * since we consider outline as active bin even it is zero
			 */
			_numOfBins += _binCnt.GetBins()[0].getCount() == 0 ? 1 : 0;
			
			_bin_width = ((double)in_width / (double)(( _numOfBins << 1) - 1));
			_bin_width_two = _bin_width * 2;

			_pixel_per_unit = (double)(in_height-LABEL_HEIGHT) / (double)_maxBinsValue;
			
			_abs_line_height = _dim.height - LINE_HEIGHT;
			_abs_axis_height = _dim.height - AXIS_HEIGHT;
			_abs_text_height = _dim.height - TEXT_HEIGHT;
			
			//save for later use
			_prevDim = _dim;
		}
	}
}
