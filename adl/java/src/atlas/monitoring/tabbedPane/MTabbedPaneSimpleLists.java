/*
 * Created on Mar 11, 2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package atlas.monitoring.tabbedPane;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.DefaultListModel;
import javax.swing.JComponent;
import javax.swing.JDesktopPane;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;

import atlas.monitoring.category.MCategory;
import atlas.monitoring.category.MCategoryQuery;
import atlas.monitoring.library.MElement;
import atlas.monitoring.proxy.MProxy;

/**
 * @author treetree
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public abstract class MTabbedPaneSimpleLists extends MTabbedPane {
	
	
	protected final String MONITORING_COMPONENT_FRAME_PACKAGE_PREFIX = "atlas.monitoring.componentFrame";
	protected final String MONITORING_COMPONENT_FRAME_PREFIX = "MComponentFrame";
	protected final String MONITORING_COMPONENT_FRAME_WAVEFORM = "Waveform";
	protected final String MONITORING_COMPONENT_FRAME_TEXT = "Text";
	protected final String MONITORING_COMPONENT_FRAME_CLUSTER = "Cluster";
	protected final String MONITORING_COMPONENT_FRAME_HORIZONTAL_HISTOGRAM = "HistogramHorizontal";
	protected final String MONITORING_COMPONENT_FRAME_VERTICAL_HISTOGRAM = "HistogramVertical";

	
	protected JList _activatedList;
	protected JList _unactivatedList;
	protected DefaultListModel _activatedDefaultListModel;
	protected DefaultListModel _unactivatedDefaultListModel;

	/**
	 * Constructor
	 * @param mc Monitoring Category
	 * @param desktop Internal desktop object
	 * @param proxy Monitoring Proxy
	 */
	public MTabbedPaneSimpleLists(MCategory mc, JDesktopPane desktop, MProxy proxy) {
		super(mc,desktop,proxy);
		
		_comPanel = createPanel();
	}
	
	public void CustomizedCleanUp() {
		try {
			/*for( int i=0; i<_activatedDefaultListModel.getSize(); i++ ) {
        String parameter_id = (String)_activatedDefaultListModel.getElementAt(i);
				_proxy.RequestMonitoringAction(_monitoringCategory.GetStopMonitoringCommand(),parameter_id);
			}*/
      _proxy.RequestMonitoringAction(MCategoryQuery.UNMONITOR_ALL_OF_IP_COMMAND, "");
		}
		catch(Exception ex) {
			
		}
	}

	
    /**
	 * Download the parameter to the list
	 */
    public boolean LoadParameterToCategory(boolean enforced) {
    	try {
    		if(!enforced && _monitoringCategory.IsParameterLoaded()) 
	    		return true;
	    	
    		List list = this._proxy.GetMonitoringParameters(_monitoringCategory.GetDonwloadingParametersCommand());
	    		
    		if(list==null)
    			return false;
    		
    		_monitoringCategory.ParseMonitoringParametersList(list);
	    	
	    	populateParameterListUI();
	    	
	    	return true;
		}
		catch(Exception ex) {
			ex.printStackTrace();
			return false;
		}
	}  

	/**
	 * Create the main panel
	 * @return
	 */
	private JPanel createPanel() {
		_activatedList = new JList();
		_unactivatedList = new JList();
		
		_activatedDefaultListModel = new DefaultListModel();
		_unactivatedDefaultListModel = new DefaultListModel();
		
		_activatedList = new JList(_activatedDefaultListModel);
		_unactivatedList = new JList(_unactivatedDefaultListModel);
		
		return createPaneForParameterList(_activatedList,_unactivatedList);
	}
	
	
	
    /**
     * Create panel for the parameter list
     * @param activatedList
     * @param unactivatedList
     * @return
     */
    private JPanel createPaneForParameterList(JList activatedList, JList unactivatedList ) {
    	activatedList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    	unactivatedList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        
        JScrollPane activatedListView = new JScrollPane(activatedList);
        JScrollPane unactivatedListView = new JScrollPane(unactivatedList);
        
        activatedListView.setPreferredSize(new Dimension(80, 100));
        unactivatedListView.setPreferredSize(new Dimension(80, 100));
        
        JPanel unactivatedPanel = createPanelForComponent(unactivatedListView, "Unactivated");
        JPanel activatedPanel = createPanelForComponent(activatedListView, "Activated");
        
        JPanel panel = new JPanel(new BorderLayout());
        panel.setLayout(new BoxLayout(panel, BoxLayout.PAGE_AXIS));
        panel.add(unactivatedPanel);
        panel.add(activatedPanel); 
        
        return panel;
    }

    /**
     * create panel for each component
     * @param comp
     * @param title
     * @return
     */
    private JPanel createPanelForComponent(JComponent comp, String title) {
		JPanel panel = new JPanel(new BorderLayout());
		panel.add(comp, BorderLayout.CENTER);
		if (title != null) {
			panel.setBorder(BorderFactory.createTitledBorder(title));
		}
		return panel;
	}
	

    
    /**
     * Once we load the parameter, we need to populate the change
     * to the UI
     */
	private void populateParameterListUI() {
		_activatedDefaultListModel.removeAllElements();
		_unactivatedDefaultListModel.removeAllElements();
		
		for(int i=0; i<_monitoringCategory.GetNumOfParameters(); i++){
			if(_monitoringCategory.GetMonitoringElementAt(i).IsMonitoring()) {
				_activatedDefaultListModel.addElement(_monitoringCategory.GetMonitoringElementAt(i).GetName());
			}
			else {
				_unactivatedDefaultListModel.addElement(_monitoringCategory.GetMonitoringElementAt(i).GetName());
			}
		}
		_activatedList.updateUI();
		_unactivatedList.updateUI();
	}
	
    /**
     * Set the status of the monitoring parameter
     * @param parameterID
     * @param isMonitoring
     */
   protected final void setMonitorElementMonitoringState(String parameterID, boolean isMonitoring) {
	
		//Linear search is fine since there are no many parameter
		//or modify the MonitoringCategories and MonitoringElment class to improve performance
		MElement[] elements = _monitoringCategory.GetMonitoringElements();
		
		for( int i=0; i<elements.length; i++) {
			if(elements[i].GetName().equals(parameterID)) {
				elements[i].SetMonitoring(isMonitoring);
				populateParameterListUI();
				break;
			}
		}	
   }
   
	/**
	 * 
	 * @author treetree
	 *
	 * TODO To change the template for this generated type comment go to
	 * Window - Preferences - Java - Code Style - Code Templates
	 */
   protected abstract class SimpleListMouseListener implements MouseListener {
   	
	   	protected String _name;
	   	protected boolean _isEnable;
	   	
	   	SimpleListMouseListener(String name, boolean isEnable) {
	   		this._name = name;
	   		this._isEnable = isEnable;
	   	}
	   	
	   	public void mousePressed(MouseEvent e) {		check(e);	}
			public void mouseReleased(MouseEvent e) {		check(e);	}
			public void mouseEntered(MouseEvent e) {}
			public void mouseExited(MouseEvent e) {}
			public void mouseClicked(MouseEvent e) {}
			protected void check(MouseEvent e) {
				if(e.isPopupTrigger())  {
					((JList)e.getSource()).setSelectedIndex(((JList)e.getSource()).locationToIndex(new java.awt.Point(e.getX(), e.getY())));
					if(((JList)e.getSource()).getSelectedIndex() > -1 ) {
						customAction(e);
					}
				}
			}
			abstract protected void customAction(MouseEvent e);
	   }
}
