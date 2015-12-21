/*
 * Created on Mar 11, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.global;

import java.util.Hashtable;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import atlas.event.ITextValueChangeEventListener;
import atlas.event.TextValueChangeEvent;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class AtlasGlobal {
	
	protected static javax.swing.event.EventListenerList _textValueChangeListenerList = new javax.swing.event.EventListenerList();
	
	public final static String MESSAGE = "message";
	public final static String ERROR_CONNECTION = "Fail to connect to Stream Mill server. Please check your connection...";
	public final static Hashtable KeywordHash = new Hashtable();
	private static AtlasGlobal _me = new  AtlasGlobal();
		
	//***************************************************
	//* Get the filaname from properties file
	//***************************************************
	
	private static ResourceBundle _resources;
	
    static {
    	try
		{
    		_resources = ResourceBundle.getBundle("atlas/resources/properties.AtlasGlobal", Locale.getDefault());
		}
		catch (MissingResourceException mre){
			mre.printStackTrace();
			_resources = null;
		}
    }
	
	// Look For Keyword In Resource File
	final static protected String[] getResourceStrings(String nm) {
		String[] strings = null;
		
		if(_resources!=null) {
			try {
				strings = _resources.getString(nm).split("\\s");
			}
			catch (Exception ex) {
				strings = new String[0];
			}
		}
		
		return strings;
    }
	


	public static AtlasGlobal GetInstance() {
		
		makeKeyWordHash();
		
		return _me;
	}
	
	private static void makeKeyWordHash() {
		
		String keywords[] = getResourceStrings("keywords");
		String str;
		
		for(int i=0; i<keywords.length; i++) {
			
			str = keywords[i].toLowerCase();
			
			if(!KeywordHash.containsKey(keywords[i])) {
				KeywordHash.put(keywords[i],"");
			}
		}
	}
	
	public void ShowErrorMessage(String message) {
		if(message==null)
			message = "";
		
		fireValueChangeEvent( new TextValueChangeEvent(_me,MESSAGE,message));
	}
	
	
	public void ShowConnectionError() {
		fireValueChangeEvent( new TextValueChangeEvent(_me,MESSAGE,ERROR_CONNECTION));
	}
	
	//
	//	For message change 
	//
	public void AddTextValueChangeEventListener(ITextValueChangeEventListener listener) {
		_textValueChangeListenerList.add(ITextValueChangeEventListener.class, listener);
    }

    // This methods allows classes to unregister for MyEvents
    public void RemoveTextValueChangeListener(ITextValueChangeEventListener listener) {
    	_textValueChangeListenerList.remove(ITextValueChangeEventListener.class, listener);
    }

    // This private class is used to fire MyEvents
    private void fireValueChangeEvent(TextValueChangeEvent evt) {
        Object[] listeners = _textValueChangeListenerList.getListenerList();
        // Each listener occupies two elements - the first is the listener class
        // and the second is the listener instance
        for (int i=0; i<listeners.length; i+=2) {
            if (listeners[i]==ITextValueChangeEventListener.class) {
                ((ITextValueChangeEventListener)listeners[i+1]).TextValueChangeEventOccurred(evt);
            }
        }
    }
    

}
