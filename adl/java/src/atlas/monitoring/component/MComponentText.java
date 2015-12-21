/*
 * Created on Feb 19, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.component;

import java.awt.Dimension;

import javax.swing.BoxLayout;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;


/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentText extends MComponent {
	
	private JTextArea _textArea;
	
	/**
	 * TODO: add back the dataType and fieldIndex
	 *
	 */
	public MComponentText() {
		super("MonitoringComponentText");
		
		this.setLayout(new BoxLayout(this,BoxLayout.PAGE_AXIS));
		
		this._textArea = new JTextArea();
		this._textArea.setEditable(false);
		
		JScrollPane pane = new JScrollPane(this._textArea);
		
		this.add(pane);
		
		this._preferredSize = new Dimension(150,400);
	}
	
	public void CleanUp() {
		//do nothing
	}
	
	public void AddValue(String textValue) {

		if(textValue==null)
			return;
		
		this._textArea.append(textValue + "\n");
		this._textArea.setCaretPosition(this._textArea.getDocument().getLength());
	}
	
	public void ClearOutput() {
		this._textArea.setText("");
	}

	public String GetOutputString() {
		return this._textArea.getText();
	}
}
