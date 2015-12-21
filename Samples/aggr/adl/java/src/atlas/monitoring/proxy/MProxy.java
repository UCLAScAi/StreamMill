/*
 * Created on Feb 19, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.proxy;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.LinkedList;
import java.util.List;

import atlas.monitoring.MMainDispatcher;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MProxy {

	
	protected final int MONITORING_PORT = 5431;
	private String _loginName;
	private String _serverName;
	private int _serverPort;
	private Socket _clientSocket;
	

	  
	public MProxy(String loginName, String serverName, int serverPort)
	{
		this._loginName = loginName;
		this._serverName = serverName;
		this._serverPort = serverPort;
	}
	  
	public List GetMonitoringParameters(String cmd) {
		
		boolean isOK = false;
		List list = new LinkedList();
		Socket clientSocket = null;
		DataInputStream is = null;
		String responseLine = null;
		OutputStream os = null;
		
		try {
			clientSocket = new Socket(this._serverName, this._serverPort);
			is = new DataInputStream(clientSocket.getInputStream());
			os = clientSocket.getOutputStream();
			os.write((cmd + " " + this._loginName).getBytes());
			
			while ((responseLine = is.readLine()) != null) {
			  list.add(responseLine);
			}
			isOK = true;
		}
		catch(IOException ex) {
			ex.printStackTrace();
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
		finally {
			try
			{
				if(os!=null)			os.close();
				if(is!=null)			is.close();
				if(clientSocket!=null)	clientSocket.close();
			}
			catch(Exception ex) {
				ex.printStackTrace();
			}
		}
		
		if(!isOK)
			return null;
		
		return list;
	}
	
	public void RequestMonitoringAction(String command, String parameter_id) {
		sendMessage(command + " " + _loginName + " " + parameter_id);
	}
	
	public void RequestMonitoringAction(String command, String parameter_id, String[] arguments) {
		
		StringBuffer buf = new StringBuffer();
		
		if(arguments!=null && arguments.length>0){
			
			for( int i=0; i<arguments.length; i++) {
				buf.append(arguments[i] + " " );
			}
		}
		
		if(buf.length()>0) {
			sendMessage(command + " " + _loginName + " " + parameter_id + " " + buf.toString().trim());
		}
		else {
			sendMessage(command + " " + _loginName + " " + parameter_id);
		}
	}

	  private void sendMessage(String command) {
	    
	  	Socket client = null;
	  	OutputStream os = null;
	  	
	  	//System.out.println("sendMessage = >" + command + "<");
	  	
	  	try
	        {
	          client = new Socket(this._serverName, this._serverPort);
	          os = client.getOutputStream();
	          os.write(command.getBytes());
	        }
	        catch(IOException exp)
	        {
	          exp.printStackTrace();
	        }
	        finally {
	        	try {	
	        		if(os!=null)		
	        			os.close();    	
	        	}  	
	        	catch(Exception ex) {	
	        		ex.printStackTrace(); 
	        	}
	        	
	        	try {	
	        		if(client!=null)	
	        			client.close();	
	        	}
	        	catch(Exception ex) {	
	        		ex.printStackTrace(); 
	        	}
	        }
	  }
	  
	  public abstract void StartListener(MMainDispatcher dispatcher);
	  public abstract void StopListening();	  
}

