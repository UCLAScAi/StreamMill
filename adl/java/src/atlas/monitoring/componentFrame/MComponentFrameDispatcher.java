/*
 * Created on Mar 6, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.componentFrame;

import atlas.monitoring.component.MComponentHash;
import atlas.monitoring.library.MPacket;
import atlas.monitoring.library.MStreamingBuffer;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MComponentFrameDispatcher {

	private MStreamingBuffer _buffer;
	private MComponentHash _hash;
	private Thread _dispatcherThread;
	
	public MComponentFrameDispatcher() {
		//size of buffer
		_buffer = new MStreamingBuffer(4096);
		_hash = new MComponentHash();
		_dispatcherThread = new Thread(new DispatchingThread(), "DispatcherThread" );
		_dispatcherThread.start();
	}
	
	/**
	 * Add the text value
	 * @param textValue
	 * @return false if the buffer full and the text value drop (networking idea)
	 */
	public boolean AddMonitoringPacket(MPacket packet) {
		return _buffer.Add(packet);
	}
	
	public void RegisterFrame(MComponentFrame frame) {
		_hash.AddFrame(frame);
	}
	
	public void UnregisterFrameByParameterId(String parameterId) {
		_hash.RemoveByParameterId(parameterId);
	}
	
	public void CleanUp(){
		if(_dispatcherThread.isAlive()) {
			_dispatcherThread.stop();
		}
	}
	
	public boolean IsFrameExist(MComponentFrame frame) {
		return _hash.IsFrameExist(frame);
	}
	
	//************************************************
	// Private methods
	//************************************************
	private class DispatchingThread implements Runnable {
		
		public void run() {
			while(true) {
				MPacket packet = (MPacket)_buffer.Get();
				_hash.Dispatch(packet.GetDataType(),packet.GetParameterId(),packet.GetRawData());
			}
		}
	}
}
