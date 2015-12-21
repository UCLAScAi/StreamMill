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
import java.util.Enumeration;
import java.util.List;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

/*
 */

public class ComponentDetails extends JDialog implements ActionListener {
  private String compId;
  private List compIds;
  private StreamMill parent;
  private JTree tree;
  
  private void createView(Container c, String details) {
    Vector defMutableTreeNodeVec;
    defMutableTreeNodeVec = new Vector();
    defMutableTreeNodeVec.add(new DefaultMutableTreeNode("Buffer Tree"));
    String nodes[] = details.split("#");
    int size = nodes.length;
    
    for(int i = 0; i < size; i++) {
      String[] pair = nodes[i].split(" ");
      int level = pair[0].length() - 1;
      String stmtName;
      String bufName;
      DefaultMutableTreeNode dmt = null;
      
      if(pair.length == 2 || pair.length == 3) {
        if(pair.length == 2) {
          bufName = pair[1];
          dmt = new DefaultMutableTreeNode(bufName);
        }
        else if(pair.length == 3) {
          stmtName = pair[1];
          bufName = pair[2];
          dmt = new DefaultMutableTreeNode("Stmt: " + stmtName);
          dmt.add(new DefaultMutableTreeNode(bufName));  
        }
        
         
        ((DefaultMutableTreeNode)defMutableTreeNodeVec.get(level)).add(dmt);
        if(defMutableTreeNodeVec.size() != level+3) {
          defMutableTreeNodeVec.add(level+1, dmt);
        }
        else {
          defMutableTreeNodeVec.set(level+1, dmt);
        }
      }
    }
    
    tree = new JTree((TreeNode)defMutableTreeNodeVec.get(0));
    expandAll(tree, true);
    JPanel p = new JPanel();
    p.add(new JScrollPane(tree));
    c.add(p);
    
    JButton vs = new JButton("View Statement");
    JButton ms = new JButton("Move Statement");
    JButton bc = new JButton("Break Component");
    
    vs.addActionListener(this);
    ms.addActionListener(this);
    bc.addActionListener(this);
    
    JPanel l = new JPanel(new GridLayout(1, 3));
    l.add(vs);
    l.add(ms);
    l.add(bc);
    c.add(BorderLayout.AFTER_LAST_LINE, l);
  }
  
  private void expandAll(JTree tree, boolean expand) {
      TreeNode root = (TreeNode)tree.getModel().getRoot();
    
      // Traverse tree from root
      expandAll(tree, new TreePath(root), expand);
  }
  private void expandAll(JTree tree, TreePath parent, boolean expand) {
      // Traverse children
      TreeNode node = (TreeNode)parent.getLastPathComponent();
      if (node.getChildCount() >= 0) {
          for (Enumeration e=node.children(); e.hasMoreElements(); ) {
              TreeNode n = (TreeNode)e.nextElement();
              TreePath path = parent.pathByAddingChild(n);
              expandAll(tree, path, expand);
          }
      }
    
      // Expansion or collapse must be done bottom-up
      if (expand) {
          tree.expandPath(parent);
      } else {
          tree.collapsePath(parent);
      }
  }

  
  public ComponentDetails(String compId, List compIds, String details, StreamMill parent)
  {
    super(parent, "Component Details");
    
    this.parent = parent;
    this.compId = compId;
    this.compIds = compIds;
    
    createView(getContentPane(), details);
    
    setModal(true);
    pack();
    show();
  }
  
