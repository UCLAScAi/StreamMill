package atlas;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

public class PreferenceDialog extends JPanel {		

	public static Preference setting = new Preference();

        static {
                try {
                        setting.setPaths(StreamMill.class.getClassLoader());
                        setting.setSize(StreamMill.class.getClassLoader());
			                  setting.setOutput_Editor(StreamMill.class.getClassLoader());
                } catch (Exception e) {}
        }

	public static String adlc = setting.getAdlc();
    	public static String berkeleyInc = setting.getBerkeleyInc();
        public static String berkeleyLib = setting.getBerkeleyLib();
        public static String adlInc = setting.getadlInc();
        public static String adlLib = setting.getadlLib();
        public static String imdbInc = setting.getimdbInc();
        public static String imdbLib = setting.getimdbLib();

	//Value for the Frame Size
	public static String frameWidth = setting.getStreamMillWidth();
        public static String frameHeight = setting.getStreamMillHeight();
        public static String editorWidth = setting.getEditorWidth();
        public static String editorHeight = setting.getEditorHeight();
    
        //Labels to identify the text fields
        private JLabel adlcLabel;
        private JLabel berkeleyIncLabel;
        private JLabel berkeleyLibLabel;
        private JLabel adlIncLabel;
        private JLabel adlLibLabel;
        private JLabel imdbLibLabel;
        private JLabel imdbIncLabel;

	//Labels to identify the text field
	private JLabel frameWidthLabel;
        private JLabel frameHeightLabel;
	private JLabel editorWidthLabel;
        private JLabel editorHeightLabel;	

        //Strings for the labels
	private static String adlcString = "adlc Executable Location: ";
        private static String berkeleyIncString = "Berkeley DB Include File: ";
        private static String berkeleyLibString = "Berkeley DB Library: ";
	private static String adlIncString = "StreamMill Include File: ";
	private static String adlLibString = "StreamMill Library: ";
	private static String imdbIncString = "In-Memory DB Include File: ";
	private static String imdbLibString = "In-Memory DB Library: ";


	// String for the labels
	private static String frameWidthString = "StreamMill Frame Width: ";
        private static String frameHeightString = "StreamMill Frame Height: ";
	private static String editorWidthString = "Editor Frame Width: ";
	private static String editorHeightString = "Editor Frame Height";
	
        //Text fields for data entry
	private JTextField adlcField;
        private JTextField berkeleyIncField;
        private JTextField berkeleyLibField;
        private JTextField adlIncField;
        private JTextField adlLibField;
        private JTextField imdbIncField;
        private JTextField imdbLibField;

	//Text fields for data entry
	private JTextField frameWidthField;
        private JTextField frameHeightField;
	private JTextField editorWidthField;
        private JTextField editorHeightField;	
	private JButton saveEnv;
	private JButton cancelButton;
	
	private JTabbedPane tabbedPane; 
        private boolean focusIsSet = false;	

	private static int editorPref = setting.getEditor_type();
	private static int outputPref = setting.getOutput_type();

	// Call setBlah() method to get the latest!!!
	public static String getAdlc() {
		//setting.setPaths();
		return setting.getAdlc();
	}
	public static String getBerkeleyInc() {
		//setting.setPaths();
		return setting.getBerkeleyInc();
	}
	public static String getBerkeleyLib() {
		//setting.setPaths();
		return setting.getBerkeleyLib();
	}
	public static String getAdlInc() {
		//setting.setPaths();
		return setting.getadlInc();
	}
	public static String getAdlLib() {
		//setting.setPaths();
		return setting.getadlLib();
	}
        public static String getImdbInc() {
	    return setting.getimdbInc();
	}
	public static String getImdbLib() {
	    return setting.getimdbLib();
	}
	public static int getEditorPref() throws Exception {
		setting.setOutput_Editor(StreamMill.class.getClassLoader());
		return setting.getEditor_type();
	}
	public static int getOutputPref() throws Exception {
		setting.setOutput_Editor(StreamMill.class.getClassLoader());
		return setting.getOutput_type();
	}	

