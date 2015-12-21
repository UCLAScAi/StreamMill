/*
 * Created on Apr 6, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.proxy;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;

import atlas.monitoring.MMainDispatcher;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MProxyUDP extends MProxy{
	
	private final int UDP_PACKET_SIZE = 1024;
	
	private Thread _listeneringThread;
	private DatagramSocket _serverSocket; 
	
	public MProxyUDP(String loginName, String serverName, int serverPort) {
		super(loginName,serverName,serverPort);
	}
	
	public void StartListener(MMainDispatcher dispatcher) {
  	
	  	if(_serverSocket==null) {
		  	try
			{
			  	_serverSocket = new DatagramSocket(this.MONITORING_PORT);
			}catch(Exception ex){
				ex.printStackTrace();
				return;
			}
	  	}
	  		  	
	  	if(_listeneringThread==null) {
	  		_listeneringThread = new Thread(new MonitoringDataListenerThread(_serverSocket,dispatcher,UDP_PACKET_SIZE));
	  		_listeneringThread.start();
	  	}
	  	else {
	  		if(!_listeneringThread.isAlive()) {
	  			_listeneringThread.start();
	  		}
	  	}
	}
  
	public void StopListening() {
	  	if(_listeneringThread != null && _listeneringThread.isAlive()) {
	  		_listeneringThread.stop();
		}
	  	
	  	_listeneringThread = null;
	  	
	  	if(_serverSocket!=null) {
	  		_serverSocket.close();
	  		_serverSocket = null;
	  	}
	}
  
  /**
   * 
   * @author treetree
   *
   * We should allow both TCP and UDP supports. The preference protocol is specified
   * in the properties files.
   */
	public static class MonitoringDataListenerThread implements Runnable {
	  
	  	private int _upd_packet_size = 1024;
	  	private MMainDispatcher _dispatcher;
	  	private DatagramSocket _serverSocket; 
		
		public MonitoringDataListenerThread(DatagramSocket serverSocket, MMainDispatcher dispatcher, int udpSize) {
			this._upd_packet_size = udpSize;
			this._dispatcher = dispatcher;
			this._serverSocket = serverSocket;
		}
		
		public void run() {
	
			//int cnt = 0;
			
			if(this._serverSocket==null)
				return;
			
			byte[] receiveData = new byte[this._upd_packet_size];
	
			String serverSentence;
			
			try {
				DatagramPacket receivePacket = new DatagramPacket(receiveData,receiveData.length);
				
				while(true){
			  		
					try {
						_serverSocket.receive(receivePacket);
						String clientSentence = new String(receivePacket.getData(), 0,receivePacket.getLength());
						
			  			this._dispatcher.DispatchData(clientSentence);
			  			//cnt++;
			  			
			  			//testing: show me the all the received string
			  			//System.out.println(cnt + " --> " + clientSentence);
					}
					catch(SocketException ex ){
						return;
					}
					catch(IOException ex) {
						ex.printStackTrace();
					}
			  	}
			}
			catch(Exception ex) {
				ex.printStackTrace();
			}
		}
	}
}
