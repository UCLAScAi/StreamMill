/*
 * Created on Apr 6, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.proxy;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import atlas.monitoring.MMainDispatcher;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MProxyTCP extends MProxy {
	
	private final int UDP_PACKET_SIZE = 1024;
	
	private Thread _listeneringThread;
	private ServerSocket _listenSocket;
	
	public MProxyTCP(String loginName, String serverName, int serverPort) {
		super(loginName,serverName,serverPort);
	}
	
	public void StartListener(MMainDispatcher dispatcher) {
  	
	  	if(_listenSocket==null) {
		  	try
			{
		  		_listenSocket = new ServerSocket(this.MONITORING_PORT);
			}catch(Exception ex){
				ex.printStackTrace();
				return;
			}
	  	}
	  		  	
	  	if(_listeneringThread==null) {
	  		_listeneringThread = new Thread(new MonitoringDataListenerThread(_listenSocket,dispatcher));
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
	  	
	  	if(_listenSocket!=null) {
	  		try {
	  			_listenSocket.close();
	  		}
	  		catch(Exception ex) {}
	  		_listenSocket = null;
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
	  
	  	private MMainDispatcher _dispatcher;
	  	private ServerSocket _serverSocket; 
		
		public MonitoringDataListenerThread(ServerSocket serverSocket, MMainDispatcher dispatcher) {
			this._dispatcher = dispatcher;
			this._serverSocket = serverSocket;
		}
		
		public void run() {
			Socket clientSocket;
		    DataInputStream in;
		    String responseLine;
		    
		    while(true)
		    {
		    	try
				{
				    while(true) {
				    	clientSocket = _serverSocket.accept();
				    	in = new DataInputStream(clientSocket.getInputStream());
				    	while ((responseLine = in.readLine()) != null) {
                //System.out.println("Data " + responseLine);
				    		this._dispatcher.DispatchData(responseLine);
				    	}
				    	in.close();
				    }
				}
		    	catch(IOException e)
				{
				}
		    }
		}
	}
}
