/*
 * Created on Mar 12, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.library;

import java.awt.Color;
import java.util.Random;
import java.util.LinkedList;
import java.util.List;


/**
 * @author treetree
 *
 * Manager the color use in the cluster
 */
public class MColorMgr {

	private final int SEED = 19879;
	private final int COLOR_GAP = 20;	//	85 * 3 = 255
	private final int COLOR_NUM = 3;	//	RGB = 3
	private final int COLOR_MAX = 255;	//	FF = 255
	private final int COLOR_FADE_RANGE = MGlobal.FADE_IDX_MAX;
	private final int COLOR_FADE_RANGE_MIN = MGlobal.FADE_IDX_MIN;
	private final Color COLOR_OUTLINE = Color.LIGHT_GRAY; 
	
	
	private final Color[] DEFAULT_COLOR_LIST = {	Color.GRAY, 
													Color.RED,
													Color.BLUE,
													Color.GREEN,
													Color.MAGENTA,
													Color.ORANGE,
													Color.PINK,
													Color.YELLOW,
													Color.getHSBColor(0.764f,0.481f,0.529f),
													Color.getHSBColor(0.111f, 1.0f, 0.6f),
													Color.getHSBColor(0.003f, 0.975f, 0.62f),
													Color.getHSBColor(0.077f, 1.0f, 0.827f),
													Color.getHSBColor(0.133f, 1.0f, 0.910f),
													Color.getHSBColor(0.318f, 0.728f, 0.404f),
													Color.getHSBColor(0.667f, 1.0f, 0.4f),
													Color.getHSBColor(0.688f, 0.487f, 0.741f),
													Color.getHSBColor(0.898f, 0.867f, 0.443f),
													Color.getHSBColor(0.082f, 1.0f, 0.388f),
													Color.getHSBColor(0.919f, 0.652f, 0.643f),
													Color.getHSBColor(0.332f, 1.0f, 0.718f)};
	
	private List _colorList;
	private Random _rand;
	
	private static MColorMgr _me;
	/**
	 * Constructor
	 */
	private MColorMgr() {
		_colorList = createDefaultList();
		_rand = new Random();
		_rand.setSeed(SEED);
	}
	
	/**
	 * Get Color Mgr Instance
	 * @return
	 */
	public static MColorMgr GetInstance() {
		if(_me==null)
			_me = new MColorMgr();
		
		return _me;
	}

	//***************************************************
	//*		Private methods
	//***************************************************
	
	private int COLOR_RED = 1;
	private int COLOR_GREEN = 2;
	private int COLOR_BLUE = 4;
	
	private Color genRandomColor() {

		int[] value = new int[3];
		int sum;
		int tmp;
		int mine;
		int range = 5000;
		boolean loop = false;
		Color color;
		int maxTry = 5000;
		int cnt = 0;
		
		do {
		
			do {
				value[0] = _rand.nextInt(255);
				value[1] = _rand.nextInt(255);
				value[2] = _rand.nextInt(255);
				sum = value[0] + value[1] + value[2];
				
				//avoid the color too white and black
			}while( sum < 200 || 600 < sum);
			
			color = new Color(value[0],value[1],value[2]);
			
			mine = color.getRGB();
			
			for( int i=0; i<_colorList.size(); i++) {
				
				tmp = ((Color)((List)_colorList.get(i)).get(MGlobal.FADE_IDX_MAX)).getRGB();
				
				//avoid the color looks similiar
				if( (mine - range < tmp) && (tmp < mine + range)  ) {
					loop = true;
					
					break;
				}
			}
			
			cnt++;
			
			//avoid infinite loop
			if(cnt > maxTry) {
				color = Color.BLACK;
				break;
			}
			
		}while(loop);

		return color;
	}
	
	private List genFadingColorList() {
		
		Color color = genRandomColor();
		
		List list = new LinkedList();
		
		for( int i=0; i<=MGlobal.FADE_IDX_MAX; i++ ) {
			list.add(getFadeOutColor(color,i));
		}
		
		return list;
	}

	private List createDefaultList() {
		
		List list = new LinkedList();
		
		for( int idx=0; idx<DEFAULT_COLOR_LIST.length; idx++ ) {
			
			List tmp = new LinkedList();
			
			for( int i=0; i<=MGlobal.FADE_IDX_MAX; i++ ) {
				tmp.add(getFadeOutColor(DEFAULT_COLOR_LIST[idx],i));
			}
			
			list.add(tmp);
		}
		
		return list;		
	}

	/*
	 * Get a color from the list
	 */
	private Color getColorFromList(int clusterId, int fadeIdx) {
		
		fadeIdx = getFade(fadeIdx);
		
		int listLen = _colorList.size();
		
		for( int i=listLen; i<=clusterId; i++ ) {
			_colorList.add(genFadingColorList());
		}
		
		return (Color)((List)_colorList.get(clusterId)).get(fadeIdx);
	}
	
	
	/**
	 * Get the correct fade index
	 * @param num
	 * @return
	 */
	private int getFade(int num) {
		if(num<COLOR_FADE_RANGE_MIN)
			return COLOR_FADE_RANGE_MIN;
		else if(num>COLOR_FADE_RANGE)
			return COLOR_FADE_RANGE;
		
		return num;
	}
	
	/**
	 * Return the new color base on the fading effect
	 * @param color
	 * @param fade
	 * @return
	 */
	private Color getFadeOutColor(Color color, int fadeOut) {
		
		int[] base = new int[3];
		
		double ONE = 1.0;
		double ratio = (((double)(fadeOut)) / ((double)COLOR_FADE_RANGE));
		double fade_val = (ONE - ratio) * ((double)COLOR_MAX);
		
		base[0] = (int)Math.floor((((double)color.getRed()) * ratio + fade_val)); 
		base[1] = (int)Math.floor((((double)color.getGreen()) * ratio + fade_val));
		base[2] = (int)Math.floor((((double)color.getBlue()) * ratio + fade_val));
		
		base[0] = base[0] > COLOR_MAX ? COLOR_MAX : base[0];
		base[1] = base[1] > COLOR_MAX ? COLOR_MAX : base[1];
		base[2] = base[2] > COLOR_MAX ? COLOR_MAX : base[2];
		
		return new Color(base[0],base[1],base[2]);
	}
	
	//***************************************************
	//*		Public methods
	//***************************************************
	
	/**
	 * Get the fading effect for current color
	 * @param color
	 * @param fadeIdx
	 * @return
	 */
	public Color GetColor(Color color, int fadeOutIdx) {
		return getFadeOutColor(color,fadeOutIdx);
	}
	
	/**
	 * Get the fading effect color base on cluster id
	 * @param cluster
	 * @param fadeIdx
	 * @return
	 */
	public Color GetColor(int cluster, int fadeOutIdx) {
		return getColorFromList(cluster,fadeOutIdx);
	}


	
}