	//////////////////////////////////////////////////
	public static void savePaths () throws IOException
	{
		DataOutputStream output = new DataOutputStream (new FileOutputStream ("Paths.dat"));

		output.writeBytes("\"" + adlc + "\"\n");
		output.writeBytes("\"" + berkeleyInc + "\"\n");
		output.writeBytes("\"" + berkeleyLib + "\"\n");
		output.writeBytes("\"" + adlInc + "\"\n");
		output.writeBytes("\"" + adlLib + "\"\n");
		output.writeBytes("\"" + imdbInc + "\"\n");
		output.writeBytes("\"" + imdbLib + "\"\n");
		output.close();
	}

	public static void saveSize () throws IOException
	{
		DataOutputStream output = new DataOutputStream (new FileOutputStream ("Size.dat"));

		output.writeBytes("\"" + frameWidth + "\" ");
		output.writeBytes("\"" + frameHeight + "\" ");
		output.writeBytes("\"" + editorWidth + "\" ");
		output.writeBytes("\"" + editorHeight + "\"");
		output.close();
	}

	public static void saveOutput_Editor () throws IOException
	{
		DataOutputStream output = new DataOutputStream (new FileOutputStream ("Output_Editor.dat"));
                /* for testing purposes
		setting.setEditor_type(editorPref);
		setting.setOutput_type(outputPref);
		System.out.println ("editorPref: " + editorPref + "outputPref: " + outputPref);
		*/

		output.writeBytes("\"" + outputPref + "\" ");
		output.writeBytes("\"" + editorPref + "\"");
		output.close();
	}
	///////////////////////////////////////////////////
	
		
	protected JFrame getFrame() {
		for (Container p = getParent(); p != null; p = p.getParent()) {
			if (p instanceof JFrame) {
				return (JFrame) p;
			}
		}
		return null;
        }	

        public PreferenceDialog() {
		JFrame parent = StreamMill.getStreamMillFrame();

		if(parent != null){
			Dimension parentSize = parent.getSize();
			Point p = parent.getLocation();
			setLocation(p.x+parentSize.width/4, p.y+parentSize.height/4);
		}

		tabbedPane = new JTabbedPane();
		Component panel1 = createEnvPane();
		tabbedPane.addTab("DB Environment", panel1);
		tabbedPane.setSelectedIndex(0);
		Component panel2 = createLibPane();
		tabbedPane.addTab("Library Files", panel2);
		Component panel3 = createHeaderPane();
		tabbedPane.addTab("Header Files", panel3);
		Component panel4 = createEditorPane();
		tabbedPane.addTab("Editor Setup", panel4);
	
		Component panel5 = createOutputPane();
		tabbedPane.addTab("Output Setup", panel5);
	
		Component panel6 = createScreenPane();
		tabbedPane.addTab("Screen Size Setup", panel6);			
		setLayout(new GridLayout(1, 1));
		add(tabbedPane);	
	}

