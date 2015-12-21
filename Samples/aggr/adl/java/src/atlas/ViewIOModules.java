package atlas;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.LinkedList;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

/*
 */

public class ViewIOModules  extends JDialog implements ActionListener {
  private List ioModuleObjects;
  private StreamMill parent;

  static class IOModuleObject
  {
    public String ioModuleName;
    public JLabel status;
    public JButton viewDS;
    //public JButton activate;
    public JButton delete;
    public JCheckBox deleteCb;
  
    IOModuleObject(String ioModuleName, JLabel status, JButton viewDS, JButton delete, JCheckBox deleteCb)
    {
      this.ioModuleName = ioModuleName;
      this.status = status;
      this.viewDS = viewDS;
      //this.activate = activate;
      this.delete = delete;
      this.deleteCb = deleteCb;
    }
  }

  private void createView(Container c, List hierarchy)
    {
      JPanel p1 = new JPanel();
      GridLayout gl = new GridLayout(hierarchy.size(), 1);
      p1.setLayout(gl);
      
      JPanel p2 = new JPanel();
      GridLayout g2 = new GridLayout(hierarchy.size(), 4);
      p2.setLayout(g2);
      
      int module = 1;
      String currentModuleId = "";
  
      for(int i =0; i< hierarchy.size(); i++)
      {
        String hi = (String)hierarchy.get(i);
        String[] part = hi.split(" ");
        JLabel status;
        JButton viewDS;
        //JButton action;
        JButton delete;
        JCheckBox deleteCb;
        
        viewDS = new JButton("View DataSource");
        viewDS.addActionListener(this);
        delete = new JButton("Delete");
        delete.addActionListener(this);
        deleteCb = new JCheckBox("Deleted", false);
        deleteCb.setEnabled(false);
        
        if(part[0].equalsIgnoreCase("yes"))
        {
          //action = new JButton("Deactivate");
          status = new JLabel("   Status:  Active");
        }
        else
        {
          //action = new JButton("Activate");
          status = new JLabel("   Status:  Not Active");
        }
        //action.addActionListener(this);
        
        JFormattedTextField x = new JFormattedTextField("IOModule: " + part[1]);
        
        /* Want to increase the height of the label, could not do it, hence using JFormattedTextField*/
        x.setBorder(BorderFactory.createEmptyBorder(x.getInsets().top+2, x.getInsets().left, 
                      x.getInsets().bottom+2, x.getInsets().right));
        x.setEditable(false);
        x.setBackground(null);
        
        p1.add(x);
        p2.add(status);
        p2.add(viewDS);
        //p2.add(action);
        p2.add(delete);
        p2.add(deleteCb);
        ioModuleObjects.add(new IOModuleObject(part[1], status, viewDS, delete, deleteCb));
      }
  
      JButton exit = new JButton("Exit");
      exit.addActionListener(this);
      
      JPanel p = new JPanel();
      p.add(p1);
      p.add(p2);
  
      JScrollPane s = new JScrollPane(p, 
                                      JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                      JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
  
      c.add(BorderLayout.EAST, s);
      c.add(BorderLayout.AFTER_LAST_LINE, exit);
    }


  public ViewIOModules(List bufs, StreamMill parent)
  {    
    super(parent, "IOModules");
    this.parent = parent;
    ioModuleObjects = new LinkedList();
    createView(getContentPane(), bufs);
    setModal(true);
    pack();
    show();
  }
  
  public int getIOMViewIndex(JButton b)
  {
    int size = ioModuleObjects.size();
    for(int i = 0; i < size; i++)
    {
      JButton button = ((IOModuleObject)ioModuleObjects.get(i)).viewDS;
      if(b == button)
        return i;
    }
    return -1;
  }

  /*public int getIOMIndex(JButton b)
  {
    int size = ioModuleObjects.size();
    for(int i = 0; i < size; i++)
    {
      JButton button = ((IOModuleObject)ioModuleObjects.get(i)).activate;
      if(b == button)
        return i;
    }
    return -1;
  }*/

  public void sendMessage(String command, Socket client) {
    try
        {
          int i = 0;
          String buf = "";
      
          client.getOutputStream().write(command.getBytes());
      
          //client.setTrafficClass(Socket);
        }
        catch(IOException exp)
        {
          exp.printStackTrace();
        }
  }
  
  public int getIOMDeleteIndex(JButton b)
  {
    int size = ioModuleObjects.size();
    for(int i = 0; i < size; i++)
    {
      JButton button = ((IOModuleObject)ioModuleObjects.get(i)).delete;
      if(b == button)
        return i;
    }
    return -1;
  }

  public void actionPerformed(ActionEvent e) {
    JButton b = (JButton)e.getSource();
    String command = e.getActionCommand();
    int iomIndex = 0;

    if(command.equals("Exit"))
    {
      this.setVisible(false);
      this.dispose();
    }
    else if(command.equals("View DataSource")) {
      iomIndex = getIOMViewIndex(b);
      IOModuleObject iomObj = (IOModuleObject)ioModuleObjects.get(iomIndex);
      if(iomObj.deleteCb.isSelected() == false)
      {
        String textToSend = StreamMill.VIEW_IOMODULE_COMMAND + " " + parent.getLoginName() + " " + iomObj.ioModuleName;
        try
        {
          Socket client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort);
          StringBuffer buf = new StringBuffer("");
          
          
          
          DataInputStream is = new DataInputStream(client.getInputStream());
    
          client.getOutputStream().write(textToSend.getBytes());
          
          //client.setTrafficClass(Socket);
          String responseLine;
          while ((responseLine = is.readLine()) != null) {
            if(responseLine.equals("\n") == false)
              buf.append(responseLine + "\n");
          }
  
          client.getOutputStream().close();
          is.close();
          client.close();
          
          new ModuleQueryDetails("Data Source Details", buf.toString(), parent, iomObj.ioModuleName, 1, this);
        }
        catch(IOException exp)
        {
          exp.printStackTrace();
        }
      }
    }
    else if(command.equals("Delete"))
    {
      if(parent.getUserStatus() != StreamMill.LOGGED_IN)
      {
        StreamMill.showHaveToBeLoggedInMessage();
        return;
      }
      Socket client = null;
      try {
        client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort);        
        iomIndex = getIOMDeleteIndex(b);
        IOModuleObject iomObj = (IOModuleObject)ioModuleObjects.get(iomIndex);
        if(iomObj.deleteCb.isSelected() == false)
        {
          iomObj.deleteCb.setSelected(true);
          sendMessage(StreamMill.DROP_IOMODULE_COMMAND + " " + parent.getLoginName() + " " + iomObj.ioModuleName, client);
        }
        client.getOutputStream().close();
        client.close();
      } catch (Exception exp) {
        exp.printStackTrace();
      }
    }
    /*else if(command.equals("Deactivate"))
    {
      if(parent.getUserStatus() != StreamMill.LOGGED_IN)
      {
        StreamMill.showHaveToBeLoggedInMessage();
        return;
      }
      Socket client = null;
      try {
        client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort);				
		
        iomIndex = getIOMIndex(b);
        IOModuleObject iomObj = (IOModuleObject)ioModuleObjects.get(iomIndex);
        if(iomObj.deleteCb.isSelected() == false)
        {
          iomObj.activate.setText("Activate");
          iomObj.status.setText("   Status:  Not Active");
          sendMessage(StreamMill.DEACTIVATE_IOMODULE_COMMAND + " " + parent.getLoginName() + " " + iomObj.ioModuleName, client);
        }
        client.getOutputStream().close();
        client.close();
      } catch (Exception exp) {
        exp.printStackTrace();
      }
    }
    else if(command.equals("Activate"))
    {
      if(parent.getUserStatus() != StreamMill.LOGGED_IN)
      {
        StreamMill.showHaveToBeLoggedInMessage();
        return;
      }
      iomIndex = getIOMIndex(b);
      IOModuleObject iomObj = (IOModuleObject)ioModuleObjects.get(iomIndex);
      if(iomObj.deleteCb.isSelected() == false)
      {
        try {
          Socket client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort);
          sendMessage(StreamMill.ACTIVATE_IOMODULE_COMMAND + " " + parent.getLoginName() + " " + iomObj.ioModuleName, client);
          DataInputStream is = new DataInputStream(client.getInputStream());
          String responseLine = null;
          StringBuffer wholeMessage = new StringBuffer();

          while ((responseLine = is.readLine()) != null) {
            wholeMessage.append(responseLine + "\n");
          }
          if(!wholeMessage.toString().trim().equals(""))
          {
            JOptionPane.showMessageDialog(this, "ERROR: Could not activate IOModule. Error message from server is:\n" +
                                                wholeMessage);
          }
          else
          {
            iomObj.activate.setText("Deactivate");
            iomObj.status.setText("   Status:  Active");
          }
 
          is.close();
          //client.getOutputStream().close();
          client.close();
				} catch (Exception exp) {
					exp.printStackTrace();
				}

      }
    }*/
  }
}

