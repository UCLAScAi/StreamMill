/*
 */
package atlas;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;


public class Echo3 {
  public static void main(String args[]) {

    ServerSocket echoServer = null;
    String line;
    DataInputStream is;
    PrintStream os;
    Socket clientSocket = null;

    try {
       echoServer = new ServerSocket(3355);
    }
    catch (IOException e) {
       System.out.println(e);
    }   
    try {
      clientSocket = echoServer.accept();
      is = new DataInputStream(clientSocket.getInputStream());
      os = new PrintStream(clientSocket.getOutputStream());
    
      line = is.readLine();
      System.out.println("Read: " + line);
      os.println("Module_Name ModuleisCheckOrEvenLonger id_Mo\nq1\nq2\nModule_Name No id_No\nq3\nq4");
      
      is.close();
      os.close(); 
    }   
    catch (IOException e) {
      System.out.println(e);
    }
  }
}
