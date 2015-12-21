/*
 * Created on Mar 6, 2005
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
public class MStreamingBuffer {

	public final int MIN_BUFFER_SIZE = 10;
	private Object _sharedObject[];
	private int _bufferSize;
	
	private boolean _writeable = true;
	private boolean _readable = false;
	private int _readLocation = 0;
	private int _writeLocation = 0;
	
	/**
	 * 
	 * @param size The size of streaming buffer
	 */
	public MStreamingBuffer(int size) {
		_bufferSize = (size<MIN_BUFFER_SIZE ? MIN_BUFFER_SIZE : size );
		_sharedObject = new Object[_bufferSize];
	}
	
	/**
	 * Return the size of buffer
	 * @return Size of buffer
	 */
	public int GetSize() {
		return _bufferSize;
	}
	
	/**
	 * Add the new data into the buffer (java.lang.String). However, if the buffer is already full, 
	 * it will drop the new text.
	 * @param text New data text
	 */
	public synchronized boolean Add(Object text) {
		
		//if no buffer space, sorry I have to drop this text
		if(!_writeable) {
			/**
			 * TODO: remove
			 */
			System.out.println("Packet drop");
			return false;
		}
		
		_sharedObject[_writeLocation] = text;
		
		_readable = true;
		
		_writeLocation = (_writeLocation+1) % _bufferSize;
		
		if(_writeLocation==_readLocation)
			_writeable = false;
		
		notify();
		
		int value = _writeLocation - _readLocation;
		
		value = value < 0 ? value * -1: value;
		
//		if(value > 1)
//			System.out.println(value);
		
		return true;
	}
	
	/**
	 * Get the text value from the streaming buffer
	 * @return Next text queued in the buffer
	 */
	public synchronized Object Get() {
		Object value;
		
		while(!_readable) {
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
		
		value = _sharedObject[_readLocation];
		
		//release the pointer
		_sharedObject[_readLocation] = null;
		
		_writeable = true;
		
		_readLocation = (_readLocation+1) % _bufferSize;
		
		if(_readLocation==_writeLocation)
			_readable = false;
		
		return value;
	}
}