  public void actionPerformed(ActionEvent e) {
    String command = e.getActionCommand();
    if(command.equals("View Statement"))
    {
      TreePath[] tps = tree.getSelectionPaths();
      if(tps == null || tps.length != 1) {
        JOptionPane.showMessageDialog(this, "ERROR: Please select a Stmt that you want to view.");
        return;
      }
      String comp = ((DefaultMutableTreeNode)tps[0].getLastPathComponent()).getUserObject().toString();
      if(!comp.startsWith("Stmt: ")) {
        JOptionPane.showMessageDialog(this, "ERROR: Please select a Stmt that you want to view.");
        return;
      }
      String stmt = comp.substring(6);
      String view_command = StreamMill.VIEW_QUERY_COMMAND + " " + parent.getLoginName() + " " + stmt;
      try
      {
        Socket client = new Socket(StreamMill.StreamServer, StreamMill.StreamServerPort);
        int i = 0;
        String buf = "";
    
        DataInputStream is = new DataInputStream(client.getInputStream());

        client.getOutputStream().write(view_command.getBytes());
    
        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          if(responseLine.equals("\n") == false)
            buf = buf + responseLine + "\n";
        }

        client.getOutputStream().close();
        is.close();
        client.close();
    
        new ModuleQueryDetails("Query Details", buf, parent);
      }    
      catch(IOException exp)
      {
        exp.printStackTrace();
      }
    }
    else if(command.equals("Move Statement"))
    {
      TreePath[] tps = tree.getSelectionPaths();
      if(tps == null || tps.length != 1) {
        JOptionPane.showMessageDialog(this, "ERROR: Please select a Stmt that you want to move.");
        return;
      }
      String comp = (String)((DefaultMutableTreeNode)tps[0].getLastPathComponent()).getUserObject();
      if(!comp.startsWith("Stmt: ")) {
        JOptionPane.showMessageDialog(this, "ERROR: Please select a Stmt that you want to move.");
        return;
      }
      String stmt = comp.substring(6);
      if(stmt != null)
      {
        String toCompId = (String)JOptionPane.showInputDialog(this, 
                                    "Please select a Component to which the statement " + stmt + " will be moved",
                                    "Select Component", JOptionPane.QUESTION_MESSAGE, null, compIds.toArray(), "");
        if(toCompId != null)
        {
          String textToSend = StreamMill.MOVE_STATEMENT_COMMAND + " " + parent.getLoginName() + " " + 
                                        stmt + " " + compId + " " + toCompId;
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
              JOptionPane.showMessageDialog(this, "ERROR: Could not move statement: " + textToSend);
            }
            else
            {
              this.dispose();
            }
 
            is.close();
            outToServer.close();
            client.close();
					} catch (IOException exp) {
						exp.printStackTrace();
					}
        }
        else
        {
          JOptionPane.showMessageDialog(this, "ERROR: You did not select a component Id to move the statement to.");
        }
      }
    }
    else if(command.equals("Break Component"))
    {
      TreePath[] tpPaths = tree.getSelectionPaths();
      if(tpPaths == null || tpPaths.length < 1) {
        JOptionPane.showMessageDialog(this, "ERROR: Please select at least one buffer as a break point.");
        return;
      }
      String[] tps = new String[tpPaths.length];
      for(int i=0; i < tps.length; i++) {
        tps[i] = (String)((DefaultMutableTreeNode)tpPaths[i].getLastPathComponent()).getUserObject();
      }
      String[] bufs = new String[tps.length];
      for(int i = 0; i < tps.length; i++) {
        if(tps[i].startsWith("Stmt: ")) {
          JOptionPane.showMessageDialog(this, "ERROR: Please select buffers only as break point(s).");
          return;
        }
        bufs[i] = tps[i];
      }
      if(bufs.length != 0)
      {
        String textToSend = StreamMill.BREAK_COMPONENT_COMMAND + " " + parent.getLoginName() + " " + compId + " ";
        for(int i = 0; i < bufs.length; i++)
        {
          textToSend = textToSend.concat((String)bufs[i]);
          if(i < bufs.length -1)
          textToSend = textToSend.concat("||");
        }
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
            JOptionPane.showMessageDialog(this, "ERROR: Could not break component: " + textToSend);
          }
          else
          {
            this.dispose();
          }
          is.close();
          outToServer.close();
          client.close();
        } catch (IOException exp) {
          exp.printStackTrace();
        }
      }
      else
      {
        JOptionPane.showMessageDialog(this, "ERROR: Please select at least one buffer as a break point.");
      }
    }
  }
}
