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

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

public class AMH extends JDialog implements ActionListener {
  private List moduleObjects;
  private int mode;
  StreamMill parent;
  
  static class ModuleObject
  {
    public JLabel status;
    public JButton active;
    public JButton delete;
    public JButton viewDetails;
    public JCheckBox deletedCb;
    public String id;
    public String name;
    public String moduleId;
    public int isModule;
    public int isActive;
    
    ModuleObject(JLabel status, JButton active, JButton delete, JButton viewDetails, String id, int isActive, 
                 String name, String moduleId, int isModule, JCheckBox deletedCb)
    {
      this.status = status;
      this.active = active;
      this.delete = delete;
      this.viewDetails = viewDetails;
      this.id = id;
      this.isActive = isActive;
      this.name = name;
      this.deletedCb = deletedCb;
      this.moduleId = moduleId;
      this.isModule = isModule;
    }
  }

  private void createView(Container c, List hierarchy, int mode)
  {
    JPanel p = new JPanel();
    GridLayout gl = null;
    
    if(mode != StreamMill.VIEW_ALL_QUERIES && mode != StreamMill.VIEW_ALL_TS_QUERIES 
        && mode != StreamMill.VIEW_ALL_STREAMS) {
      gl = new GridLayout(hierarchy.size(), 4);
    }
    else {
      gl = new GridLayout(hierarchy.size(), 6);      
    }
    p.setLayout(gl);
    int module = 1;
    String currentModuleId = "";
    
    for(int i =0; i< hierarchy.size(); i++)
    {
      String hi = (String)hierarchy.get(i);
      if(!hi.equals("-"))
      {
        String id;
        String name;
        JButton active = null;
        JLabel status = null;
        JButton d = new JButton("Delete");
        d.addActionListener(this);
        JButton v = new JButton("View Details");
        v.addActionListener(this);
        int isActive = 0;
        
        String[] part = hi.split(" ");
        if(module == 1)
        {
          name = "Module: " + part[0];
          currentModuleId = part[1];
          id = part[1];
          if((mode == StreamMill.VIEW_ALL_QUERIES 
              || mode == StreamMill.VIEW_ALL_TS_QUERIES
              || mode == StreamMill.VIEW_ALL_STREAMS) 
            && part[2].equals("yes")) {
            status = new JLabel("Status: Active");
            active = new JButton("Deactivate");
            isActive = 1;
          }
          else if(mode == StreamMill.VIEW_ALL_QUERIES
                  || mode == StreamMill.VIEW_ALL_TS_QUERIES
                  || mode == StreamMill.VIEW_ALL_STREAMS) {
            status = new JLabel("Status: Not active");
            active = new JButton("Activate");
            isActive = 0;
          }
        }
        else
        {
          name = "    Stmt:" + part[0];
          id = part[0];
          if((mode == StreamMill.VIEW_ALL_QUERIES 
              || mode == StreamMill.VIEW_ALL_TS_QUERIES 
              || mode == StreamMill.VIEW_ALL_STREAMS) 
              && part[1].equals("yes")) {
            status = new JLabel("Status: Active");
            active = new JButton("Deactivate");
            isActive = 1;
          }
          else if(mode == StreamMill.VIEW_ALL_QUERIES 
                  || mode == StreamMill.VIEW_ALL_TS_QUERIES  
                  || mode == StreamMill.VIEW_ALL_STREAMS) {
            status = new JLabel("Status: Not active");
            active = new JButton("Activate");
            isActive = 0;
          }
        }
        p.add(new JLabel(name));
        p.add(v);
        if(mode == StreamMill.VIEW_ALL_QUERIES || mode == StreamMill.VIEW_ALL_TS_QUERIES
            || mode == StreamMill.VIEW_ALL_STREAMS) {
          p.add(status);
          p.add(active);
        }
        p.add(d);
        JCheckBox cb = new JCheckBox("Deleted", false);
        cb.setEnabled(false);
        p.add(cb);
        if(active != null) {
          active.addActionListener(this);
        }
        
        moduleObjects.add(new ModuleObject(status, active, d, v, id, isActive, name, currentModuleId, module, cb));
        module = 0;
      }
      else
      {
        gl.setRows(gl.getRows() - 1);
        module = 1;
      }
    }
    
    JButton exit = new JButton("Exit");
    exit.addActionListener(this);
    
    JScrollPane s = new JScrollPane(p, 
                                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    
    c.add(BorderLayout.CENTER, s);
    c.add(BorderLayout.AFTER_LAST_LINE, exit);
  }

  public AMH(List hierarchy, StreamMill parent, int mode)
  {
    super(parent, "Active Modules/Queries");
    moduleObjects = new LinkedList();
    
    this.parent = parent;
    this.mode = mode;
    if(mode == StreamMill.VIEW_ALL_STREAMS)
    {
      this.setTitle("Active Modules/Streams");
    }
    else if(mode == StreamMill.VIEW_ALL_TABLES)
    {
      this.setTitle("Active Modules/Tables");
    }
    else if(mode == StreamMill.VIEW_ALL_AGGREGATES)
    {
      this.setTitle("Defined Aggregates");
    }
    else if(mode == StreamMill.VIEW_ALL_EXTERNS)
    {
      this.setTitle("Defined External Functions");
    }
    else if(mode == StreamMill.VIEW_ALL_TS_QUERIES)
    {
      this.setTitle("Active Time Series Queries");
    }
    createView(getContentPane(), hierarchy, mode);
    setModal(true);
    pack();
    show();
  }
  
  private void processDelete(ModuleObject m, int index)
  {
    String command = "";
    
    if(m.isModule == 0)
    {
      if(mode == StreamMill.VIEW_ALL_STREAMS || mode == StreamMill.VIEW_ALL_TABLES)
        command = StreamMill.DELETE_DECLARE_COMMAND + " " + parent.getLoginName() + " " + m.moduleId + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_QUERIES)
        command = StreamMill.DELETE_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.moduleId + " " + m.id;
    }
    else
    {
      if(mode == StreamMill.VIEW_ALL_STREAMS || mode == StreamMill.VIEW_ALL_TABLES)
        command = StreamMill.DELETE_DECLARE_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_QUERIES)
        command = StreamMill.DELETE_QUERY_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_AGGREGATES)
        command = StreamMill.DELETE_AGGREGATE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_EXTERNS)
        command = StreamMill.DELETE_EXTERN_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_TS_QUERIES)
              command = StreamMill.DELETE_TS_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.id;
    }

    try
    {
      Socket client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort);
    
      client.getOutputStream().write(command.getBytes());
      DataInputStream is = new DataInputStream(client.getInputStream());
      String responseLine = null;
      StringBuffer wholeMessage = new StringBuffer();

      while ((responseLine = is.readLine()) != null) {
        wholeMessage.append(responseLine + "\n");
      }
      if(!wholeMessage.toString().trim().equals(""))
      {
        if(m.isModule == 0)
          JOptionPane.showMessageDialog(this, "ERROR: Could not delete Query/Declare. Error message from server is:\n" +
                                              wholeMessage.toString() + "If you are trying to delete declares then make " + 
                                              "sure that there are no continuous queries running on it.");
        else
          JOptionPane.showMessageDialog(this, "ERROR: Could not delete at least one of the queries/declares in the module.\n" +
                                              "Error message from the server is:\n" + wholeMessage.toString());
      }
      else
      {
        m.deletedCb.setSelected(true);
        if(m.isModule == 1)
        {
          for(int i = index +1; i< moduleObjects.size(); i++)
          {
            ModuleObject queryObj = (ModuleObject)moduleObjects.get(i);
            if(queryObj.isModule == 1)
              break;
            queryObj.deletedCb.setSelected(true);
          }
        }
      }
      
      is.close();     
      //client.getOutputStream().close();
      client.close();
    }
    catch(IOException exp)
    {
      exp.printStackTrace();
    }
  }
  
  private void processActivate(ModuleObject m, int index) {
    String command = null;
    if(m.isModule == 1) {
      if(mode == StreamMill.VIEW_ALL_STREAMS)
        command = StreamMill.ACTIVATE_STREAM_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_QUERIES)
        command = StreamMill.ACTIVATE_QUERY_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_TS_QUERIES)
        command = StreamMill.ACTIVATE_TS_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.id;
    }
    else if(m.isModule != 1) {
      if(mode == StreamMill.VIEW_ALL_STREAMS)
        command = StreamMill.ACTIVATE_STREAM_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else
        command = StreamMill.ACTIVATE_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.id;
    }
    String reply = parent.sendCommandToServerAndReceiveExactReply(command);
    if(reply != null && !reply.trim().equals("")) {
      StreamMill.showErrorMessage("Could not activate query. Error message from server is:\n" + reply);
    }
    else {
      m.status.setText("Status: Active");
      m.isActive = 1;
      m.active.setText("Deactivate");
      
      if(m.isModule == 1)
      {
        for(int i = index +1; i< moduleObjects.size(); i++)
        {
          ModuleObject queryObj = (ModuleObject)moduleObjects.get(i);
          if(queryObj.isModule == 1)
            break;
          queryObj.status.setText("Status: Active");
          queryObj.isActive = 1;
          queryObj.active.setText("Deactivate");
        }
      }
    }
  }
  
  private void processDeactivate(ModuleObject m, int index) {
    String command = null;
    if(m.isModule == 1) {
      if(mode == StreamMill.VIEW_ALL_STREAMS)
        command = StreamMill.DEACTIVATE_STREAM_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_QUERIES)
        command = StreamMill.DEACTIVATE_QUERY_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else
        command = StreamMill.DEACTIVATE_TS_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.id;
    }
    else if(m.isModule != 1) {
      if(mode == StreamMill.VIEW_ALL_STREAMS)
         command = StreamMill.DEACTIVATE_STREAM_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else
        command = StreamMill.DEACTIVATE_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.id;
    }
    String reply = parent.sendCommandToServerAndReceiveExactReply(command);
    if(reply != null && !reply.trim().equals("")) {
      StreamMill.showErrorMessage("Could not activate query. Error message from server is:\n" + reply);
    }
    else {
      m.status.setText("Status: Not active");
      m.isActive = 0;
      m.active.setText("Activate");
    }      
    if(m.isModule == 1)
    {
      for(int i = index +1; i< moduleObjects.size(); i++)
      {
        ModuleObject queryObj = (ModuleObject)moduleObjects.get(i);
        if(queryObj.isModule == 1)
          break;
        queryObj.status.setText("Status: Not active");
        queryObj.isActive = 0;
        queryObj.active.setText("Activate");
      }
    }
  }
  
  private void processViewDetails(ModuleObject m)
  {
    String command = "";
    
    if(m.isModule == 0)
    {
      if(mode == StreamMill.VIEW_ALL_STREAMS || mode == StreamMill.VIEW_ALL_TABLES)
        command = StreamMill.VIEW_DECLARE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_QUERIES)
        command = StreamMill.VIEW_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.id;
    }
     
    else if(m.isModule == 1)
    {
      if(mode == StreamMill.VIEW_ALL_STREAMS || mode == StreamMill.VIEW_ALL_TABLES)
        command = StreamMill.VIEW_DECLARE_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_QUERIES)
        command = StreamMill.VIEW_QUERY_MODULE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_AGGREGATES)
        command = StreamMill.VIEW_AGGREGATE_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_EXTERNS)
        command = StreamMill.VIEW_EXTERN_COMMAND + " " + parent.getLoginName() + " " + m.id;
      else if(mode == StreamMill.VIEW_ALL_TS_QUERIES)
              command = StreamMill.VIEW_TS_QUERY_COMMAND + " " + parent.getLoginName() + " " + m.id;
    }
          
    try
    {
      Socket client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort);
      String buf = "";
        
      DataInputStream is = new DataInputStream(client.getInputStream());
  
      client.getOutputStream().write(command.getBytes());
        
      //client.setTrafficClass(Socket);
      String responseLine;
      while ((responseLine = is.readLine()) != null) {
        if(responseLine.equals("\n") == false)
          buf = buf + responseLine + "\n";
      }

      client.getOutputStream().close();
      is.close();
      client.close();
        
      new ModuleQueryDetails("Module/Query/Stmt Details", buf, parent, m.name, 1, this);
    }
    catch(IOException exp)
    {
      exp.printStackTrace();
    }
  }
  
  public void actionPerformed(ActionEvent e) {
    JButton b = (JButton)e.getSource();
    String command = e.getActionCommand();
    
    if(command.equals("Exit"))
    {
      this.setVisible(false);
      this.dispose();
    }
    else if(command.equals("Delete"))
    {
      if(parent.getUserStatus() != StreamMill.LOGGED_IN)
      {
        StreamMill.showHaveToBeLoggedInMessage();
        return;
      }
      for(int i = 0; i < moduleObjects.size(); i++)
      {
        ModuleObject m = (ModuleObject)moduleObjects.get(i);
        if(b == m.delete)
        {
          if(m.deletedCb.isSelected() == false)
            processDelete(m, i);  //passing index to set deleted of module's queries (if we are deleteing a module)
          //System.out.println("Delete button clicked for " 
          //                    + m.name + ", id = " 
          //                    + m.id + ", isModule = " + m.isModule);
          break;
        }
      }
    }
    else if(command.equals("View Details"))
    {
      for(int i = 0; i < moduleObjects.size(); i++)
      {
        ModuleObject m = (ModuleObject)moduleObjects.get(i);
        if(b == m.viewDetails)
        {
          if(m.deletedCb.isSelected() == false)
            processViewDetails(m);
          //System.out.println("View Details button clicked for " 
          //                    + m.name + ", id = " 
          //                    + m.id + ", isModule = " + m.isModule);
          break;
        }
      }
    }
    else if(command.equals("Activate"))
    {
      if(parent.getUserStatus() != StreamMill.LOGGED_IN)
      {
        StreamMill.showHaveToBeLoggedInMessage();
        return;
      }
      for(int i = 0; i < moduleObjects.size(); i++)
      {
        ModuleObject m = (ModuleObject)moduleObjects.get(i);
        if(b == m.active)
        {
          if(m.deletedCb.isSelected() == false)
            processActivate(m, i);
          //System.out.println("View Details button clicked for " 
          //                    + m.name + ", id = " 
          //                    + m.id + ", isModule = " + m.isModule);
          break;
        }
      }
    }
    else if(command.equals("Deactivate"))
    {
      if(parent.getUserStatus() != StreamMill.LOGGED_IN)
      {
        StreamMill.showHaveToBeLoggedInMessage();
        return;
      }
      for(int i = 0; i < moduleObjects.size(); i++)
      {
        ModuleObject m = (ModuleObject)moduleObjects.get(i);
        if(b == m.active)
        {
          if(m.deletedCb.isSelected() == false)
            processDeactivate(m, i);
          //System.out.println("View Details button clicked for " 
          //                    + m.name + ", id = " 
          //                    + m.id + ", isModule = " + m.isModule);
          break;
        }
      }
    }
  }
}
