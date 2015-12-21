
package atlas;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FileDialog;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.Serializable;
import java.io.Writer;
import java.net.ConnectException;
import java.net.Socket;
import java.util.Date;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.util.regex.Pattern;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.JViewport;
import javax.swing.UIManager;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultStyledDocument;
import javax.swing.text.Document;
import javax.swing.text.JTextComponent;
import javax.swing.text.PlainDocument;
import javax.swing.undo.CannotRedoException;
import javax.swing.undo.CannotUndoException;
import javax.swing.undo.UndoManager;

import atlas.event.ITextValueChangeEventListener;
import atlas.event.TextValueChangeEvent;
import atlas.global.AtlasGlobal;
import atlas.gui.AtlasSyntexTextPane;
import atlas.gui.LogoFrame;
import atlas.gui.MenuFrame;
import atlas.monitoring.MonitoringEngineFrame;
import atlas.monitoring.library.MColorMgr;


public class StreamMill extends MenuFrame implements ITextValueChangeEventListener {

  public static String StreamServer = "etna.cs.ucla.edu";

  public static int StreamServerPort = 5431;

  public static final int NOT_LOGGED_IN = 0;

  public static final int LOGGED_IN = 1;

  public static final int LIB_VIEW = 2;

  public static final int VIEW_ALL_QUERIES = 0;

  public static final int VIEW_ALL_STREAMS = 1;

  public static final int VIEW_ALL_TABLES = 2;

  public static final int VIEW_ALL_AGGREGATES = 3;

  public static final int VIEW_ALL_EXTERNS = 4;

  public static final int VIEW_ALL_TS_QUERIES = 5;

  public static final String ADD_QUERIES_COMMAND = "AddQueries";

  public static final String VIEWALL_QUERIES_COMMAND = "ViewAllQueries";

  public static final String VIEW_QUERY_MODULE_COMMAND = "ViewQueryModule";

  public static final String VIEW_QUERY_COMMAND = "ViewQuery";

  public static final String ACTIVATE_QUERY_COMMAND = "ActivateQuery";

  public static final String DEACTIVATE_QUERY_COMMAND = "DeactivateQuery";

  public static final String ACTIVATE_QUERY_MODULE_COMMAND = "ActivateQueryModule";

  public static final String DEACTIVATE_QUERY_MODULE_COMMAND = "DeactivateQueryModule";

  public static final String DELETE_QUERY_MODULE_COMMAND = "DeleteQueryModule";

  public static final String DELETE_QUERY_COMMAND = "DeleteQuery";

  public static final String ADD_TABLES_COMMAND = "AddTables";

  public static final String ADD_STREAMS_COMMAND = "AddStreams";

  public static final String VIEWALL_TABLES_COMMAND = "ViewAllTables";

  public static final String VIEWALL_STREAMS_COMMAND = "ViewAllStreams";

  public static final String VIEW_DECLARE_MODULE_COMMAND = "ViewDeclareModule";

  public static final String VIEW_DECLARE_COMMAND = "ViewDeclare";

  public static final String DELETE_DECLARE_MODULE_COMMAND = "DeleteDeclareModule";

  public static final String DELETE_DECLARE_COMMAND = "DeleteDeclare";

  public static final String ACTIVATE_STREAM_COMMAND = "ActivateStream";

  public static final String DEACTIVATE_STREAM_COMMAND = "DeactivateStream";

  public static final String ACTIVATE_STREAM_MODULE_COMMAND = "ActivateStreamModule";

  public static final String DEACTIVATE_STREAM_MODULE_COMMAND = "DeactivateStreamModule";

  public static final String ADD_AGGREGATE_COMMAND = "AddAggregate";

  public static final String VIEWALL_AGGREGATES_COMMAND = "ViewAllAggregates";

  public static final String VIEW_AGGREGATE_COMMAND = "ViewAggregate";

  public static final String DELETE_AGGREGATE_COMMAND = "DeleteAggregate";

  public static final String ADD_EXTERN_COMMAND = "AddExtern";

  public static final String VIEWALL_EXTERNS_COMMAND = "ViewAllExterns";

  public static final String VIEW_EXTERN_COMMAND = "ViewExtern";

  public static final String DELETE_EXTERN_COMMAND = "DeleteExtern";

  public static final String ADD_TS_QUERY_COMMAND = "AddTSQuery";

  public static final String VIEWALL_TS_QUERIES_COMMAND = "ViewAllTSQueries";

  public static final String VIEW_TS_QUERY_COMMAND = "ViewTSQuery";

  public static final String DELETE_TS_QUERY_COMMAND = "DeleteTSQuery";

  public static final String ACTIVATE_TS_QUERY_COMMAND = "ActivateTSQuery";

  public static final String DEACTIVATE_TS_QUERY_COMMAND = "DeactivateTSQuery";

  public static final String VIEW_BUFFERS_COMMAND = "ViewBuffers";

  public static final String MONITOR_BUFFER_COMMAND = "MonitorBuffer";

  public static final String UNMONITOR_BUFFER_COMMAND = "UnMonitorBuffer";

  public static final String ADD_IOMODULE_COMMAND = "AddIOModule";

  public static final String VIEW_IOMODULES_COMMAND = "ViewIOModules";

  public static final String VIEW_IOMODULE_COMMAND = "ViewIOModule";

  public static final String DROP_IOMODULE_COMMAND = "DropIOModule";

  //public static final String ACTIVATE_IOMODULE_COMMAND =
  // "ActivateIOModule";
  //public static final String DEACTIVATE_IOMODULE_COMMAND =
  // "DeactivateIOModule";

  public static final String VIEW_COMPONENTS_COMMAND = "ViewComponents";

  public static final String VIEW_COMPONENT_DETAILS_COMMAND = "ViewComponentDetails";

  public static final String MOVE_STATEMENT_COMMAND = "MoveStatement";

  public static final String BREAK_COMPONENT_COMMAND = "BreakComponent";

  public static final String MERGE_COMPONENTS_COMMAND = "MergeComponents";

  public static final String SET_COMPONENT_PRIORITY_COMMAND = "SetComponentPriority";

  public static final String DOES_USERNAME_EXIST_COMMAND = "DoesUserExist";

  public static final String ADD_NEW_USER_COMMAND = "AddNewUser";

  public static final String AUTHENTICATE_USER_COMMAND = "AuthenticateUser";

  public static final String MAKE_USER_PUBLIC_COMMAND = "MakeUserPublic";

  public static final String MAKE_USER_PRIVATE_COMMAND = "MakeUserPrivate";

  public static final String VIEW_LIB_COMMAND = "ViewLib";

  public static final String VIEW_ALL_COMMAND = "ViewAll";

  public static final String ONE_TUPLE_TEST_COMMAND = "OneTupleTest";

  public static final String SNAPSHOT_QUERY_COMMAND = "SnapshotQuery";

  //////////////////////////////////////////////////////////////
  //                                                          //
  //                  Variable DEFINITION //
  //                                                          //
  //////////////////////////////////////////////////////////////

  // Before we do anything, read the saved setting
  public static Preference setting = new Preference();

