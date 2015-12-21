/*
 * Created on Feb 8, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.library;

/**
 *  @author Tree
- * @version $Revision: 1.5 $ $Date: 2005/10/31 23:26:39 $
 *
 */
public class MBufferElement {
	
	private boolean _hasValue;
	private double _value;
	
	/**
	 * Default Constructor: contains no value 
	 */
	public MBufferElement() {
		this._hasValue = false;
		this._value = 0.0;
	}
	
	/**
	 * Constructor
	 * @param value element's value
	 */
	public MBufferElement( double value ) {
		this._value = value;
		this._hasValue = true;
	}
	
	/**
	 * Check to see if this element contains value
	 * @return true - contains value
	 */
	public boolean HasValue() {
		return this._hasValue;
	}
	
	/**
	 * Set the value of this element stored in StreamingBuffer
	 * @param value
	 * @see MStreamingBuffer
	 */
	public void SetValue( double value ) {
		this._value = value;
		//bug fix: 20050304 : skip will turn the flag off. 
		//so, we have to reset the flag
		this._hasValue = true;		
	}
	
	/**
	 * Get the value stored in element
	 * @return element's value
	 */
	public double GetValue() {
		return HasValue() ? this._value : 0.0;
	}
	
	/**
	 * Clear the value of this element
	 */
	public void Clear() {
		this._hasValue = false;
	}
}
