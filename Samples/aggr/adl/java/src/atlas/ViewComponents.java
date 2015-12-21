package atlas;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.LinkedList;
import java.util.List;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

/*
 */

public class ViewComponents extends JDialog implements ActionListener {
  private List componentObjects;
  private StreamMill parent;
  
  static class ComponentObject
  {
    public JButton viewDetails;
    public JButton setPriority;
    public JLabel lblPriority;
    public int priority;
    public String id;
    
    ComponentObject(JButton viewDetails, String id, JButton setPriority, 
                    JLabel lblPriority, int priority)
    {
      this.viewDetails = viewDetails;
      this.id = id;
      this.setPriority = setPriority;
      this.lblPriority = lblPriority;
      this.priority = priority;
    }
  }

  private void createView(Container c, String comps)
  {
    String[] components = comps.split("\\|\\|");
    JPanel p = new JPanel();
    GridLayout gl = new GridLayout(components.length, 4);
    p.setLayout(gl);
    
    for(int i = 0; i < components.length; i++)
    {
      JButton b = new JButton("View Details");
      JButton s = new JButton("Set Priority");
      
      b.addActionListener(this);
      s.addActionListener(this);
      String[] compProps = components[i].split(" ");
      String compId = compProps[0];
      int priority = Integer.parseInt(compProps[1]);
      JLabel l = new JLabel("" + priority);
      componentObjects.add(new ComponentObject(b, compId, s, l, priority));
      p.add(new JLabel(compId));
      p.add(l);
      p.add(s);
      p.add(b);
      
    }
    
    JButton join = new JButton("Merge Components");
    join.addActionListener(this);
    JButton exit = new JButton("Exit");
    exit.addActionListener(this);
    
    JScrollPane s = new JScrollPane(p, 
                                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    
    c.add(BorderLayout.CENTER, s);
    JPanel l = new JPanel(new GridLayout(1, 2));
    l.add(BorderLayout.WEST, join);
    l.add(BorderLayout.EAST, exit);
    c.add(BorderLayout.AFTER_LAST_LINE, l);
  }
  
  
  public ViewComponents(StreamMill parent, String components)
  {
    super(parent, "Components");
    this.parent = parent;
    componentObjects = new LinkedList();
    
     
    createView(getContentPane(), components);
    setModal(true);
    pack();
    show();
  }
  
  private void processSetPriority(ComponentObject m) {
    String priority = (String)JOptionPane.showInputDialog(this, 
                                "Please specify positive numeric priority",
                                "Set Priority", JOptionPane.QUESTION_MESSAGE);
    int intPriority = 0;
    try {
      intPriority = Integer.parseInt(priority);	
		} catch (Exception e) {
      JOptionPane.showMessageDialog(this, "ERROR: The priority you specified is not numeric.");
      return;
		}
    
    if(intPriority <= 0) {
      JOptionPane.showMessageDialog(this, "ERROR: The priority you specified is negative or 0.");
      return;
    }
    
    if(intPriority == m.priority) {
      JOptionPane.showMessageDialog(this, "The component's priority is already " + m.priority + ".");
      return;
    }
    
    String textToSend = StreamMill.SET_COMPONENT_PRIORITY_COMMAND + " " + parent.getLoginName() + " " + m.id + " " + intPriority;
    
    try {
      Socket client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort); 
      char[] buff = new char[4096];
      int nch;
      OutputStream outToServer = client.getOutputStream();
      outToServer.write(textToSend.getBytes());

      DataInputStream is = new DataInputStream(client.getInputStream());
      String responseLine = null;
      responseLine = is.readLine();
      if(responseLine != null)
      {
        JOptionPane.showMessageDialog(this, "ERROR: Could not set priority: " + textToSend);
      }
      else
      {
        m.priority = intPriority;
        m.lblPriority.setText("" + intPriority);
      }
 
      is.close();
      outToServer.close();
      client.close();
    } catch (IOException exp) {
      exp.printStackTrace();
    }
  }
  
  
  private void processViewDetails(ComponentObject m)
  {
    String command = "";
    
    command = StreamMill.VIEW_COMPONENT_DETAILS_COMMAND + " " + parent.getLoginName() + " " + m.id;    
      
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
      
      List compIds = new LinkedList();
      for(int i = 0; i < componentObjects.size(); i++)
      {
        ComponentObject cb = (ComponentObject)componentObjects.get(i);
        if(!cb.id.equals(m.id))
        {
          compIds.add(cb.id);
        }
      }
       
      new ComponentDetails(m.id, compIds, buf, this.parent);
      this.dispose();
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
    else if(command.equals("Merge Components"))
    {
      List compIds = new LinkedList();
      for(int i = 0; i < componentObjects.size(); i++)
      {
        ComponentObject cb = (ComponentObject)componentObjects.get(i);
        compIds.add(cb.id);
      }
      String comp1 = (String)JOptionPane.showInputDialog(this, 
                                  "Please select a Component1 to merge",
                                  "Select Component1", JOptionPane.QUESTION_MESSAGE, null, compIds.toArray(), "");
      if(comp1 != null)
      {
        String comp2 = (String)JOptionPane.showInputDialog(this, 
                                          "Please select a Component2 to merge",
                                          "Select Component2", JOptionPane.QUESTION_MESSAGE, null, compIds.toArray(), "");
        if(comp2 != null)
        {
          if(comp2.equals(comp1))
          {
            JOptionPane.showMessageDialog(this, "ERROR: The components picked for merging are the same.");
          }
          else
          {
            String textToSend = StreamMill.MERGE_COMPONENTS_COMMAND + " " + parent.getLoginName() + " " +
                                         comp1 + " " + comp2;
            try {
              Socket client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort); 
              char[] buff = new char[4096];
              int nch;
              OutputStream outToServer = client.getOutputStream();
              outToServer.write(textToSend.getBytes());

              DataInputStream is = new DataInputStream(client.getInputStream());
              String responseLine = null;
              responseLine = is.readLine();
              if(responseLine != null)
              {
                JOptionPane.showMessageDialog(this, "ERROR: Could not merge components: " + textToSend);
              }
 
              is.close();
              outToServer.close();
              client.close();
            } catch (IOException exp) {
              exp.printStackTrace();
            } 
          }
        }
      }
    }
    else if(command.equals("View Details"))
    {
      for(int i = 0; i < componentObjects.size(); i++)
      {
        ComponentObject m = (ComponentObject)componentObjects.get(i);
        if(b == m.viewDetails)
        {
          processViewDetails(m);
          break;
        }
      }
    }
    else if(command.equals("Set Priority"))
    {
      for(int i = 0; i < componentObjects.size(); i++)
      {
        ComponentObject m = (ComponentObject)componentObjects.get(i);
        if(b == m.setPriority)
        {
          processSetPriority(m);
          break;
        }
      }
    }
  }
}