	protected Component createEnvPane() {

        //Create the labels.
        adlcLabel = new JLabel(adlcString);
	berkeleyIncLabel = new JLabel(berkeleyIncString);
        berkeleyLibLabel = new JLabel(berkeleyLibString);
        adlIncLabel = new JLabel(adlIncString);
        adlLibLabel = new JLabel(adlLibString);
	imdbLibLabel = new JLabel(imdbLibString);
	imdbIncLabel = new JLabel(imdbIncString);


        //Create the text fields and set them up.
	adlcField = new JTextField(adlc, 30);
        berkeleyIncField = new JTextField(berkeleyInc, 30);
        berkeleyLibField = new JTextField(berkeleyLib, 30);
        adlIncField = new JTextField(adlInc, 30);
        adlLibField = new JTextField(adlLib, 30);
	imdbIncField = new JTextField(imdbInc, 30);
	imdbLibField = new JTextField(imdbLib, 30);

        //Tell accessibility tools about label/textfield pairs.
	adlcLabel.setLabelFor(adlcField);
        berkeleyIncLabel.setLabelFor(berkeleyIncField);
        berkeleyLibLabel.setLabelFor(berkeleyLibField);
        adlIncLabel.setLabelFor(adlIncField);
        adlLibLabel.setLabelFor(adlLibField);
	imdbIncLabel.setLabelFor(imdbIncField);
	imdbLibLabel.setLabelFor(imdbLibField);

        //Layout the labels in a panel.
        JPanel labelPane = new JPanel();
        labelPane.setLayout(new GridLayout(0, 1));
	labelPane.add(adlcLabel);
        labelPane.add(berkeleyIncLabel);
        labelPane.add(berkeleyLibLabel);
        labelPane.add(adlIncLabel);
        labelPane.add(adlLibLabel);
	labelPane.add(imdbIncLabel);
	labelPane.add(imdbLibLabel);

        //Layout the text fields in a panel.
        JPanel fieldPane = new JPanel();
        fieldPane.setLayout(new GridLayout(0, 1));
        fieldPane.add(adlcField);
	fieldPane.add(berkeleyIncField);
        fieldPane.add(berkeleyLibField);
        fieldPane.add(adlIncField);
        fieldPane.add(adlLibField);
	fieldPane.add(imdbIncField);
	fieldPane.add(imdbLibField);

	//Save User Prefered Environment
	saveEnv = new JButton("Save Berkeley DB Environment");

        saveEnv.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {

				adlc = adlcField.getText();
				berkeleyInc = berkeleyIncField.getText();
				berkeleyLib = berkeleyLibField.getText();
				adlInc = adlIncField.getText();
				adlLib = adlLibField.getText();
				imdbInc = imdbIncField.getText();
				imdbLib = imdbLibField.getText();
				
				try {
					savePaths();
				} catch (IOException E) {}

				/*System.out.println(berkeleyInc);
				System.out.println(berkeleyLib);
				System.out.println(adlInc);
				System.out.println(adlLib);
				*/
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
				
           		   	return;
            	}
        });

	//Cancel
	cancelButton = new JButton("Cancel");
        cancelButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
                return;
            }
        });

	//Button Panel
        JPanel buttonPane = new JPanel();
        buttonPane.setLayout(new GridLayout(1, 0));
        buttonPane.add(saveEnv);
        buttonPane.add(cancelButton);

	//Put the panels in another panel, labels on left,
        //text fields on right.
        JPanel contentPane = new JPanel();
        contentPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        contentPane.setLayout(new BorderLayout());
        contentPane.add(labelPane, BorderLayout.CENTER);
        contentPane.add(fieldPane, BorderLayout.EAST);
	contentPane.add(buttonPane, BorderLayout.SOUTH);
	return contentPane;
    	}

	//For Editor Lib Pane
	private JButton saveButton;

	//For Create Lib Pane
	private JPanel labelPane;
	private JList list;
    	private DefaultListModel listModel;
    	private JButton addButton;
	private JButton removeButton;
    	private JTextField libraryName;

	protected Component createLibPane() {
        
		listModel = new DefaultListModel();
		listModel.addElement(berkeleyLib);
		listModel.addElement(adlLib);
		listModel.addElement(imdbLib);

		//Create the list and put it in a scroll pane
        	list = new JList(listModel);
        	list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        	list.setSelectedIndex(0);
		list.addListSelectionListener(new ListSelectionListener() {
			public void valueChanged(ListSelectionEvent e) {
				if (e.getValueIsAdjusting() == false) {

				    if (list.getSelectedIndex() == -1) {
				        //No selection, disable remove button.
				        addButton.setEnabled(false);
				        libraryName.setText("");

				    } else {
				        //Selection, update text field.
				        removeButton.setEnabled(true);
				        String name = list.getSelectedValue().toString();
				        libraryName.setText(name);
				    }
				}
			}
		});

        	JScrollPane listScrollPane = new JScrollPane(list);
		listScrollPane.setBorder(BorderFactory.createTitledBorder("Directory of Library"));
		saveButton = new JButton("Save Library Setup");
        	saveButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				// TODO: Execute With Library Settings
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
			}
		});		

        	addButton = new JButton("New Library!");
        	addButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				//User didn't type in a name...
				if (libraryName.getText().equals("")) {
				    Toolkit.getDefaultToolkit().beep();
				    return;
				}

				int index = list.getSelectedIndex();
				int size = listModel.getSize();

				//If no selection or if item in last position is selected,
				//add the new add to end of list, and select new add.
				if (index == -1 || (index+1 == size)) {
				    listModel.addElement(libraryName.getText());
				    list.setSelectedIndex(size);

				//Otherwise insert the new add after the current selection,
				//and select new add.
				} else {
				    listModel.insertElementAt(libraryName.getText(), index+1);
				    list.setSelectedIndex(index+1);
				}
                		return;
			}
		});

        	removeButton = new JButton("Remove Library!");
        	removeButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
			    
				//This method can be called only if
				//there's a valid selection
				//so go ahead and remove whatever's selected.
				int index = list.getSelectedIndex();
				listModel.remove(index);

			        int size = listModel.getSize();

			        if (size == 0) {
			        	//Nobody's left, disable firing.
			        	removeButton.setEnabled(false);

			    	} else {
			    		//Adjust the selection.
			        	if (index == listModel.getSize())//removed item in last position
			            		index--;
			        	list.setSelectedIndex(index);   //otherwise select same index
			    	}

                		return;
			}  
		});

		//Cancel
		cancelButton = new JButton("Cancel");
        	cancelButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();
           			return;
            		}
        	});

		libraryName = new JTextField(30);
        	libraryName.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				//User didn't type in a name...
				if (libraryName.getText().equals("")) {
				    Toolkit.getDefaultToolkit().beep();
				    return;
				}

				int index = list.getSelectedIndex();
				int size = listModel.getSize();

				//If no selection or if item in last position is selected,
				//add the new add to end of list, and select new add.
				if (index == -1 || (index+1 == size)) {
				    listModel.addElement(libraryName.getText());
				    list.setSelectedIndex(size);

				//Otherwise insert the new add after the current selection,
				//and select new add.
				} else {
				    listModel.insertElementAt(libraryName.getText(), index+1);
				    list.setSelectedIndex(index+1);
				}
                		return;
			}
		});

        	String name = listModel.getElementAt(
				list.getSelectedIndex()).toString();

		libraryName.setText(name);		

		//Layout the button fields in a panel.
       		JPanel buttonPane = new JPanel();
		buttonPane.setLayout(new GridLayout(1, 0));
		buttonPane.add(saveButton);
       		buttonPane.add(addButton);
       		buttonPane.add(removeButton);
       		buttonPane.add(cancelButton);	

		//Layout the button fields in a panel.
	        JPanel namePane = new JPanel();
		namePane.setBorder(BorderFactory.createTitledBorder("Name of Library"));
	        namePane.setLayout(new GridLayout(1, 0));
		namePane.add(libraryName);

		JPanel lowerPane = new JPanel();
	        lowerPane.setLayout(new BorderLayout());
		lowerPane.add(namePane, BorderLayout.CENTER);
		lowerPane.add(buttonPane, BorderLayout.SOUTH);

		//Put the panels in another panel, labels on left,
	        //text fields on right.
	        JPanel contentPane = new JPanel();
	        contentPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
	        contentPane.setLayout(new BorderLayout());
	        contentPane.add(listScrollPane, BorderLayout.CENTER);
	    	contentPane.add(lowerPane, BorderLayout.SOUTH);

		return contentPane;
        }

 	//For Create Header Pane
	private JList header;
    	private DefaultListModel headerModel;
    	private JTextField headerName;

	protected Component createHeaderPane() {
        
		headerModel = new DefaultListModel();
		headerModel.addElement(berkeleyInc);
		headerModel.addElement(adlInc);
		headerModel.addElement(imdbInc);

		//Create the list and put it in a scroll pane
        	header = new JList(headerModel);
        	header.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        	header.setSelectedIndex(0);

		header.addListSelectionListener(new ListSelectionListener() {
			public void valueChanged(ListSelectionEvent e) {
				if (e.getValueIsAdjusting() == false) {

					if (header.getSelectedIndex() == -1) {
						//No selection, disable remove button.
				        	addButton.setEnabled(false);
				        	headerName.setText("");

					} else {
						//Selection, update text field.
				        	removeButton.setEnabled(true);
				        	String name = header.getSelectedValue().toString();
				        	headerName.setText(name);
					}
				}
			}
		});
			  
        	JScrollPane headerScrollPane = new JScrollPane(header);
		headerScrollPane.setBorder(BorderFactory.createTitledBorder("Directory of Header Files"));
		
		saveButton = new JButton("Save Library Setup");
        	saveButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				// TODO: Execute With Library Settings
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
			}
		});		

        	addButton = new JButton("New Library!");

        	addButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				//User didn't type in a name...
				if (headerName.getText().equals("")) {
				    Toolkit.getDefaultToolkit().beep();
				    return;
				}

				int index = header.getSelectedIndex();
				int size = headerModel.getSize();

				//If no selection or if item in last position is selected,
				//add the new add to end of list, and select new add.
				if (index == -1 || (index+1 == size)) {
				    headerModel.addElement(headerName.getText());
				    header.setSelectedIndex(size);

				//Otherwise insert the new add after the current selection,
				//and select new add.
				} else {
				    headerModel.insertElementAt(headerName.getText(), index+1);
				    header.setSelectedIndex(index+1);
				}
                		return;
			}
		});

        	removeButton = new JButton("Remove Library!");
        	removeButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
			    
				//This method can be called only if
				//there's a valid selection
			    	//so go ahead and remove whatever's selected.
			    	int index = header.getSelectedIndex();
			    	headerModel.remove(index);

			    	int size = headerModel.getSize();

			    	if (size == 0) {
			    		//Nobody's left, disable firing.
			       		removeButton.setEnabled(false);

			    	} else {
			    		//Adjust the selection.
			        	if (index == headerModel.getSize())//removed item in last position
			            		index--;

			        	header.setSelectedIndex(index);   //otherwise select same index
			    	}

                		return;
			}
		});

		//Cancel
		cancelButton = new JButton("Cancel");
        	cancelButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();
                		return;
            		}
        	});

		headerName = new JTextField(30);
        	headerName.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				//User didn't type in a name...
				if (headerName.getText().equals("")) {
				    Toolkit.getDefaultToolkit().beep();
				    return;
				}

				int index = header.getSelectedIndex();
				int size = headerModel.getSize();

				//If no selection or if item in last position is selected,
				//add the new add to end of list, and select new add.
				if (index == -1 || (index+1 == size)) {
				    headerModel.addElement(headerName.getText());
				    header.setSelectedIndex(size);

				//Otherwise insert the new add after the current selection,
				//and select new add.
				} else {
				    headerModel.insertElementAt(headerName.getText(), index+1);
				    header.setSelectedIndex(index+1);
				}
                		return;
			}
		});

        	String name = headerModel.getElementAt(
                	header.getSelectedIndex()).toString();

        	headerName.setText(name);		

        	//Layout the button fields in a panel.
       	 	JPanel buttonPane = new JPanel();
        	buttonPane.setLayout(new GridLayout(1, 0));
		buttonPane.add(saveButton);
        	buttonPane.add(addButton);
        	buttonPane.add(removeButton);
        	buttonPane.add(cancelButton);

		//Layout the button fields in a panel.
        	JPanel namePane = new JPanel();
		namePane.setBorder(BorderFactory.createTitledBorder("Name of Header Files"));
        	namePane.setLayout(new GridLayout(1, 0));
		namePane.add(headerName);
		
		JPanel lowerPane = new JPanel();
        	lowerPane.setLayout(new BorderLayout());
		lowerPane.add(namePane, BorderLayout.CENTER);
		lowerPane.add(buttonPane, BorderLayout.SOUTH);

		//Put the panels in another panel, labels on left,
        	//text fields on right.
        	JPanel contentPane = new JPanel();
        	contentPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        	contentPane.setLayout(new BorderLayout());
        	contentPane.add(headerScrollPane, BorderLayout.CENTER);
    		contentPane.add(lowerPane, BorderLayout.SOUTH);
		return contentPane;
    	}
	
	protected Component createEditorPane() {
        	int numEditor = 3;
		final ButtonGroup editorGroup = new ButtonGroup();
		final JRadioButton editorChoice[] = new JRadioButton[numEditor];

		editorChoice[0] = new JRadioButton("Open File Using StreamMill Editor");
		editorChoice[0].setActionCommand(editorChoice[0].getText());

        	editorChoice[1] = new JRadioButton("Open File Using Vi Editor");
		editorChoice[1].setActionCommand(editorChoice[1].getText());
		editorChoice[2] = new JRadioButton("Open File Using Emac Editor");
		editorChoice[2].setActionCommand(editorChoice[2].getText());
        
		editorGroup.add(editorChoice[0]);
		editorGroup.add(editorChoice[1]);
		editorGroup.add(editorChoice[2]);

		
		if (editorPref == 0)
			editorChoice[0].setSelected(true);
		else if (editorPref == 1)
			editorChoice[1].setSelected(true);
		else if (editorPref == 2)
			editorChoice[2].setSelected(true);

		saveButton = new JButton("Save Editor Setup");
        	saveButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				if (editorChoice[0].isSelected()){
					editorPref = 0;
					/*System.out.println(editorPref);*/

					try {
						saveOutput_Editor();
					} catch (IOException E) {}
					
				}
				else if (editorChoice[1].isSelected()){
					editorPref = 1;
					/*System.out.println(editorPref);*/

					try {
						saveOutput_Editor();
					} catch (IOException E) {}
					
				}	
				else if (editorChoice[2].isSelected()){
					editorPref = 2;
					/*System.out.println(editorPref);*/

					try {
						saveOutput_Editor();
					} catch (IOException E) {}
					
				}				
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
			}
		});

		//Cancel
		cancelButton = new JButton("Cancel");
        	cancelButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
                		return;
            		}
        	});		

        	//Layout the labels in a panel.
	        JPanel labelPane = new JPanel();
		labelPane.setBorder(BorderFactory.createTitledBorder("Select An Editor"));
       		labelPane.setLayout(new GridLayout(0, 1));
        	for (int i = 0; i < numEditor; i++) {
            		labelPane.add(editorChoice[i]);
        	}	
        	//Layout the button fields in a panel.
        	JPanel buttonPane = new JPanel();
        	buttonPane.setLayout(new GridLayout(1, 0));
        	buttonPane.add(saveButton);
        	buttonPane.add(cancelButton);
		//Put the panels in another panel, labels on left,
        	//text fields on right.
        	JPanel contentPane = new JPanel();
        	contentPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        	contentPane.setLayout(new BorderLayout());
        	contentPane.add(labelPane, BorderLayout.CENTER);
    		contentPane.add(buttonPane, BorderLayout.SOUTH);

		return contentPane;
    	}

	protected Component createOutputPane() {
        	int numOutput = 2;
		final ButtonGroup outputGroup = new ButtonGroup();
		final JRadioButton outputChoice[] = new JRadioButton[numOutput];

		outputChoice[0] = new JRadioButton("Output Result in Editor Panel (Enable Edit Output)");
		outputChoice[0].setActionCommand(outputChoice[0].getText());

		outputChoice[1] = new JRadioButton("Output Result in Result Panel (Disable Edit Output)");
		outputChoice[1].setActionCommand(outputChoice[1].getText());

		outputGroup.add(outputChoice[0]);
		outputGroup.add(outputChoice[1]);

		if (outputPref == 0)
			outputChoice[0].setSelected(true);
		else 
			outputChoice[1].setSelected(true);

		saveButton = new JButton("Save Output Setup");
        	saveButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (outputChoice[0].isSelected()){
					outputPref = 0;
					/*System.out.println(outputPref);*/

					try {
						saveOutput_Editor();
					} catch (IOException E) {}
					
				}
				else if (outputChoice[1].isSelected()){
					outputPref = 1;
					/*System.out.println(outputPref);*/

					try {
						saveOutput_Editor();
					} catch (IOException E) {}
					
				}				
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
			}
		});

		//Cancel
		cancelButton = new JButton("Cancel");
	        cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
                		return;
            		}
        	});		

        	//Layout the labels in a panel.
        	JPanel labelPane = new JPanel();
		labelPane.setBorder(BorderFactory.createTitledBorder("Select Output Result Panel"));

        	labelPane.setLayout(new GridLayout(0, 1));
        	for (int i = 0; i < numOutput; i++) {
            		labelPane.add(outputChoice[i]);
        	}	

 	       	//Layout the button fields in a panel.
		JPanel buttonPane = new JPanel();
        	buttonPane.setLayout(new GridLayout(1, 0));
        	buttonPane.add(saveButton);
        	buttonPane.add(cancelButton);
		//Put the panels in another panel, labels on left,
        	//text fields on right.
        	JPanel contentPane = new JPanel();
        	contentPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        	contentPane.setLayout(new BorderLayout());
        	contentPane.add(labelPane, BorderLayout.CENTER);
    		contentPane.add(buttonPane, BorderLayout.SOUTH);

		return contentPane;
 	}

	protected Component createScreenPane() {
        
		//Create the labels.
        	frameWidthLabel = new JLabel(frameWidthString);
		frameHeightLabel = new JLabel(frameHeightString);
		editorWidthLabel = new JLabel(editorWidthString);
		editorHeightLabel = new JLabel(editorHeightString);
		
		//Create the text fields and set them up.
		frameWidthField = new JTextField(frameWidth, 5);
        	frameHeightField = new JTextField(frameHeight, 5);
        	editorWidthField = new JTextField(editorWidth, 5);
        	editorHeightField = new JTextField(editorHeight, 5);
        
        	//Tell accessibility tools about label/textfield pairs.
		frameWidthLabel.setLabelFor(frameWidthField);
        	frameHeightLabel.setLabelFor(frameHeightField);
        	editorWidthLabel.setLabelFor(editorWidthField);
        	editorHeightLabel.setLabelFor(editorHeightField);
   		JPanel frameSizePane = new JPanel();
		frameSizePane.setBorder(BorderFactory.createTitledBorder("Set StreamMill Frame Size"));
		frameSizePane.setLayout(new GridLayout(2, 0));
		frameSizePane.add(frameWidthLabel);
		frameSizePane.add(frameHeightLabel);
		frameSizePane.add(frameWidthField);
		frameSizePane.add(frameHeightField);
		
		JPanel editorSizePane = new JPanel();
		editorSizePane.setBorder(BorderFactory.createTitledBorder("Set Editor Frame Size"));
		editorSizePane.setLayout(new GridLayout(2, 0));
		editorSizePane.add(editorWidthLabel);
		editorSizePane.add(editorHeightLabel);
		editorSizePane.add(editorWidthField);
		editorSizePane.add(editorHeightField);		

		saveButton = new JButton("Save Frame Size Setup");
        	saveButton.addActionListener(new ActionListener() {
            		public void actionPerformed(ActionEvent e) {				

				frameWidth = frameWidthField.getText();
				frameHeight = frameHeightField.getText();
				editorWidth = editorWidthField.getText();
				editorHeight = editorHeightField.getText();

				try {
					saveSize();
				} catch (IOException E) {}

				StreamMill.getStreamMillFrame().setSize(Integer.parseInt(frameWidth),Integer.parseInt(frameHeight));
				StreamMill.getStreamMillFrame().validate();
				StreamMill.getEditorScroller().setPreferredSize(new Dimension(Integer.parseInt(editorWidth),Integer.parseInt(editorHeight)));
				StreamMill.getEditorScroller().validate();
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
            		}
		});

		//Cancel
		cancelButton = new JButton("Cancel");
	        cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				StreamMill.getPrefSetting().setVisible(false);
				StreamMill.getPrefSetting().dispose();;
                		return;
            		}
        	});		

        	//Layout the labels in a panel.
        	JPanel labelPane = new JPanel();
		labelPane.setLayout(new GridLayout(0, 1));
		labelPane.add(frameSizePane);
		labelPane.add(editorSizePane);

		//Layout the button fields in a panel.
        	JPanel buttonPane = new JPanel();
        	buttonPane.setLayout(new GridLayout(1, 0));
        	buttonPane.add(saveButton);
        	buttonPane.add(cancelButton);
		//Put the panels in another panel, labels on left,
        	//text fields on right.
        	JPanel contentPane = new JPanel();
        	contentPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        	contentPane.setLayout(new BorderLayout());
        	contentPane.add(labelPane, BorderLayout.CENTER);
    		contentPane.add(buttonPane, BorderLayout.SOUTH);

		return contentPane;
    	}
}
/*
class CustomDialog extends JDialog {
	public static String libName = "";
	public static String libLocation = "";
	//Labels to identify the text fields
	private JLabel libNameLabel;
	private JLabel libLocationLabel;
		
	//Strings for the labels
	private static String libNameString = "New Library Name: ";
	private static String libLocationString = "New Library Location: ";
	//Text fields for data entry
	private JTextField libNameField;
	private JTextField libLocationField;
	public static String getLibName() {
		return libName;
	}
	public static String getLibLocation() {
		return libLocation;
	}

	public CustomDialog(Frame parent, String title) {	
		super(parent, title);
						
		//Create the labels.
		libNameLabel = new JLabel(libNameString);
		libLocationLabel = new JLabel(libLocationString);

		//Create the text fields and set them up.
		libNameField = new JTextField(libName, 20);
		libLocationField = new JTextField(libLocation, 20);
	
		//Tell accessibility tools about label/textfield pairs.
		libNameLabel.setLabelFor(libNameField);
		libLocationLabel.setLabelFor(libLocationField);
		// If there was a parent, set dialog position inside
		if(parent != null){
			Dimension parentSize = parent.getSize();
			Point p = parent.getLocation();
			setLocation(p.x+parentSize.width/4, p.y+parentSize.height/4);
		}
		// Create Message Pane
		JPanel messagePane = new JPanel();
		messagePane.setLayout(new GridLayout(2, 0));
		messagePane.add(libNameLabel);
		messagePane.add(libNameField);
		messagePane.add(libLocationLabel);
		messagePane.add(libLocationField);
		getContentPane().add(messagePane);
		JButton addButton = new JButton("OK");
		addButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				libName = libNameField.getText();
				libLocation = libLocationField.getText();	
				setVisible(false);
				dispose();
				int num = PreferenceDialog.getNumLib();
				System.out.println(num);
				JRadioButton button[] = PreferenceDialog.getLibRadioButtons();
				button[num] = new JRadioButton(libName);
				String string[] = PreferenceDialog.getLibraryLocation();
				string[num] = new String(libLocation);
				PreferenceDialog.getLabelPane().removeAll();
				for (int i = 0; i < num + 1; i++) {
					if (button[i].getText() != "Deleted") {
						PreferenceDialog.getLabelPane().add(button[i]);
					}
				}
				PreferenceDialog.getLabelPane().validate();

				return;
			}
		});
				
		JButton cancelButton = new JButton("Cancel");
		cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				setVisible(false);
				dispose();
				return;
			}
		});
				
		// Create Button Pane
		JPanel buttonPane = new JPanel();
		buttonPane.add(addButton);
		buttonPane.add(cancelButton);
				
		getContentPane().add(buttonPane, BorderLayout.SOUTH);
		setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		pack();
		setVisible(true);
		}
}					
	
*/
