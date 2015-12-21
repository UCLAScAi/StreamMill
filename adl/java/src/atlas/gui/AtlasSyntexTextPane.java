/*
 * Created on Mar 9, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.gui;

import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import javax.swing.Action;
import javax.swing.JTextPane;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;
import javax.swing.text.StyledEditorKit;
import javax.swing.text.TabSet;
import javax.swing.text.TabStop;

import atlas.global.AtlasGlobal;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class AtlasSyntexTextPane extends JTextPane implements KeyListener {
	
	private final int STATE_INI = 0;
	private final int STATE_TXT = 1;
	private final int STATE_CMT_START_1 = 2;
	private final int STATE_CMT_START_2 = 3;
	private final int STATE_CMD_END_1 = 4;
	private final int STATE_STR = 5;
	private Action HIGHTLIGHT_ACTION = new StyledEditorKit.ForegroundAction("Blue",Color.BLUE);
    private Action TEXT_HIGHLIGHT_ACTION = new StyledEditorKit.ForegroundAction("BLACK",Color.BLACK);
    private Action CMT_HIGHLIGHT_ACTION = new StyledEditorKit.ForegroundAction("GREEN",Color.getHSBColor(0.331f, 0.9f, 0.573f));
    private Action STR_HIGHLIGHT_ACTION = new StyledEditorKit.ForegroundAction("RED",Color.RED);
//    private Action BOLD_ACTION = new StyledEditorKit.BoldAction();
//    private Action ITALIC_ACTION = new StyledEditorKit.ItalicAction();
	
    private StyledDocument _doc; 
    private boolean _isHighlight = true;
    private boolean _isCmtOrStrOn = false;
    
	public AtlasSyntexTextPane() {
	
    	//_doc = getStyledDocument();
    	
    	SetTabs(2);
    	
    	addKeyListener(this);
	}
	
	public void SetHighLight(boolean choice) {
		_isHighlight = choice;
	}
	
	public void SetTabs(int charactersPerTab)
	{
		FontMetrics fm = getFontMetrics( this.getFont() );
		int charWidth = fm.charWidth( 'w' );
		int tabWidth = charWidth * charactersPerTab;
 
		TabStop[] tabs = new TabStop[100];
 
		for (int j = 0; j < tabs.length; j++)
		{
			int tab = j + 1;
			tabs[j] = new TabStop( tab * tabWidth );
		}
 
		TabSet tabSet = new TabSet(tabs);
		SimpleAttributeSet attributes = new SimpleAttributeSet();
		StyleConstants.setTabSet(attributes, tabSet);
		int length = getDocument().getLength();
		getStyledDocument().setParagraphAttributes(0, length, attributes, false);
	}
	
	public void keyPressed(KeyEvent e){}
	public void keyReleased(KeyEvent e){
	if(_isHighlight && !(e.isAltDown() || e.isControlDown() || e.isActionKey() || e.isShiftDown() ) && (_isCmtOrStrOn || !Character.isLetterOrDigit(e.getKeyChar())))
		{
			if(e.getKeyCode() == KeyEvent.VK_ALT || e.getKeyCode() == KeyEvent.VK_CONTROL ||
			   e.getKeyCode() == KeyEvent.VK_SHIFT		) {
				return;
			}
			syntex();
		}	
	}
	
	
	public void keyTyped(KeyEvent e){
		
		
	}
	
	public void setText(String text) {
		super.setText(text);
		grabFocus();
	}

	//*************************************************************************
	//* New Implementation
	//* 2005/03/19
	//*************************************************************************
	public void syntex() {
		
		int state = STATE_INI;
		int end = 0;
		
		try {
			
			_doc = getStyledDocument();
			
			end = getCaretPosition();
		 	
	        String text = _doc.getText(0, _doc.getLength());
	       
	        setCaretPosition(_doc.getLength());
	        
	        int pos = 0;
	        
	        int max = text.length() -1;
	        
	        select(0,_doc.getLength());
    		
	        //First remove all old highlights
	        TEXT_HIGHLIGHT_ACTION.actionPerformed(null);
	        
	        int start = pos;
	        char ch;
	        
	        //state checking
	        
	        int idx;
	        
	        for( idx=0; idx<text.length(); idx++, pos++) {
	        	
	        	ch = text.charAt(idx);
	        	
	        	if(state==STATE_INI) {
	        		
	        		start = pos;
	        		
	        		if(Character.isLetter(ch)) {
	        			state = STATE_TXT;
	        		}
	        		else if(ch == '/') {
	        			state=STATE_CMT_START_1;

	        		}
	        		else if(ch=='\'') {
	        			state=STATE_STR;

	        		}
	        	}
	        	else if(state==STATE_TXT) {
	        		if(!Character.isLetter(ch)) {
	        			//I will not consume this character
	        			String tmp = text.substring(start,idx);
	        			
	        			if(AtlasGlobal.KeywordHash.containsKey(tmp.toLowerCase())) {
		        			select(start,pos);
		            		HIGHTLIGHT_ACTION.actionPerformed(null);

		        		}
	            		idx--;	//since I am doing lookahead
	            		pos--;
	            		state = STATE_INI;
	        		}
	        	}
	        	else if(state==STATE_STR) {
	        		//I will consume this character
	        		if(ch=='\'') {
	        			select(start,pos+1);
	            		STR_HIGHLIGHT_ACTION.actionPerformed(null);
	            		state = STATE_INI;
	        		}
	        	}
	        	else if(state==STATE_CMT_START_1) {
	        		if(ch=='*') {
	        			state = STATE_CMT_START_2;
	        		}
	        		else {
	        			//I will not consume this character
	        			state=STATE_INI;
	        			idx--;	//since I am doing lookahead
	        			pos--;
	        		}
	        	}
	        	else if(state==STATE_CMT_START_2) {
	        		if(ch=='*') {
	        			state=STATE_CMD_END_1;
	        		}
	        	}
	        	else if(state==STATE_CMD_END_1) {
	        		if(ch=='/') {
	        			select(start,pos+1);
	            		CMT_HIGHLIGHT_ACTION.actionPerformed(null);
	            		//ITALIC_ACTION.actionPerformed(null);
	            		state = STATE_INI;
	        		}
	        		else if( ch!='*'){
	        			state=STATE_CMT_START_2;
	        		}
	        	}
	        	else {
	        		return;
	        	}
	        }
	        
	        _isCmtOrStrOn = true;
	        
	        //end
	        if(state==STATE_TXT) {
	        	String tmp;
	        	
	        	if(start==idx) {
	        		tmp = text.substring(idx);
	        	}
	        	else {
	        		tmp = text.substring(start,idx);
	        	}
	        	
	        	if(AtlasGlobal.KeywordHash.containsKey(tmp.toLowerCase())) {
        			select(start,pos);
            		HIGHTLIGHT_ACTION.actionPerformed(null);
            		//BOLD_ACTION.actionPerformed(null);
        		}
	        	_isCmtOrStrOn = false;
        	}
        	else if(state==STATE_CMT_START_2 || state==STATE_CMD_END_1) {
        		select(start,pos+1);
        		CMT_HIGHLIGHT_ACTION.actionPerformed(null);
        	}
        	else if(state==STATE_STR) {
        		select(start,pos+1);
        		STR_HIGHLIGHT_ACTION.actionPerformed(null);
        	}
        	else if(state==STATE_INI) {
        		setCaretPosition(end);
        		TEXT_HIGHLIGHT_ACTION.actionPerformed(null);
        		_isCmtOrStrOn = false;
        	}
    	 }
		 catch(Exception ex) {
		 	ex.printStackTrace();
		 }
		 finally {
		 	setCaretPosition(end);
			//TEXT_HIGHLIGHT_ACTION.actionPerformed(null);
		 }
	}
	
	
	
//	public void syntexHighlight() {
//  // First remove all old highlights
//
//	int end = 0;
//	
//	 try {
//	 	
//      String text = _doc.getText(0, _doc.getLength());
//      int pos = 0;
//      
//      int max = text.length() -1;
//      end = getCaretPosition();
//      
//      select(0,_doc.getLength());
//		
//      TEXT_HIGHLIGHT_ACTION.actionPerformed(null);
//		
//		for( int i=0; i<keywords.length; i++ ) {
//	        
//      	String pattern = keywords[i];
//   
//          // Search for pattern
//          while ((pos = text.indexOf(pattern, pos)) >= 0) {
//              // Create highlighter using private painter and apply around pattern
//              
//          	int prev = pos - 1;
//          	int after = pos+pattern.length();
//          	
//          	prev = (prev < 0 ? 0 : prev );
//          	
//          	if( ((prev<=0)||Character.isSpace(text.charAt(prev)) || !(Character.isLetter(text.charAt(prev)))) && 
//          		((after>max) || Character.isSpace(text.charAt(after)) || !(Character.isLetter(text.charAt(after)))) ) {
//          		
//          		select(prev,after);
//          		
//          		HIGHTLIGHT_ACTION.actionPerformed(null);
//              
//          		setCaretPosition(after);
//          	}
//          	pos += pattern.length();
//          }
//      }
//		
//		
//		
//		//comment
//		pos = 0;
//		
//		 while ((pos = text.indexOf("/*", pos)) >= 0)  {
//		 	
//		 	int prev = pos-1;
//		 	int after = text.indexOf("*/", 2+pos);
//		 	
//		 	if(after <=0)
//		 		after = end;
//		 	else
//		 		after += 2;
//		 	
//		 	select(prev,after);
//		 	CMT_HIGHLIGHT_ACTION.actionPerformed(null);
//		 	
//		 	setCaretPosition(after);
//		 	
//		 	pos = after;
//		 }
//	 } catch (BadLocationException e) {
//      
//  }
//	 finally {
//	 	setCaretPosition(end);
//		TEXT_HIGHLIGHT_ACTION.actionPerformed(null);
//	 }
//}

	
	
}
