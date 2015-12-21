/*
 * Created on Feb 8, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.library;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MCircularBuffer {

	//private variable
	private MBufferElement[] _buffer;
	private int _cursor;
	private int _end;
	private int _savedEnd;
	private boolean _isbufferFull;
	private int _length;
	
	
	/**
	 * Constructor
	 * @param length The length of the circular buffer. Minimum length is 1.
	 */
	public MCircularBuffer( int length ) {
		
		//default
		this._cursor = 0;
		this._end = 0;
		this._savedEnd = 0;
		this._isbufferFull = false;
			
		//set		
		if( length < 1 ) {
			length = 1;
		}
		
		_length = length;
		
		this._buffer = new MBufferElement[length];
	}
	
	public void ClearAll() {
		this._cursor = 0;
		this._end = 0;
		this._savedEnd = 0;
		this._isbufferFull = false;
		
		for( int i=0; i<_length && _buffer[i]!=null; i++) 
			_buffer[i]=null;
	}
	
	/**
	 * Add new element to the buffer. Depending on the size of the buffer, 
	 * old value will be erase.
	 * @param value Value that stored in the buffer
	 */
	public void Put( double value ) {
		
		if( this._isbufferFull ) {
			this._buffer[this._end].SetValue(value);	
		}
		else {
			MBufferElement element = new MBufferElement( value );
			
			this._buffer[this._end] = element;
			
			if( this._end == this._buffer.length -1 )
				this._isbufferFull = true;
		}
	}
	
	public void Increment( ) {
		this._end = this.calcNextIndex(this._end);
	}
	
	public void Skip() {
		if( this._isbufferFull ) {
			this._buffer[this._end].Clear();	
		}
		else {
			MBufferElement element = new MBufferElement( );
			
			this._buffer[this._end] = element;
			
			if( this._end == this._buffer.length -1 )
				this._isbufferFull = true;
		}
		
		this._end = this.calcNextIndex(this._end);
	}
	
	//**************************************************
	//* Iterator that used to read the buffer
	//**************************************************
	
	/**
	 * Initialized the read action
	 */
	public void initRead() {
		this._cursor = this._end;
		this._savedEnd = this._cursor;
	}
	
	/**
	 * Check to see if the buffer still contains the previous value
	 * @return true if buffer contains previous value
	 */
	public boolean hasPrevious() {
		
		int tmpEnd = this._end;
		
		this._cursor = calcPrvIndex(this._cursor);
		
		if( this._cursor == tmpEnd || this._cursor == this._savedEnd )
			return false;
		
		this._savedEnd = this._end;
		
		if( this._isbufferFull || this._cursor < tmpEnd )
			return true;
		
		return false;
	}
	
	/**
	 * Has the current value stored in buffer
	 * @return Value in double type
	 */
	public double GetValue() {
		
		if(this._buffer[this._cursor]==null)
			return 0.0;
		
		return this._buffer[this._cursor].GetValue();
	}
	
	/**
	 * Check to see if current buffer element has value
	 * @return True if the element has value
	 */
	public boolean HasValue() {
		
		if(this._buffer[this._cursor]==null)
			return false;
		
		return this._buffer[this._cursor].HasValue();
	}
		
	//private function
	/**
	 * Determine the next buffer index regarding to current buffer index
	 * @return Next buffer index
	 */
	private int calcNextIndex( int idx ) {
		idx++;
		
		if( idx >= this._buffer.length )
			idx = 0;
		
		return idx;
	}
	
	/**
	 * Return the previous index of the buffer
	 * @return Previous index of the buffer
	 */
	private int calcPrvIndex( int idx ) {
		idx--;
		
		if( idx < 0 )
			idx = this._buffer.length - 1;
		
		return idx;
	}
}

