package atlas;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

public class ModuleQueryDetails extends JDialog implements ActionListener {

  StreamMill parent;
  String moduleName;
  String buf;
  JDialog toCloseOnEdit;
  
  public ModuleQueryDetails(String title, String buf, StreamMill parent) {
    this(title, buf, parent, null, 0, null);
  }
  
  public ModuleQueryDetails(String title, String buf, StreamMill parent, String moduleName, int edit, JDialog toCloseOnEdit)
  {
    super(parent, title);
    this.parent = parent;
    this.moduleName = moduleName;
    buf = buf.replaceAll(":", ":\n").replaceAll(":\n\n", ":\n").replaceAll(parent.getLoginName() + "\\$", "");
    this.buf = buf;
    this.toCloseOnEdit = toCloseOnEdit;
    
    Container contentPane = getContentPane();
    
    JTextArea textArea = new JTextArea();
    
    textArea.setEditable(false);
    textArea.setLineWrap(true);
    
    
    JScrollPane s = new JScrollPane(textArea, 
                                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    s.setPreferredSize(new Dimension(600, 500));

    textArea.append(buf);
    
    JButton c = new JButton("Close");
    c.addActionListener(this);
    
    JPanel p = new JPanel();
    p.setLayout(new FlowLayout());
    if(edit == 1) {
      JButton b = new JButton("Edit");
      b.addActionListener(this);
      p.add(b);
    }
    p.add(c);
    contentPane.add(BorderLayout.NORTH, s);
    contentPane.add(BorderLayout.SOUTH, p);
    
    setModal(true);
    pack();
    show();
  }

  public void actionPerformed(ActionEvent e)
  {
    String command = e.getActionCommand();
    
    if(command.equals("Close")) {
      this.setVisible(false);
      this.dispose();
    }
    else if(command.equals("Edit")) {
      StreamMill.currentFile = moduleName;
      StringBuffer buffer = new StringBuffer();
      String[] str = buf.split("\n");
      for(int i= 0; i < str.length; i++) {
        if(!str[i].startsWith("#")) {
          buffer.append(str[i] + "\n");
        }
      }
      
      parent.getEditor().setText(buffer.toString());
      this.setVisible(false);
      this.dispose();
      if(toCloseOnEdit != null) {
        toCloseOnEdit.setVisible(false);
        toCloseOnEdit.dispose();  
      }
    }
  }
}