  static {
    try {
      setting.setPaths(StreamMill.class.getClassLoader());
      setting.setSize(StreamMill.class.getClassLoader());
      setting.setOutput_Editor(StreamMill.class.getClassLoader());

    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public static JFrame frame; // StreamMill Interface

  // All StreamMillGUI Component
  public static JTextArea result; // Frame Center - Lower Panel

  //public static JTextArea editor; // Frame Center - Upper Panel
  public static AtlasSyntexTextPane editor;

  public static String currentFile = null;

  public static String currentFilePath = null;

  private JMenuBar menubar; // Frame.setJMenuBar

  private JToolBar toolbar; // Frame North

  private Box statusbox; // Frame South (for status, execTime, rowCount)

  private JScrollPane resultScroller;

  public static JScrollPane editorScroller;

  private JTextField status; // Very bottom of the interface

  private JTextField execTime; // Inside the ststusbox

  private JTextField rowCount;

  private Hashtable commands;

  private Hashtable menuItems;

  public HashMap monitoredBuffers;

  private String loginName = null;

  private String password = null;

  private int userStatus = NOT_LOGGED_IN;

  private Thread bufferViewerThread;

  // Define File
  public static String setFile;

  public static String compileFile; // .cc file

  public static String execFile;

  public static String objectFile; // .o file

  public static String executable;

  // Define Compile and Execution Suffix
  public static final String lastIndexSuffix = ".adl";
  public static final String compileSuffix = ".cc";
  public static final String objectSuffix = ".o";

  // Define Suffix Used In Resource File
  public static final String imageSuffix = "Image";
  public static final String labelSuffix = "Label";
  public static final String actionSuffix = "Action";
  public static final String tipSuffix = "Tooltip";

  // Resource File With .GIF
  public static ResourceBundle resources;

  public static void setCurrentFile(String file, String filePath) {
    currentFile = file;
    currentFilePath = filePath;
  }

  public static void showErrorMessage(String message) {
    StreamMill.getResult().setText(message);
  }

  public static void showHaveToBeLoggedInMessage() {
    showErrorMessage("ERROR: You have to be logged in to be able to perform this action.");
  }

  public static void showLibViewOrLoggedInMessage() {
    showErrorMessage("ERROR: You have to be logged in or you should be viewing library to perform this action.");
  }

  public static void displayNoTextError(String as) {
    showErrorMessage("ERROR: There is no text in the scratch pad above. \nThe system will use the text in the above area as "
        + as + ".");
  }

  public static JFrame getStreamMillFrame() {
    return frame;
  }

  public static JScrollPane getEditorScroller() {
    return editorScroller;
  }

  public static int getRandomNumber() {
    return ((int) Math.random()) % 1000000;
  }

  public static String getValidFileName(String name, String objName) {
    name = JOptionPane.showInputDialog(frame, "Please enter " + objName
        + ":", name.replaceAll(" ", "").replaceAll("\\.", "")
        .replaceAll("-", ""));
    if (name != null && !name.trim().equals("") && name.indexOf(".") < 0
        && name.indexOf(" ") < 0 && name.indexOf("-") < 0) {
      return name;
    }
    return null;
  }

  public int getUserStatus() {
    return userStatus;
  }

  private String getUserStatusText() {
    if (userStatus == NOT_LOGGED_IN) {
      return "Logged out";
    }
    if (userStatus == LOGGED_IN) {
      return "Logged in as " + loginName;
    }
    if (userStatus == LIB_VIEW) {
      return "Viewing lib " + loginName;
    }
    return "Ready!";
  }

  public void setUserStatus(int st) {
    this.userStatus = st;
    status.setText(getUserStatusText());
  }

  public void setLoginName(String name) {
    this.loginName = name;
  }

  public String getEditorDataForSending() {
    String data = StreamMill.this.getEditor().getText().trim();
    if(data.endsWith(";"))
      return data.concat("\n");
    return data.concat(";\n");
  }

  // Check If Resource Exist
  static {
    try {
      resources = ResourceBundle.getBundle("atlas/resources/properties.StreamMill", Locale.getDefault());
    } catch (MissingResourceException mre) {
      System.err.println("resources/StreamMill.properties does NOT EXIST");
      System.exit(1);
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                  Main IMPLEMENTATION //
  //                        //
  //////////////////////////////////////////////////////////////

  public static void main(String[] args) {

    try {
      //Version Check - For Swing
      String vers = System.getProperty("java.version");
      if (vers.compareTo("1.2.2") < 0) {
        System.out.println("StreamMill Interface Will Not Run Properly " + "Without Java VM 1.2.2 or Higher. ");
      }

      //Entire StreamMill Interface

      frame = new StreamMill();
    } catch (Throwable t) {
      System.out.println("uncaught exception: " + t);
      t.printStackTrace();
    }
  }

  public String getLoginName() {
    return loginName;
  }

  public String sendCommandToServerAndReceiveExactReply(String command) {
    StringBuffer buf = new StringBuffer();
    try {
      Socket client = new Socket(StreamMill.StreamServer,
          StreamMill.StreamServerPort);
      DataInputStream is = new DataInputStream(client.getInputStream());

      client.getOutputStream().write(command.getBytes());
      String responseLine;
      while ((responseLine = is.readLine()) != null) {
        buf.append(responseLine + "\n");
      }

      client.getOutputStream().close();
      is.close();
      client.close();
    } 
    catch(ConnectException ex) {
      AtlasGlobal.GetInstance().ShowConnectionError();
      buf.append("Socket Error");
      ex.printStackTrace();     
    }
    catch (IOException exp) {
      AtlasGlobal.GetInstance().ShowConnectionError();
      buf.append("Socket Error");
      exp.printStackTrace();
    }
    return buf.toString();
  }

  public String sendCommandToServerAndReceiveReply(String command) {
    StringBuffer buf = new StringBuffer();
    try {
      Socket client = new Socket(StreamMill.StreamServer,StreamMill.StreamServerPort);
      
      DataInputStream is = new DataInputStream(client.getInputStream());

      client.getOutputStream().write(command.getBytes());
      String responseLine;
      while ((responseLine = is.readLine()) != null) {
        buf.append(responseLine);
      }

      client.getOutputStream().close();
      is.close();
      client.close();
    } 
    catch(ConnectException ex) {
      
      AtlasGlobal.GetInstance().ShowConnectionError();
      
      buf.append("Socket Error");
      ex.printStackTrace();     
    }
    catch (IOException exp) {
      
      AtlasGlobal.GetInstance().ShowConnectionError();
      
      buf.append("Socket Error");
      exp.printStackTrace();
    }
    
    return buf.toString();
  }

  public boolean loginNameDoesNotExist(String name) {
    //Server writes "1\n" to the port if username does not exist
    String reply = sendCommandToServerAndReceiveReply(DOES_USERNAME_EXIST_COMMAND
        + " " + name + " " + name);

    if (reply != null && !reply.equals("")) {
      return true;
    }
    return false;
  }

  private int createNewUserAccount(String loginName, String email,
      String password) {
    String reply = sendCommandToServerAndReceiveReply(ADD_NEW_USER_COMMAND
        + " " + loginName + " " + loginName + " " + email + " "
        + password);
    if (reply != null && !reply.equals("")) {
      JOptionPane
          .showMessageDialog(
              this,
              "Could not create new user "
                  + loginName
                  + ". This is mostly likely because of internal error, so please try to create new user again, "
                  + "if it does not work then contact us. This can also happen if the server is not running.");
      return -1;
    }
    return 0;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public String getPassword() {
    return password;
  }

  private String PromptForPassword(String message, String title) {
    new GetPasswordDialog(this, message, title);
    return getPassword();
  }

  private boolean isValidEmail(String email) {
    ///^([a-zA-Z0-9_\.\-])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z])+$/
    //".+@.+\\..+"
    if (email == null
        || email.length() == 0
        || !email
            .matches("([a-zA-Z0-9_\\.\\-])+@(([a-zA-Z0-9\\-])+\\.)+([a-zA-Z])+")
        || email.indexOf(" ") > 0) {
      return false;
    }
    return true;
  }

  public int createNewAccount(String loginName) {

    String email = JOptionPane.showInputDialog(this,
        "Please enter your valid email address:").trim();

    if (!isValidEmail(email)) {
      JOptionPane
          .showMessageDialog(this,
              "The email address entered is not valid. Please try again.");
      return -1;
    }

    String password = PromptForPassword(
        "Enter password for login "
            + loginName
            + ". Passwords are not encrypted. (atleast 4 characters long).",
        "Enter Password");
    if (password == null || password.indexOf(" ") >= 0
        || password.length() < 4) {
      JOptionPane.showMessageDialog(this,
          "Invalid password. Please try to create new user again.");
      return -1;
    }
    String password1 = PromptForPassword(
        "Re-enter password for login "
            + loginName
            + ". Passwords are not encrypted. (atleast 4 characters long).",
        "Enter Password");
    if (password1 == null || !password.equals(password1)) {
      JOptionPane
          .showMessageDialog(this,
              "Two passwords do not match. Please try to create new user again.");
      return -1;
    }

    return createNewUserAccount(loginName, email, password);
  }

  public int authenticateUser(String loginName, String password) {
    String reply = sendCommandToServerAndReceiveReply(AUTHENTICATE_USER_COMMAND
        + " " + loginName + " " + loginName + " " + password);
    if (reply != null && !reply.equals("")) {
      return -1;
    }
    return 0;
  }

  public void sendMessage(String command) {
    try {
      Socket client = new Socket(StreamMill.StreamServer,
          StreamMill.StreamServerPort);
      int i = 0;
      String buf = "";

      client.getOutputStream().write(command.getBytes());

      //client.setTrafficClass(Socket);

      client.getOutputStream().close();
      client.close();
    } catch (IOException exp) {
      AtlasGlobal.GetInstance().ShowConnectionError();
      exp.printStackTrace();
    }
  }

  // Constructor

  private void setFonts(int size) {
    Font font = new Font(UIManager.getFont("Label.font").getName(),
        Font.PLAIN, size);

    UIManager.put("Frame.font", font);
    UIManager.put("Panel.font", font);
    UIManager.put("OptionPane.messageFont", font);
    UIManager.put("OptionPane.buttonFont", font);
    UIManager.put("Label.font", font);
    UIManager.put("Menu.font", font);
    UIManager.put("MenuItem.font", font);
    UIManager.put("Button.font", font);
    UIManager.put("TextField.font", font);
    UIManager.put("TextArea.font", font);
    UIManager.put("EditorPane.font", font);
    UIManager.put("CheckBox.font", font);
    UIManager.put("ComboBox.font", font);
    UIManager.put("TitledBorder.font", font);
    UIManager.put("TabbedPane.font", font);
    UIManager.put("List.font", font);

    editor.setFont(font);
    result.setFont(font);
  }

  StreamMill() {

    super("StreamMill Interface", "StreamMill");
    // Setup GUI Content

    //
    // Show the logo
    //
    JFrame logoFrame = null;
    int default_waitTime = 1000;
    int waitTime = default_waitTime;

    Date stamp = new Date();
    long prevTime = stamp.getTime();
    
    try {
      logoFrame = new LogoFrame(getResource("LogoImage"));

      try {
        waitTime = Integer.parseInt(getResourceString("LogoWaitTime"));
      } catch (Exception ex) {
        ex.printStackTrace();
        waitTime = 1000;
      }
    } catch (Exception e) {
      e.printStackTrace();
    }

    MColorMgr.GetInstance();
    
    chooser = new JFileChooser();

    chooser.setCurrentDirectory(new File("."));

    setBackground(Color.lightGray);
    getContentPane().setLayout(new BorderLayout());
    getContentPane().add("Center", new StreamMillGUI());

    this.loginName = null;
    this.userStatus = NOT_LOGGED_IN;

    // Setup MenuBar
//    menubar = createMenubar();
//    setJMenuBar(menubar);

    // Add Window Listener
    addWindowListener(new StreamMillClose());
    pack();

    // To set size, read from the file "Size.dat"
    Integer int1 = new Integer(setting.getStreamMillWidth());
    Integer int2 = new Integer(setting.getStreamMillHeight());

    setSize(int1.intValue(), int2.intValue());

    // Place it at Center of Screen
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    Dimension size = getSize();
    screenSize.height = screenSize.height / 2;
    screenSize.width = screenSize.width / 2;
    size.height = size.height / 2;
    size.width = size.width / 2;
    int y = screenSize.height - size.height;
    int x = screenSize.width - size.width;
    setLocation(x, y);

    //listener to the message change form AtlasGlobal
    AtlasGlobal.GetInstance().AddTextValueChangeEventListener(this);
    
    if (logoFrame != null) {
      try {
        waitTime = waitTime - (int)(stamp.getTime() - prevTime);
      
        if(waitTime>0) {        
          Thread.sleep(waitTime);
        }
      } catch (Exception ex) {}
      logoFrame.setVisible(false);
      logoFrame.dispose();
      //new LoginAction().actionPerformed(null);
    }
    
    show();
    
    //new ViewMonitoringAction().actionPerformed(null);

  }//End: StreamMill Constructor

  // Winodw Close Application Event
  protected static final class StreamMillClose extends WindowAdapter {
    public void windowClosing(WindowEvent e) {
      System.exit(0);
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                  StreamMillGUI IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  class StreamMillGUI extends JPanel {

    // Constructor
    StreamMillGUI() {

      super(true);

      // Set Cross Platform Look and Feel Using Swing
      try {
        UIManager.setLookAndFeel(UIManager
            .getCrossPlatformLookAndFeelClassName());
        UIManager.setLookAndFeel(UIManager
            .getSystemLookAndFeelClassName());
      } catch (Exception exc) {
        System.err
            .println("Error Loading Cross Platform Look and Feel: "
                + exc);
      }

      // JPanel Setup
      setBorder(BorderFactory.createEtchedBorder());
      setLayout(new BorderLayout());

      // Create Embedded JTextComponent - Editor
      editor = new AtlasSyntexTextPane();
      editorScroller = createScollPane(editor);
      editor.getDocument().addUndoableEditListener(undoHandler);
      editorScroller.setPreferredSize(new Dimension(250, 250));

      StreamServer = tokenize(getResourceString("StreamServer"))[0];
      StreamServerPort = Integer
          .parseInt(tokenize(getResourceString("StreamServerPort"))[0]);

      // Create Embedded JTextComponent - Result
      result = new JTextArea();
      result.setForeground(Color.RED);
      setFonts(Integer
          .parseInt(tokenize(getResourceString("fontSize"))[0]));
      result.setEditable(false);
      resultScroller = createScollPane(result);
      resultScroller.setBorder(BorderFactory.createCompoundBorder(
          BorderFactory.createCompoundBorder(BorderFactory
              .createTitledBorder("Results"), BorderFactory
              .createEmptyBorder(1, 1, 1, 1)), resultScroller
              .getBorder()));

    
      //      // Setup Command Table For Menu
      //      commands = new Hashtable();
      //      Action[] actions = getActions();
      //      for (int i = 0; i < actions.length; i++) {
      //        Action act = actions[i];
      //        commands.put(act.getValue(Action.NAME), act);
      //      }

      // Panel - Editor and Result Panel
      JPanel panel = new JPanel();
      BoxLayout panelLayout = new BoxLayout(panel, BoxLayout.Y_AXIS);
      panel.setLayout(panelLayout);
      panel.add(editorScroller);
      panel.add(resultScroller);

      setJMenuBar(createMenubar());
      add("North", createToolbar());
      add("Center", panel); //panel consists of editor and result
                  // textarea
      add("South", createStatusbox());
    }
  }

  // Create Scroller Panel For TextComponent
  protected JScrollPane createScollPane(JTextComponent panel) {

    JScrollPane scroller = new JScrollPane();
    JViewport port = scroller.getViewport();

    //panel.setFont(new Font("monospaced", Font.PLAIN, 12));
    //scroller.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

    port.add(panel);

    try {
      String vpFlag = resources.getString("ViewportBackingStore");
      Boolean bs = new Boolean(vpFlag);
      port.setBackingStoreEnabled(bs.booleanValue());
    } catch (MissingResourceException mre) {
      // Using the Viewport Default
    }

    return scroller;
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                  StatusBox IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  private Component createStatusbox() {

    // TEST Status Bar
    statusbox = new Box(BoxLayout.X_AXIS);
    status = new JTextField("Ready!", 100);
    execTime = new JTextField("Exec Time: 0:00:00", 40);
    rowCount = new JTextField("0 Row", 15);

    status.setEnabled(false);
    execTime.setEnabled(false);
    rowCount.setEnabled(false);
    statusbox.add(status);
    statusbox.add(execTime);
    statusbox.add(rowCount);

    return statusbox;
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                  Action IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  // Action Defined
  public static final String openAction = "open";

  public static final String setAction = "set";

  public static final String newAction = "new";

  public static final String saveAction = "save";

  public static final String saveAsAction = "saveAs";

  public static final String saveResultAction = "saveResult";

  public static final String printAction = "print";

  public static final String printResultAction = "printResult";

  public static final String closeAction = "close";

  public static final String exitAction = "exit";

  public static final String compileAction = "compile";

  public static final String execAction = "exec";

  public static final String newUserAction = "newUser";

  public static final String loginAction = "login";

  public static final String makePublicAction = "makePublic";

  public static final String makePrivateAction = "makePrivate";

  public static final String logoutAction = "logout";

  public static final String changeUserAction = "changeUser";

  public static final String RegisterDecAction = "registerDec";

  public static final String ViewTableDecAction = "viewTableDec";

  public static final String RegisterStreamDecAction = "registerStreamDec";

  public static final String ViewStreamDecAction = "viewStreamDec";

  public static final String RegisterAggrAction = "registerAggr";

  public static final String ViewAggrAction = "viewAggr";

  public static final String RegisterExternAction = "registerExtern";

  public static final String ViewExternAction = "viewExtern";

  public static final String RegisterTSAction = "registerTS";

  public static final String ViewTSAction = "viewTS";

  public static final String RegisterQueAction = "registerQue";

  public static final String ViewQueAction = "viewQue";

  public static final String ViewBufsAction = "viewBufs";

  public static final String viewMonitoringAction = "viewMonitoring";

  public static final String AddIOModuleAction = "addIOModule";

  public static final String ViewIOModulesAction = "viewIOModules";

  public static final String ViewComponentsAction = "viewComponents";

  public static final String viewLibAction = "viewLib";

  public static final String viewAllAction = "viewAll";

  public static final String oneTupleTestAction = "oneTupleTest";

  public static final String SnapshotAction = "snapshot";

  //  public static final String stopAction = "stop"; // FUTURE OPTION
  //  public static final String textResultAction = "textResult"; // FUTURE
  // OPTION
  //  public static final String gridResultAction = "gridResult"; // FUTURE
  // OPTION
  public static final String clearAction = "clear";

  public static final String aboutAction = "about";

  public static final String docAction = "doc";

  public static final String examplesAction = "examples";

  public static final String prefAction = "pref";

  private UndoAction undoAction = new UndoAction();

  private RedoAction redoAction = new RedoAction();

  protected FileDialog fileDialog;

  protected JFileChooser chooser;

  protected UndoableEditListener undoHandler = new UndoHandler();

  protected UndoManager undo = new UndoManager();

  public static JFrame prefSetting;

  // 0. Default Execution Message
  // 1. Loading File To Editor
  // 2. Compiling File
  // 3. Executing File
  private static int execCode = 0;

  public static int getExecCode() {
    return execCode;
  }

  protected AtlasSyntexTextPane getEditor() {
    return editor;
  }

  private static boolean cleanPane = true;

  public static boolean getCleanPane() {
    return cleanPane;
  }

  public static JTextArea getResult() {
//    int temp = 0;
//    try {
//      setting.setOutput_Editor(StreamMill.class.getClassLoader());
//      temp = setting.getOutput_type();
//    } catch (Exception e5) {
//    }
//
//    if (temp == 0)
//      return editor;
//    else
      //if user did not specify to print output to the editor
      return result;
  }

  // Yanked from JMenu, ideally this would be public.
  private class ActionChangedListener implements PropertyChangeListener {

    JMenuItem menuItem;

    ActionChangedListener(JMenuItem mi) {
      super();
      this.menuItem = mi;
    }

    public void propertyChange(PropertyChangeEvent e) {

      String propertyName = e.getPropertyName();

      if (e.getPropertyName().equals(Action.NAME)) {
        String text = (String) e.getNewValue();
        menuItem.setText(text);
      } else if (propertyName.equals("enabled")) {
        Boolean enabledState = (Boolean) e.getNewValue();
        menuItem.setEnabled(enabledState.booleanValue());
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //    New IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class NewAction extends AbstractAction {

    public NewAction() {
      super(newAction);
    }

    public NewAction(String nm) {
      super(nm);
    }

    public void actionPerformed(ActionEvent e) {
      Document oldDoc = getEditor().getDocument();
      if (oldDoc != null)
        oldDoc.removeUndoableEditListener(undoHandler);
      getEditor().setDocument(new PlainDocument());
      getEditor().getDocument().addUndoableEditListener(undoHandler);
      Frame frame = StreamMill.this;
      frame.setTitle("Untitled");
      validate();
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                 Exit ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class ExitAction extends AbstractAction {

    public ExitAction() {
      super(exitAction);
    }

    public void actionPerformed(ActionEvent e) {
      System.exit(0);
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 Undo ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  public class UndoHandler implements UndoableEditListener {

    /**
     * Messaged when the Document has created an edit, the edit is added to
     * <code>undo</code>, an instance of UndoManager.
     */
    public void undoableEditHappened(UndoableEditEvent e) {
      undo.addEdit(e.getEdit());
      undoAction.update();
      redoAction.update();
    }
  }

  public class UndoAction extends AbstractAction {
    public UndoAction() {
      super("Undo");
      setEnabled(false);
    }

    public void actionPerformed(ActionEvent e) {
      try {
        undo.undo();
      } catch (CannotUndoException ex) {
        System.out.println("Unable to undo: " + ex);
        ex.printStackTrace();
      }
      update();
      redoAction.update();
    }

    protected void update() {
      if (undo.canUndo()) {
        setEnabled(true);
        putValue(Action.NAME, undo.getUndoPresentationName());
      } else {
        setEnabled(false);
        putValue(Action.NAME, "Undo");
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                 Redo ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class RedoAction extends AbstractAction {
    public RedoAction() {
      super("Redo");
      setEnabled(false);
    }

    public void actionPerformed(ActionEvent e) {
      try {
        undo.redo();
      } catch (CannotRedoException ex) {
        System.out.println("Unable to redo: " + ex);
        ex.printStackTrace();
      }
      update();
      undoAction.update();
    }

    protected void update() {
      if (undo.canRedo()) {
        setEnabled(true);
        putValue(Action.NAME, undo.getRedoPresentationName());
      } else {
        setEnabled(false);
        putValue(Action.NAME, "Redo");
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                 Set ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class SetAction extends NewAction implements Serializable {

    public SetAction() {
      super(setAction);
    }

    public void actionPerformed(ActionEvent e) {

      Frame frame = StreamMill.this;

      if (fileDialog == null) {
        fileDialog = new FileDialog(frame);
      }

      fileDialog.setMode(FileDialog.LOAD);
      fileDialog.show();

      String file = fileDialog.getFile();

      if (file == null) {
        return;
      }

      String directory = fileDialog.getDirectory();
      setFile = directory + file;
      File f = new File(directory, file);

      if (f.exists()) {
        Document oldDoc = getEditor().getDocument();
        if (oldDoc != null)
          oldDoc.removeUndoableEditListener(undoHandler);
      }

      getEditor().setDocument(new PlainDocument());
      frame.setTitle(setFile);
      directOpenFile = true;

    }
  }

  private boolean directOpenFile = false;

  //////////////////////////////////////////////////////////////
  //                  //
  //                 Open ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  private void readObject(ObjectInputStream in) throws IOException {

    try {
      in.defaultReadObject();
    } catch (IOException io) {
      // should put in status panel
      System.err.println("IOException: " + io.getMessage());
    } catch (ClassNotFoundException cnf) {
      // should put in status panel
      System.err.println("Class not found: " + cnf.getMessage());
    }
  }

  private void writeObject(ObjectOutputStream out) throws IOException {

    try {
      out.defaultWriteObject();
    } catch (IOException io) {
      // should put in status panel
      System.err.println("IOException: " + io.getMessage());
    }
  }

  public class OpenAction extends NewAction implements Serializable {
    File f;

    public OpenAction() {
      super(openAction);
    }

    public void actionPerformed(ActionEvent e) {
      Frame frame = StreamMill.this;

      if ((setFile == null) || (directOpenFile == false)) {

        if (fileDialog == null) {
          fileDialog = new FileDialog(frame);
        }

        fileDialog.setMode(FileDialog.LOAD);
        fileDialog.show();
        String file = fileDialog.getFile();

        if (file == null) {
          return;
        }

        String directory = fileDialog.getDirectory();
        setFile = directory + file;
        f = new File(directory, file);

        if (f.exists()) {
          Document oldDoc = getEditor().getDocument();
          if (oldDoc != null)
            oldDoc.removeUndoableEditListener(undoHandler);
        }
        getEditor().setStyledDocument(new DefaultStyledDocument());
        frame.setTitle(setFile);
      }

      try {
        setting.setOutput_Editor(StreamMill.class.getClassLoader());
        if (setting.getEditor_type() == 1) {
          getResult().setText("");
          cleanPane = true;
          Exec.exec("xterm -e vi" + " " + setFile);
        } else if (setting.getEditor_type() == 2) {
          getResult().setText("");
          cleanPane = true;
          Exec.exec("emacs" + " " + setFile);
        } else {
          if (setFile != null)
            f = new File(setFile);
          getResult().setText("");
          cleanPane = true;
          Thread loader = new FileLoader(f, editor.getDocument());
          loader.start();
          StreamMill.setCurrentFile(f.getName(), setFile);
        }
      } catch (Exception e4) {
      }

      directOpenFile = false;
    }
  }

  class FileLoader extends Thread {

    Document doc;

    File f;

    FileLoader(File f, Document doc) {
      setPriority(4);
      this.f = f;
      this.doc = doc;
    }

    public void run() {
      try {
        long start = startTime();
        statusbox.removeAll();

        execTime = new JTextField("Loading File...", 40);
        execTime.setEnabled(false);

        rowCount = new JTextField("0 Row", 15);
        rowCount.setEnabled(false);

        /*
         * JProgressBar progressBar = new JProgressBar();
         * progressBar.setBorder(BorderFactory.createEtchedBorder());
         * progressBar.setMinimum(0); progressBar.setMaximum((int)
         * f.length());
         * 
         * statusbox.add(progressBar);
         * statusbox.add(Box.createHorizontalStrut(150));
         * statusbox.add(execTime); statusbox.add(rowCount);
         * statusbox.validate();
         */

        Reader in = new FileReader(f);
        char[] buff = new char[4096];
        int nch;

        while ((nch = in.read(buff, 0, buff.length)) != -1) {
          doc.insertString(doc.getLength(), new String(buff, 0, nch),
              null);
          //progressBar.setValue(progressBar.getValue() + nch);
        }

        doc.addUndoableEditListener(undoHandler);
        execCode = 1;
        showTime(start);
      } catch (IOException ioe) {
        System.err.println(ioe.toString());
      } catch (BadLocationException ble) {
        System.err.println(ble.getMessage());
      }
    }
  }

  protected long startTime() {
    return System.currentTimeMillis();
  }

  protected void showTime(long startTime) {
    if (execCode == 1) {
      execTime.setText("File Load Time: " + elapsedTime(startTime)
          + " sec.");
    } else if (execCode == 2) {
      execTime.setText("Compilation Time: " + elapsedTime(startTime)
          + " sec.");
    } else if (execCode == 3) {
      execTime.setText("Execution Time: " + elapsedTime(startTime)
          + " sec.");
    }
  }

  protected double elapsedTime(long startTime) {
    double delta = (double) (System.currentTimeMillis() - startTime);
    return (delta / 1000.0);
  }

  
   public void TextValueChangeEventOccurred(TextValueChangeEvent e) {
    if(e.GetName().equals(AtlasGlobal.MESSAGE)) {
       showErrorMessage(e.GetValue());
    }
   }
  
  //////////////////////////////////////////////////////////////
  //                  //
  //                 Save ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class SaveAction extends AbstractAction implements Serializable {

    public SaveAction() {
      super(saveAction);
    }

    public void actionPerformed(ActionEvent e) {
      Frame frame = StreamMill.this;

      String file = StreamMill.currentFilePath;

      if (file == null) {
        return;
      }

      File f = new File(file);

      try {
        //StreamMill.this.setTitle(f.getAbsolutePath());
        Writer out = new FileWriter(f);
        //    String editorText = new String();
        /*
         * char[] buff = new char[4096]; int nch;
         */
        getEditor().write(out);
        /*
         * while((nch = in.read(buff, 0, buff.length)) != -1){
         * out.write(buff, 0, buff.length);
         * doc.insertString(doc.getLength(), new String(buff, 0, nch),
         * null); progressBar.setValue(progressBar.getValue() + nch); }
         */
        /*
         * FileOutputStream fstrm = new FileOutputStream(f);
         * ObjectOutputStream ostrm = new ObjectOutputStream(fstrm);
         * ostrm.writeObject(getEditor().getDocument()); ostrm.flush();
         * ostrm.close();
         */
      } catch (IOException io) {
        System.err.println("IOException: " + io.getMessage());
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 Save ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 SaveAs ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class SaveAsAction extends AbstractAction implements Serializable {

    public SaveAsAction() {
      super(saveAsAction);
    }

    public void actionPerformed(ActionEvent e) {
      Frame frame = StreamMill.this;

      if (fileDialog == null) {
        fileDialog = new FileDialog(frame);
      }

      fileDialog.setMode(FileDialog.SAVE);
      fileDialog.setFile(StreamMill.currentFile);
      fileDialog.show();
      String file = fileDialog.getFile();

      if (file == null) {
        return;
      }

      String directory = fileDialog.getDirectory();
      File f = new File(directory, file);

      try {
        StreamMill.this.setTitle(directory + file);
        Writer out = new FileWriter(f);
        //    String editorText = new String();
        /*
         * char[] buff = new char[4096]; int nch;
         */
        StreamMill.setCurrentFile(f.getName(), directory + file);
        frame.setTitle(directory);
        getEditor().write(out);
        /*
         * while((nch = in.read(buff, 0, buff.length)) != -1){
         * out.write(buff, 0, buff.length);
         * doc.insertString(doc.getLength(), new String(buff, 0, nch),
         * null); progressBar.setValue(progressBar.getValue() + nch); }
         */
        /*
         * FileOutputStream fstrm = new FileOutputStream(f);
         * ObjectOutputStream ostrm = new ObjectOutputStream(fstrm);
         * ostrm.writeObject(getEditor().getDocument()); ostrm.flush();
         * ostrm.close();
         */
      } catch (IOException io) {
        System.err.println("IOException: " + io.getMessage());
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 SaveAs ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 Close ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class CloseAction extends AbstractAction implements Serializable {

    public CloseAction() {
      super(closeAction);
    }

    public void actionPerformed(ActionEvent e) {
      Frame frame = StreamMill.this;
      if (StreamMill.currentFilePath != null) {
        int opt = JOptionPane.showConfirmDialog(StreamMill.this,
            "Do you want to save \""
                + StreamMill.currentFilePath + "\"?",
            "Save file?", JOptionPane.YES_NO_OPTION);

        if (opt == JOptionPane.YES_OPTION) {
          File f = new File(StreamMill.currentFilePath);
          try {
            Writer out = new FileWriter(f);
            getEditor().write(out);
          } catch (IOException io) {
            System.err.println("IOException: " + io.getMessage());
          }
        }
        getEditor().setText("");
        StreamMill.setCurrentFile(null, null);
        frame.setTitle("StreamMill Interface");
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 Close ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                //
  //               NewUser IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class NewUserAction extends AbstractAction implements Serializable {

    public NewUserAction() {
      super(newUserAction);
    }

    public void actionPerformed(ActionEvent e) {
      String loginName = JOptionPane
          .showInputDialog(
              StreamMill.this,
              "Please specify a loginName:\n"
                  + "LoginName cannot be empty and it should not contain spaces.",
              "LoginName", JOptionPane.QUESTION_MESSAGE);
      if (loginName == null || loginName.trim().equals("")
          || loginName.indexOf(" ") >= 0) {
        showErrorMessage("Invalid loginName. Please try to create new user again.");
      } else if (!loginNameDoesNotExist(loginName)) {
        showErrorMessage("LoginName already exists. Please try another loginName.");
      } else {
        //TODO: Also take their email address
        int rc = createNewAccount(loginName);
        if (rc >= 0) {
          StreamMill.this.setLoginName(loginName);
          StreamMill.this.setUserStatus(LOGGED_IN);
          StreamMill.getResult().setText(
              "User " + loginName + " successfully created. You are logged in as " + loginName + ".");
        } else {
          StreamMill.this.setLoginName(null);
          StreamMill.this.setUserStatus(NOT_LOGGED_IN);
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //              //
  //             NewUser IMPLEMENTATION //
  //              //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                //
  //               LOGIN IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class LoginAction extends AbstractAction implements Serializable {

    public LoginAction() {
      super(loginAction);
    }

    public void actionPerformed(ActionEvent e) {
      String loginName = JOptionPane.showInputDialog(StreamMill.this,
          "Enter Your LoginName:", "Enter Login",
          JOptionPane.QUESTION_MESSAGE);
      if (loginName == null || loginName.trim().equals("")
          || loginName.indexOf(" ") >= 0) {
        showErrorMessage("Invalid loginName. Please try to login again.");
      } else if (loginNameDoesNotExist(loginName)) {
        showErrorMessage("LoginName does not exist. Please try to login again.");
      } else {
        String password = PromptForPassword("Enter password for login "
            + loginName
            + ". Passwords must be atleast 4 characters long.",
            "Enter Password");
        if (password == null || password.indexOf(" ") >= 0
            || password.length() < 4) {
          showErrorMessage("Invalid password. Please try to login again.");
        } else {
          int rc = authenticateUser(loginName, password);
          if (rc < 0)
            showErrorMessage("Invalid password. Please try to login again");
          else {
            StreamMill.this.setLoginName(loginName);
            StreamMill.this.setUserStatus(LOGGED_IN);
            StreamMill.getResult().setText(
                "Successfully logged in as " + loginName);
          }
        }
      }
      
      //**********************
      //* fate login (development purpose)
      //***********************
//      String loginName = "treetree";
//
//      StreamMill.this.setLoginName(loginName);
//      StreamMill.this.setUserStatus(LOGGED_IN);
//      StreamMill.getResult().setText(
//            "Successfully logged in as " + loginName);
    }
  }

  //////////////////////////////////////////////////////////////
  //              //
  //             LOGIN IMPLEMENTATION //
  //              //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                //
  //               MakePublic IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class MakePublicAction extends AbstractAction implements
      Serializable {

    public MakePublicAction() {
      super(makePublicAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      String command = StreamMill.MAKE_USER_PUBLIC_COMMAND + " "
          + StreamMill.this.getLoginName() + " "
          + StreamMill.this.getLoginName();
      String reply = sendCommandToServerAndReceiveReply(command);
      if (reply != null && !reply.trim().equals("")) {
        showErrorMessage("ERROR: Unable to make user public.\n"
            + "This could be an internal error.  Restart the client and retry.");
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //              //
  //             MakePublic IMPLEMENTATION //
  //              //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                //
  //               MakePrivate IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class MakePrivateAction extends AbstractAction implements
      Serializable {

    public MakePrivateAction() {
      super(makePrivateAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      String command = StreamMill.MAKE_USER_PRIVATE_COMMAND + " "
          + StreamMill.this.getLoginName() + " "
          + StreamMill.this.getLoginName();
      String reply = sendCommandToServerAndReceiveReply(command);
      if (reply != null && !reply.trim().equals("")) {
        showErrorMessage("ERROR: Unable to make user private.\n"
            + "This could be an internal error.  Restart the client and retry.");
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //              //
  //             MakePrivate IMPLEMENTATION //
  //              //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                //
  //               LOGOUT IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class LogoutAction extends AbstractAction implements Serializable {

    public LogoutAction() {
      super(logoutAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      //TODO: Deactivate user queries here
      StreamMill.this.setLoginName(null);
      StreamMill.this.setUserStatus(NOT_LOGGED_IN);
    }
  }

  //////////////////////////////////////////////////////////////
  //              //
  //             LOGOUT IMPLEMENTATION //
  //              //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                //
  //               ChangeUser IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ChangeUserAction extends AbstractAction implements
      Serializable {

    public ChangeUserAction() {
      super(changeUserAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      String loginName = JOptionPane.showInputDialog(StreamMill.this,
          "Enter Your LoginName:", "Enter Login",
          JOptionPane.QUESTION_MESSAGE);
      if (loginName == null || loginName.trim().equals("")
          || loginName.indexOf(" ") >= 0) {
        showErrorMessage("Invalid loginName. Please try to login again.");
      } else if (loginNameDoesNotExist(loginName)) {
        showErrorMessage("LoginName does not exist. Please try to login again.");
      } else {
        String password = JOptionPane
            .showInputDialog(
                StreamMill.this,
                "Enter password for login "
                    + loginName
                    + ".\nPasswords must be atleast 4 characters long.",
                "Enter Password", JOptionPane.QUESTION_MESSAGE);
        if (password == null || password.indexOf(" ") >= 0
            || password.length() < 4) {
          showErrorMessage("Invalid password. Please try to login again.");
        } else {
          int rc = authenticateUser(loginName, password);
          if (rc < 0)
            showErrorMessage("Invalid password. Please try to login again");
          else {
            StreamMill.this.setLoginName(loginName);
            StreamMill.this.setUserStatus(LOGGED_IN);
          }
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //              //
  //             ChangeUser IMPLEMENTATION //
  //              //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                //
  //               ViewLib IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewLibAction extends AbstractAction implements Serializable {

    public ViewLibAction() {
      super(viewLibAction);
    }

    public void actionPerformed(ActionEvent e) {
      String command;
      if (StreamMill.this.getLoginName() != null)
        command = StreamMill.VIEW_LIB_COMMAND + " "
            + StreamMill.this.getLoginName();
      else
        command = StreamMill.VIEW_LIB_COMMAND + " abc";
      String reply = sendCommandToServerAndReceiveReply(command);
      if (reply == null) {
        showErrorMessage("No Libraries defined at this point.");
        return;
      }
      String[] libs = reply.split("\\|\\|");
      String libName;
      if (libs.length == 0) {
        showErrorMessage("No Libraries defined at this point.");
      } else {
        libName = (String) JOptionPane.showInputDialog(StreamMill.this,
            "Select library to view:", "Select Library",
            JOptionPane.QUESTION_MESSAGE, null, libs, "");
        if (!(libName == null || libName.trim().equals(""))) {
          StreamMill.this.setLoginName(libName);
          StreamMill.this.setUserStatus(LIB_VIEW);
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //              //
  //             ViewLib IMPLEMENTATION //
  //              //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 REGISTERAGGR IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class RegisterAggrAction extends AbstractAction implements
      Serializable {

    public RegisterAggrAction() {
      super(RegisterAggrAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }

      if (StreamMill.this.getEditor().getText() == null || StreamMill.this.getEditor().getText().trim().equals("")) {
        displayNoTextError("code for the aggregate");
        return;
      }
      
      String dataToSend = getEditorDataForSending();
      String fileName = StreamMill.currentFilePath;

      /*
       * if(fileName == null || fileName.trim().equals("")) { fileName =
       * "aggr_" + getRandomNumber(); } else if(fileName.indexOf(".") >=
       * 0){ fileName = fileName.substring(0, fileName.indexOf(".")); }
       * 
       * fileName = getValidFileName(fileName, "the name of the
       * aggregate"); if(fileName == null) { showErrorMessage("Invalid
       * aggregate name, please try again."); return; } fileName =
       * fileName + ".aggr";
       */

      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        OutputStream outToServer = client.getOutputStream();
        String command = ADD_AGGREGATE_COMMAND + " "
            + StreamMill.this.loginName + " "; // + fileName + "\n";
                               // instead find the
                               // name automatically

        outToServer.write(command.getBytes());
        outToServer.write(dataToSend.getBytes());

        DataInputStream is = new DataInputStream(client
            .getInputStream());
        String responseLine = null;
        StringBuffer wholeMessage = new StringBuffer();

        while ((responseLine = is.readLine()) != null) {
          wholeMessage.append(responseLine + "\n");
        }
        if (!wholeMessage.toString().trim().equals("")) {
          getResult().setText(
              "ERROR: Could not create aggregate. "
                  + "Error message from server is:\n"
                  + wholeMessage.toString() + "\n");
        } else {
          getResult().setText("Aggregate successfully created.");
        }

        is.close();
        outToServer.close();
        client.close();
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 REGISTERAGGR IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 REGISTEREXTERN IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public static String validateExternDec(String declaration) {
    Pattern p = Pattern.compile("\\W");
    String[] vals = p.split(declaration);
    String retVal = null;

    if (vals.length < 3) {
      retVal = "Invalid declaration.";
    }
    if (!vals[0].trim().equalsIgnoreCase("external")) {
      retVal = "First word in external function declaration must be \"external\".";
    }

    int index = declaration.indexOf('(');
    if (index <= 0) {
      retVal = "Invalid declaration - no '(' found.";
    }
    int count = 1;
    int i = index + 1;
    while (i < declaration.length()) {
      if (declaration.charAt(i) == ')') {
        count--;
      } else if (declaration.charAt(i) == '(') {
        count++;
      }
      i++;
    }
    if (i == declaration.length() && count != 0) {
      retVal = "Invalid declaration- number of parantheses don't match.";
    }

    return retVal;
  }

  public static String getFunctionName(String declaration) {
    Pattern p = Pattern.compile("\\W");
    String[] valsp = p.split(declaration);
    String[] vals = declaration.split(" ");
    String retVal = null;

    if (valsp[1].trim().equalsIgnoreCase("table")) {

      int index = declaration.indexOf('(');
      int count = 1;
      int i = index + 1;
      while (count > 0 && i < declaration.length()) {
        if (declaration.charAt(i) == ')') {
          count--;
        } else if (declaration.charAt(i) == '(') {
          count++;
        }
        i++;
      }
      String tabName = declaration.substring(i);
      int idxOfParan = tabName.indexOf('(');
      if (idxOfParan <= 0) {
        return null;
      }
      retVal = tabName.substring(0, idxOfParan).trim();
    } else if (valsp[1].trim().equalsIgnoreCase("char")) {
      /*
       * for(int x=0; x < 5; x++) { System.out.println(valsp[x].trim() + " " +
       * vals[x].trim()); }
       */

      if (vals[2].trim().startsWith("(")) {
        String canTab = vals[3].trim();
        if (canTab.indexOf('(') > 0) {
          retVal = canTab.substring(0, canTab.indexOf('('));
        } else
          retVal = canTab;
      } else if (vals[1].trim().indexOf('(') > 0)
        retVal = valsp[4].trim();
      else
        retVal = valsp[2].trim();
    } else {
      retVal = valsp[2].trim();
    }
    return retVal;
  }

  public class RegisterExternAction extends AbstractAction implements
      Serializable {

    public RegisterExternAction() {
      super(RegisterExternAction);
    }

    public void actionPerformed(ActionEvent e) {
      Frame frame = StreamMill.this;
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      
      if (StreamMill.this.getEditor().getText() == null || StreamMill.this.getEditor().getText().trim().equals("")) {
        displayNoTextError("extern defnition");
        return;
      }
      
      String dataToSend = getEditorDataForSending();

      String err = validateExternDec(dataToSend);
      String funcName = null;
      if (err == null) {
        funcName = getFunctionName(dataToSend);
        if (funcName == null) {
          showErrorMessage("Invalid external function declaration.");
          return;
        }
      } else {
        showErrorMessage(err);
        return;
      }

      if (fileDialog == null) {
        fileDialog = new FileDialog(frame);
      }

      fileDialog.setMode(FileDialog.LOAD);
      fileDialog.show();
      String file = fileDialog.getFile();

      if (file == null) {
        return;
      }

      String directory = fileDialog.getDirectory();
      File f = new File(directory, file);

      try {
        Reader in = new FileReader(f);
        char[] buff = new char[4096];
        int nch;
        String funcDef = "";

        while ((nch = in.read(buff, 0, buff.length)) != -1) {
          funcDef = funcDef + new String(buff, 0, nch);
        }
        in.close();

        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        OutputStream outToServer = client.getOutputStream();
        String command = ADD_EXTERN_COMMAND + " "
            + StreamMill.this.loginName + " " + funcName + "\n"; // +
                                       // fileName
                                       // +
                                       // "\n";
                                       // instead
                                       // find
                                       // the
                                       // name
                                       // automatically

        outToServer.write(command.getBytes());
        outToServer.write(dataToSend.getBytes());
        outToServer.write("\n".getBytes());
        outToServer.write(funcDef.getBytes());

        DataInputStream is = new DataInputStream(client
            .getInputStream());
        String responseLine = null;
        StringBuffer wholeMessage = new StringBuffer();

        while ((responseLine = is.readLine()) != null) {
          wholeMessage.append(responseLine + "\n");
        }
        if (!wholeMessage.toString().trim().equals("")) {
          getResult().setText(
              "ERROR: Could not define extern. "
                  + "Error message from server is:\n"
                  + wholeMessage.toString() + "\n");
        } else {
          getResult().setText("Extern successfully defined.");
        }

        is.close();
        outToServer.close();
        client.close();
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  
  
  
  //////////////////////////////////////////////////////////////
  //                        //
  //                 REGISTEREXTERN IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 REGISTERDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class RegisterDecAction extends AbstractAction implements
      Serializable {

    public RegisterDecAction() {
      super(RegisterDecAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      
      if (StreamMill.this.getEditor().getText() == null || StreamMill.this.getEditor().getText().trim().equals("")) {
        displayNoTextError("declaration(S)");
        return;
      }
      
      String dataToSend = getEditorDataForSending();
      String fileName = StreamMill.currentFilePath;


      if (fileName == null || fileName.trim().equals("")) {
        fileName = "dcl_" + getRandomNumber();
      } else if (fileName.indexOf(".") >= 0) {
        fileName = fileName.substring(0, fileName.indexOf("."));
      }

      fileName = getValidFileName(fileName,
          "a valid name for the declare module");
      if (fileName == null) {
        showErrorMessage("Invalid module name, please try again.");
        return;
      }
      fileName = fileName + ".dcl";

      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        OutputStream outToServer = client.getOutputStream();
        String command = ADD_TABLES_COMMAND + " "
            + StreamMill.this.loginName + " " + fileName + "\n";

        outToServer.write(command.getBytes());
        outToServer.write(dataToSend.getBytes());
        DataInputStream is = new DataInputStream(client
            .getInputStream());
        String responseLine = null;
        StringBuffer wholeMessage = new StringBuffer();

        while ((responseLine = is.readLine()) != null) {
          wholeMessage.append(responseLine + "\n");
        }
        if (!wholeMessage.toString().trim().equals("")) {
          getResult().setText(
              "ERROR: Could not register tables. "
                  + "Error message from server is:\n"
                  + wholeMessage.toString() + "\n");
        } else {
          getResult().setText(
              "Declare object(s) successfully created.");
        }
        outToServer.close();
        client.close();
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 REGISTERDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 REGISTERSTREAMDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class RegisterStreamDecAction extends AbstractAction implements
      Serializable {

    public RegisterStreamDecAction() {
      super(RegisterStreamDecAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      
      if (StreamMill.this.getEditor().getText() == null || StreamMill.this.getEditor().getText().trim().equals("")) {
        displayNoTextError("stream declaration(s)");
        return;
      }
      
      String dataToSend = getEditorDataForSending();
      String fileName = StreamMill.currentFilePath;


      if (fileName == null || fileName.trim().equals("")) {
        fileName = "dcl_" + getRandomNumber();
      } else if (fileName.indexOf(".") >= 0) {
        fileName = fileName.substring(0, fileName.indexOf("."));
      }

      fileName = getValidFileName(fileName,
          "a valid name for the stream declare module");
      if (fileName == null) {
        showErrorMessage("Invalid module name, please try again.");
        return;
      }
      fileName = fileName + ".dcl";

      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        OutputStream outToServer = client.getOutputStream();
        String command = ADD_STREAMS_COMMAND + " "
            + StreamMill.this.loginName + " " + fileName + "\n";

        outToServer.write(command.getBytes());
        outToServer.write(dataToSend.getBytes());

        DataInputStream is = new DataInputStream(client
            .getInputStream());
        String responseLine = null;
        StringBuffer wholeMessage = new StringBuffer();

        while ((responseLine = is.readLine()) != null) {
          wholeMessage.append(responseLine + "\n");
        }
        if (!wholeMessage.toString().trim().equals("")) {
          getResult().setText(
              "ERROR: Could not register streams. "
                  + "Error message from server is:\n"
                  + wholeMessage.toString() + "\n");
        } else {
          getResult().setText("Stream(s) successfully registered.");
        }
        outToServer.close();
        client.close();
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 REGISTERSTREAMDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWAGGR IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewAggrAction extends AbstractAction implements Serializable {

    public ViewAggrAction() {
      super(ViewAggrAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        List buf = new LinkedList();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEWALL_AGGREGATES_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.add(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        new AMH(buf, StreamMill.this, StreamMill.VIEW_ALL_AGGREGATES); // display
                                         // active
                                         // module
                                         // hierarchy
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWAGGR IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWEXTERN IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewExternAction extends AbstractAction implements
      Serializable {

    public ViewExternAction() {
      super(ViewExternAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        List buf = new LinkedList();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEWALL_EXTERNS_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.add(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        new AMH(buf, StreamMill.this, StreamMill.VIEW_ALL_EXTERNS); // display
                                      // active
                                      // module
                                      // hierarchy
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWAGGR IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWAll IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewAllAction extends AbstractAction implements Serializable {

    public ViewAllAction() {
      super(viewAllAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        StringBuffer buf = new StringBuffer();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client.getOutputStream().write(
            (VIEW_ALL_COMMAND + " " + StreamMill.this.loginName)
                .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.append(responseLine + "\n");
        }

        client.getOutputStream().close();
        is.close();
        client.close();
        new ModuleQueryDetails("All Modules", buf.toString(),
            StreamMill.this);

      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWALL IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWCOMPONENTS IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewComponentsAction extends AbstractAction implements
      Serializable {

    public ViewComponentsAction() {
      super(ViewComponentsAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        String response = "";

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEW_COMPONENTS_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          response = response.concat(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        if (response != null && !response.trim().equals(""))
          new ViewComponents(StreamMill.this, response); // display
                                   // active
                                   // module
                                   // hierarchy
        else
          JOptionPane.showMessageDialog(StreamMill.this,
              "There are no components in the system.");
        //new ViewComponents(StreamMill.this, "pqr1||pqr2||pqr3");
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  ////
  //          viewPerformance IMPLEMENTATION //
  ////
  //////////////////////////////////////////////////////////////
  private MonitoringEngineFrame _monitoringFrame = null;

  public class ViewMonitoringAction extends AbstractAction implements
      Serializable {

    public ViewMonitoringAction() {
      super(viewMonitoringAction);
    }

    public void actionPerformed(ActionEvent e) {

      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }

      try {
        if (_monitoringFrame == null) {
          _monitoringFrame = new MonitoringEngineFrame(
              StreamMill.this, StreamMill.this.loginName);

          _monitoringFrame.addWindowListener(new WindowListener() {
            //Window Event Listener
            public void windowClosing(WindowEvent e) {
              _monitoringFrame = null;
            }

            public void windowClosed(WindowEvent e) {
            }

            public void windowOpened(WindowEvent e) {
            }

            public void windowIconified(WindowEvent e) {
            }

            public void windowDeiconified(WindowEvent e) {
            }

            public void windowActivated(WindowEvent e) {
            }

            public void windowDeactivated(WindowEvent e) {
            }
          });
        }
      } catch (Exception exp) {
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWBUFS IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWTABLEDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewTableDecAction extends AbstractAction implements
      Serializable {

    public ViewTableDecAction() {
      super(ViewTableDecAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        List buf = new LinkedList();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEWALL_TABLES_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.add(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        new AMH(buf, StreamMill.this, StreamMill.VIEW_ALL_TABLES); // display
                                       // active
                                       // module
                                       // hierarchy
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWTABLEDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWSTREAMDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewStreamDecAction extends AbstractAction implements
      Serializable {

    public ViewStreamDecAction() {
      super(ViewStreamDecAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        List buf = new LinkedList();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEWALL_STREAMS_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.add(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        new AMH(buf, StreamMill.this, StreamMill.VIEW_ALL_STREAMS); // display
                                      // active
                                      // module
                                      // hierarchy
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWSTREAMDEC IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 ADDIOMODULE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class AddIOModuleAction extends AbstractAction implements
      Serializable {

    public AddIOModuleAction() {
      super(AddIOModuleAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      String dataToSend = StreamMill.this.getEditor().getText();
      String fileName = StreamMill.currentFilePath;

      if (dataToSend == null || dataToSend.trim().equals("")) {
        displayNoTextError("the source code for the datasource");
        return;
      }

      if (fileName == null || fileName.trim().equals("")) {
        fileName = "iomod_" + getRandomNumber();
      } else if (fileName.indexOf(".") >= 0) {
        fileName = fileName.substring(0, fileName.indexOf("."));
      }

      fileName = getValidFileName(fileName,
          "a valid name for the IOmodule");
      if (fileName == null) {
        showErrorMessage("Invalid module name, please try again.");
        return;
      }
      fileName = fileName + ".cc";
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        OutputStream outToServer = client.getOutputStream();
        String command = ADD_IOMODULE_COMMAND + " "
            + StreamMill.this.loginName + " " + fileName + "\n";
        outToServer.write(command.getBytes());

        outToServer.write(dataToSend.getBytes());

        DataInputStream is = new DataInputStream(client
            .getInputStream());
        String responseLine = null;
        StringBuffer wholeMessage = new StringBuffer();

        while ((responseLine = is.readLine()) != null) {
          wholeMessage.append(responseLine + "\n");
        }
        if (!wholeMessage.toString().trim().equals("")) {
          showErrorMessage("ERROR: Could not add IOModule. "
              + "Error message from server is:\n"
              + wholeMessage.toString() + "\n");
        } else {
          getResult().setText("IOModule successfully created.");
        }

        is.close();
        outToServer.close();
        client.close();
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                //
  //               ADDIOMODULE IMPLEMENTATION //
  //                //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWIOMODULE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewIOModulesAction extends AbstractAction implements
      Serializable {

    public ViewIOModulesAction() {
      super(ViewIOModulesAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        List buf = new LinkedList();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEW_IOMODULES_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.add(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        new ViewIOModules(buf, StreamMill.this); // display active
                             // module hierarchy
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWIOMODULE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 REGISTERQUE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class RegisterQueAction extends AbstractAction implements
      Serializable {

    public RegisterQueAction() {
      super(RegisterQueAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      
      if (StreamMill.this.getEditor().getText() == null || StreamMill.this.getEditor().getText().trim().equals("")) {
        displayNoTextError("query(ies)");
        return;
      }
      
      String dataToSend = getEditorDataForSending();
      String fileName = StreamMill.currentFilePath;


      if (fileName == null || fileName.trim().equals("")) {
        fileName = "cq_" + getRandomNumber();
      } else if (fileName.indexOf(".") >= 0) {
        fileName = fileName.substring(0, fileName.indexOf("."));
      }

      fileName = getValidFileName(fileName,
          "a valid name for the query(ies) module");
      if (fileName == null) {
        showErrorMessage("Invalid module name, please try again.");
        return;
      }
      fileName = fileName + ".cq";
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        OutputStream outToServer = client.getOutputStream();
        String command = ADD_QUERIES_COMMAND + " "
            + StreamMill.this.loginName + " " + fileName + "\n";

        outToServer.write(command.getBytes());
        outToServer.write(dataToSend.getBytes());

        DataInputStream is = new DataInputStream(client
            .getInputStream());
        String responseLine = null;
        StringBuffer wholeMessage = new StringBuffer();

        while ((responseLine = is.readLine()) != null) {
          wholeMessage.append(responseLine + "\n");
        }
        if (!wholeMessage.toString().trim().equals("")) {
          getResult().setText(
              "ERROR: Could not register queries. "
                  + "Error message from server is:\n"
                  + wholeMessage.toString() + "\n");
        } else {
          getResult().setText("Query(ies) successfully registered.");
        }

        is.close();
        outToServer.close();
        client.close();
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 REGISTERQUE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 REGISTERTS IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class RegisterTSAction extends AbstractAction implements
      Serializable {

    public RegisterTSAction() {
      super(RegisterTSAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      
      if (StreamMill.this.getEditor().getText() == null || StreamMill.this.getEditor().getText().trim().equals("")) {
        displayNoTextError("a time series query");
        return;
      }
      
      String dataToSend = getEditorDataForSending();
      String fileName = StreamMill.currentFile;

      if (fileName == null || fileName.trim().equals("")) {
        fileName = "ts_" + getRandomNumber();
      } else if (fileName.indexOf(".") >= 0) {
        fileName = fileName.substring(0, fileName.indexOf("."));
      }

      fileName = getValidFileName(fileName,
          "a valid name for the time series query");
      if (fileName == null) {
        showErrorMessage("Invalid time series query name, please try again.");
        return;
      }
      fileName = fileName + ".ts";
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        OutputStream outToServer = client.getOutputStream();
        String command = ADD_TS_QUERY_COMMAND + " "
            + StreamMill.this.loginName + " " + fileName + "\n";

        outToServer.write(command.getBytes());
        outToServer.write(dataToSend.getBytes());

        DataInputStream is = new DataInputStream(client
            .getInputStream());
        String responseLine = null;
        StringBuffer wholeMessage = new StringBuffer();

        while ((responseLine = is.readLine()) != null) {
          wholeMessage.append(responseLine + "\n");
        }
        if (!wholeMessage.toString().trim().equals("")) {
          getResult().setText(
              "ERROR: Could not register query. "
                  + "Error message from server is:\n"
                  + wholeMessage.toString() + "\n");
        } else {
          getResult().setText(
              "Time Series query successfully registered.");
        }

        is.close();
        outToServer.close();
        client.close();
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 REGISTERTS IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWQUE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewQueAction extends AbstractAction implements Serializable {

    public ViewQueAction() {
      super(ViewQueAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        List buf = new LinkedList();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEWALL_QUERIES_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.add(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        new AMH(buf, StreamMill.this, StreamMill.VIEW_ALL_QUERIES); // display
                                      // active
                                      // module
                                      // hierarchy
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWQUE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 VIEWTS IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class ViewTSAction extends AbstractAction implements Serializable {

    public ViewTSAction() {
      super(ViewTSAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() == NOT_LOGGED_IN) {
        showLibViewOrLoggedInMessage();
        return;
      }
      try {
        Socket client = new Socket(StreamMill.StreamServer,
            StreamMill.StreamServerPort);
        int i = 0;
        List buf = new LinkedList();

        DataInputStream is = new DataInputStream(client
            .getInputStream());

        client
            .getOutputStream()
            .write(
                (VIEWALL_TS_QUERIES_COMMAND + " " + StreamMill.this.loginName)
                    .getBytes());

        //client.setTrafficClass(Socket);
        String responseLine;
        while ((responseLine = is.readLine()) != null) {
          buf.add(responseLine);
        }

        client.getOutputStream().close();
        is.close();
        client.close();

        new AMH(buf, StreamMill.this, StreamMill.VIEW_ALL_TS_QUERIES); // display
                                         // active
                                         // module
                                         // hierarchy
      } catch (IOException exp) {
        AtlasGlobal.GetInstance().ShowConnectionError();
        exp.printStackTrace();
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 VIEWQUE IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 SNAPSHOT IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class SnapshotAction extends AbstractAction implements Serializable {

    public SnapshotAction() {
      super(SnapshotAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      String dataToSend = getEditorDataForSending();
      String fileName = StreamMill.currentFilePath;

      if (dataToSend == null || dataToSend.trim().equals("")) {
        displayNoTextError("a snapshot query");
        return;
      }

      if (fileName == null || fileName.trim().equals("")) {
        fileName = "adl_" + getRandomNumber();
      } else if (fileName.indexOf(".") >= 0) {
        fileName = fileName.substring(0, fileName.indexOf("."));
      }

      fileName = getValidFileName(fileName,
          "a valid name for snapshot query file");
      if (fileName == null) {
        showErrorMessage("Invalid query file name, please try again.");
        return;
      }
      fileName = fileName + ".cq";
      StringBuffer command = new StringBuffer(SNAPSHOT_QUERY_COMMAND
          + " " + StreamMill.this.loginName + " " + fileName + "\n");

      command.append(dataToSend);

      String reply = sendCommandToServerAndReceiveExactReply(command
          .toString());
      getResult().setText("Snapshot Query Result:\n" + reply);
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 SNAPSHOT IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //                 OneTupleTest IMPLENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  /**
   * Trys to write the document as a serialization.
   */
  public class OneTupleTestAction extends AbstractAction implements
      Serializable {

    public OneTupleTestAction() {
      super(oneTupleTestAction);
    }

    public void actionPerformed(ActionEvent e) {
      if (StreamMill.this.getUserStatus() != LOGGED_IN) {
        showHaveToBeLoggedInMessage();
        return;
      }
      String dataToSend = getEditorDataForSending();
      String fileName = StreamMill.currentFile;

      if (dataToSend == null || dataToSend.trim().equals("")) {
        displayNoTextError("one tuple test query(ies)");
        return;
      }

      if (fileName == null || fileName.trim().equals("")) {
        fileName = "cq_" + getRandomNumber();
      } else if (fileName.indexOf(".") >= 0) {
        fileName = fileName.substring(0, fileName.indexOf("."));
      }

      fileName = getValidFileName(fileName,
          "a valid name for the query(ies) module");
      if (fileName == null) {
        showErrorMessage("Invalid module name, please try again.");
        return;
      }
      fileName = fileName + ".cq";
      StringBuffer command = new StringBuffer(ONE_TUPLE_TEST_COMMAND
          + " " + StreamMill.this.loginName + " " + fileName + "\n");

      command.append(dataToSend);

      String reply = sendCommandToServerAndReceiveExactReply(command
          .toString());
      getResult().setText("One Tuple Test Result:\n" + reply);
    }
  }

  //////////////////////////////////////////////////////////////
  //                        //
  //                 ONETUPLETEST IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  /**
   * Trys to write the document as a serialization.
   */
  public class SaveResultAction extends AbstractAction implements
      Serializable {

    public SaveResultAction() {
      super(saveResultAction);
    }

    public void actionPerformed(ActionEvent e) {
      Frame frame = StreamMill.this;

      if (fileDialog == null) {
        fileDialog = new FileDialog(frame);
      }

      fileDialog.setMode(FileDialog.SAVE);
      fileDialog.show();
      String file = fileDialog.getFile();

      if (file == null) {
        return;
      }

      String directory = fileDialog.getDirectory();
      File f = new File(directory, file);

      try {
        Writer out = new FileWriter(f);
        getResult().write(out);
      } catch (IOException io) {
        System.err.println("IOException: " + io.getMessage());
      }
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                Print ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class PrintAction extends AbstractAction {

    public PrintAction() {
      super(printAction);
    }

    public void actionPerformed(ActionEvent e) {
      PrintUtilities.printComponent(getEditor());
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //            PrintResult ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class PrintResultAction extends AbstractAction {

    public PrintResultAction() {
      super(printResultAction);
    }

    public void actionPerformed(ActionEvent e) {
      PrintUtilities.printComponent(getResult());
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //               Compile ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class CompileAction extends AbstractAction {

    Frame frame = StreamMill.this;

    public CompileAction() {
      super(compileAction);
    }

    public void actionPerformed(ActionEvent e) {

      long start = startTime();

      statusbox.removeAll();

      execTime = new JTextField("Compiling File...", 40);
      execTime.setEnabled(false);

      rowCount = new JTextField("0 Row", 15);
      rowCount.setEnabled(false);

      JProgressBar progressBar = new JProgressBar();
      JLabel progressLabel = new JLabel();

      progressBar.setBorder(BorderFactory.createEtchedBorder());
      progressBar.setMinimum(0);
      progressBar.setMaximum(4);

      statusbox.add(progressBar);
      statusbox.add(Box.createHorizontalStrut(150));
      statusbox.add(execTime);
      statusbox.add(rowCount);
      statusbox.validate();

      getResult().setText("");
      cleanPane = true;

      // Generate .cc file using adlc
      progressBar.setString("Compiling Using adlc");

      // Defile "Files"
      // We need to convert window-like path to unix-like path
      // This is due to the fact we are running this program through
      // cygwin on windows
      /* String cygwin = new String("/cygwin"); */
      String file_path = new String(frame.getTitle());
      file_path = file_path.replace('\\', '/');

      int slashindex = file_path.indexOf('/');
      int lastindex = file_path.lastIndexOf(lastIndexSuffix);

      executable = file_path.substring(slashindex, lastindex);

      if (file_path.startsWith("F:") || file_path.startsWith("D:")
          || file_path.startsWith("E:"))
        execFile = file_path.substring(0, lastindex);
      else
        execFile = executable;

      compileFile = execFile + compileSuffix;
      objectFile = execFile + objectSuffix;

      /*
       * For testing purposes System.out.println ("\n\n testing...
       * execFile = " + execFile); System.out.println ("\n\n testing...
       * file_path = " + file_path); System.out.println ("\n\n testing...
       * executable = " + executable);
       */

      /*
       * In case, you changed the paths while the session was alive So,
       * update it
       */
      try {
        setting.setPaths(StreamMill.class.getClassLoader());
      } catch (Exception e2) {
      }

      if (file_path.startsWith("F:") || file_path.startsWith("D:")
          || file_path.startsWith("E:"))
        Exec.execWait("c:/Atlas/axl/adlc " + " c:" + execFile + ".adl");
      else
        Exec.execWait("c:/Atlas/axl/adlc " + " c:" + execFile + ".adl");

      // Create compiled, executed and object file
      progressBar.setValue(progressBar.getValue() + 1);
      progressLabel.setText("Generating .cc File");

      // Compile .cc file
      progressBar.setValue(progressBar.getValue() + 1);
      progressLabel.setText("Compile With Include Files");
      cleanPane = false;

      if (file_path.startsWith("F:") || file_path.startsWith("D:")
          || file_path.startsWith("E:"))
        Exec.execWaitPrint("g++ -o c:" + objectFile
            + " -IC:/Atlas/include -g -c c:" + compileFile);
      else
        Exec.execWaitPrint("g++ -o c:" + objectFile
            + " -IC:/Atlas/include -g -c c:" + compileFile);

      progressBar.setValue(progressBar.getValue() + 1);
      progressLabel.setText("Compile With Library Files");
      cleanPane = false;

      if (file_path.startsWith("F:") || file_path.startsWith("D:")
          || file_path.startsWith("f:"))
        Exec
            .execWaitPrint("g++ -o c:"
                + execFile
                + " c:"
                + objectFile
                + " "
                + "c:/Atlas/libraries/libadl.a c:/Atlas/libraries/libdb.a c:/Atlas/libraries/libimdb.a");
      else
        Exec
            .execWaitPrint("g++ -o c:"
                + execFile
                + " c:"
                + objectFile
                + " "
                + "c:/Atlas/libraries/libadl.a c:/Atlas/libraries/libdb.a c:/Atlas/libraries/libimdb.a");

      execCode = 2;
      progressBar.setValue(progressBar.getValue() + 1);
      showTime(start);
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //                 Exec ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class ExecAction extends AbstractAction {

    Frame frame = StreamMill.this;

    public ExecAction() {
      super(execAction);
    }

    public void actionPerformed(ActionEvent e) {

      // Don't print results, but wait until done
      // before continuing on.

      /*
       * long start = startTime(); statusbox.removeAll(); execTime = new
       * JTextField("Compiling File...", 40); execTime.setEnabled(false);
       * rowCount = new JTextField("0 Row", 15);
       * rowCount.setEnabled(false); JProgressBar progressBar = new
       * JProgressBar(); JLabel progressLabel = new JLabel(); // hard
       * coding 1 = exec
       * progressBar.setBorder(BorderFactory.createEtchedBorder());
       * progressBar.setMinimum(0); progressBar.setMaximum(1);
       * statusbox.add(progressBar);
       * statusbox.add(Box.createHorizontalStrut(150));
       * statusbox.add(execTime); statusbox.add(rowCount);
       * 
       * getResult().setText(""); cleanPane = true;
       * 
       * progressBar.setValue(progressBar.getValue() + 1);
       * progressLabel.setText("Executing The File");
       */
      Exec.execPrint(executable);
      /*
       * rowCount.setText(Exec.getRowCount() + " Rows"); execCode = 3;
       * showTime(start); statusbox.validate();
       */
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //               Clear ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class ClearAction extends AbstractAction {

    public ClearAction() {
      super(clearAction);
    }

    public void actionPerformed(ActionEvent e) {
      getEditor().setText("");
    }
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //               About ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class AboutAction extends AbstractAction {

    Frame frame = StreamMill.this;

    public AboutAction() {
      super(aboutAction);
    }

    public void actionPerformed(ActionEvent e) {
      int i = JOptionPane.showConfirmDialog(frame,
          "StreamMill Interface Version 1.1",
          "About StreamMill Interface", JOptionPane.DEFAULT_OPTION,
          JOptionPane.INFORMATION_MESSAGE);

    } /*
       * class AboutDialog extends JDialog implements ActionListener {
       * public AboutDialog(Frame parent, String title, String message){
       * super(parent, title, true);
       *  // If there was a parent, set dialog position inside if(parent !=
       * null){ Dimension parentSize = parent.getSize(); Point p =
       * parent.getLocation(); setLocation(p.x+parentSize.width/4,
       * p.y+parentSize.height/4); } // Create Message Pane JPanel
       * messagePane = new JPanel(); messagePane.add(new JLabel(message));
       * getContentPane().add(messagePane); // Create Button Pane JPanel
       * buttonPane = new JPanel(); JButton button = new JButton("OK");
       * buttonPane.add(button); button.addActionListener(this);
       * getContentPane().add(buttonPane, BorderLayout.SOUTH);
       * setDefaultCloseOperation(DISPOSE_ON_CLOSE); pack();
       * setVisible(true); }
       *  // OK Button Action public void actionPerformed(ActionEvent e){
       * setVisible(false); dispose(); } }
       */
  }

  //////////////////////////////////////////////////////////////
  //                  //
  //               Doc ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class DocAction extends AbstractAction {

    Frame frame = StreamMill.this;

    public DocAction() {
      super(docAction);
    }

    public void actionPerformed(ActionEvent e) {
      //String name =
      // StreamMill.class.getClassLoader().getResource().getPath();
      BrowserControl
          .displayURL("http://wis.cs.ucla.edu/stream-mill/doc/stream-mill-manual/client.pdf");
    }
  }

  //////////////////////////////////////////////////////////////
  //                      //
  //               Doc ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //               Examples ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  public class ExamplesAction extends AbstractAction {

    Frame frame = StreamMill.this;

    public ExamplesAction() {
      super(examplesAction);
    }

    public void actionPerformed(ActionEvent e) {
      //String name =
      // StreamMill.class.getClassLoader().getResource().getPath();
      BrowserControl
          .displayURL("http://wis.cs.ucla.edu/stream-mill/examples");
    }
  }

  //////////////////////////////////////////////////////////////
  //                      //
  //               Doc ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  //                  //
  //               Pref ACTION IMPLEMENTATION //
  //                  //
  //////////////////////////////////////////////////////////////
  class PrefAction extends AbstractAction {

    Frame frame = StreamMill.this;

    public JOptionPane prefMessage;

    PrefAction() {
      super(prefAction);
    }

    public void actionPerformed(ActionEvent e) {

      prefSetting = new JFrame("StreamMill Preference");

      prefSetting.addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {
          prefSetting.setVisible(false);
          prefSetting.dispose();
        }

        //Whenever window gets the focus, let the
          //TextFieldDemo set the initial focus.
          //public void windowActivated(WindowEvent e) {
          //    prefSetting.setFocus();
          //}
        });

      prefSetting.getContentPane().add(new PreferenceDialog(),
          BorderLayout.CENTER);
      prefSetting.setSize(600, 250);

      // Place it at Center of Screen
      Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
      Dimension size = prefSetting.getSize();
      screenSize.height = screenSize.height / 2;
      screenSize.width = screenSize.width / 2;
      size.height = size.height / 2;
      size.width = size.width / 2;
      int y = screenSize.height - size.height;
      int x = screenSize.width - size.width;
      prefSetting.setLocation(x, y);
      prefSetting.setVisible(true);
    }
  }

  public static JFrame getPrefSetting() {
    return prefSetting;
  }
}//End: Class StreamMill

/*
 * prefMessage = new JOptionPane("Berkerly DB Location:" + "\n" + "Ex:
 * /usr/local/db/", JOptionPane.QUESTION_MESSAGE, JOptionPane.OK_CANCEL_OPTION,
 * null); prefMessage.setSelectionValues(null); prefMessage.setWantsInput(true);
 * JDialog prefDialog = prefMessage.createDialog(frame, "Preference");
 * prefDialog.show(); String dbLocation = (String) prefMessage.getInputValue();
 * 
 * if (dbLocation == "uninitializedValue") { System.out.println("Using the
 * Default"); }
 */
// TO DO
// Exec the Save input String
