package atlas;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.Panel;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;

/*
 */

public class GetPasswordDialog extends JDialog implements ActionListener {

  private TextField pass;
  private StreamMill parent;
  
  public GetPasswordDialog(StreamMill parent, String message, String title) {
      super(parent, title);
      setModal(true);
  
      this.parent = parent;
      
      pass = new TextField();
      pass.setEchoChar('*');
      
      GridLayout whole = new GridLayout(3, 1);
      Container c = getContentPane();
      c.setLayout(whole);
      
      JPanel p1 = new JPanel(new FlowLayout(FlowLayout.LEFT));
      p1.add(new JLabel(message));
      c.add(p1);
    
      JPanel p2 = new JPanel(new FlowLayout(FlowLayout.LEFT));
      p2.add(pass);
      //pass.setSize(30, 30);
      pass.setColumns((int)(message.length()*0.75));
      c.add(p2, 1);
      
      Panel p = new Panel(new FlowLayout(FlowLayout.CENTER));
      JButton ok = new JButton("Ok");
      JButton cancel = new JButton("Cancel");
      
      ok.setDefaultCapable(true);
      
      ok.addActionListener(this);
      cancel.addActionListener(this);
      
      p.add(ok, BorderLayout.LINE_START);
      p.add(cancel, BorderLayout.LINE_END);
      
      c.add(p);
      //this.setSize(630, 130);
      this.setLocationRelativeTo(parent);
      this.getRootPane().setDefaultButton(ok);
      addWindowListener(new WC());
      pack();
      show();
    }
  
    public void doClose(String password) {
      parent.setPassword(password.trim());
      this.dispose();
    }
  
    class WC extends WindowAdapter {
      public void windowClosing(WindowEvent e) {
        GetPasswordDialog.this.doClose("");
      }
    }
  
    public void actionPerformed(ActionEvent e) {
      String command = e.getActionCommand();
    
      if(command.equals("Ok")) {
        doClose(pass.getText());
      }
      else if(command.equals("Cancel")) {
        doClose("");
      }
    }

}
