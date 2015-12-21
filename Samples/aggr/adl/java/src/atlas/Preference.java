// Preference.java
//
// The class that represents the saved settings of AXL
//
// Written by James W. Lee
// July 11, 2000
//
// Last Edited: Sep 22, 2000
//   Added im_db properties

package atlas;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.Serializable;
import java.io.StreamTokenizer;

public class Preference implements Serializable
{
    // Paths
    private String adlc;
    private String berkeleyInc;
    private String berkeleyLib;
    private String adlInc;
    private String adlLib;
    private String imdbInc;
    private String imdbLib;

    // Size
    private String streamMill_width;
    private String streamMill_height;
    private String editor_width;
    private String editor_height;

    // Result (output) & Editor
    private int output_type;
    private int editor_type;

    public Preference()
    {
	this ("", "", "", "", "", "", "", "", "", "", "", 0, 0);
    }

    public Preference(String a, String bi, String bl, String ai, String al, String ii, String il, String aw, String ah, String ew, String eh, int r, int e)
    {
	setAdlc(a);
	setBerkeleyInc(bi);
	setBerkeleyLib(bl);
	setAdlInc(ai);
	setAdlLib(al);
	setImdbInc(ii);
	setImdbLib(il);
	setStreamMillWidth(aw);
	setStreamMillHeight(ah);
	setEditorWidth(ew);
	setEditorHeight(eh);
	setOutput_type(r);
	setEditor_type(e);
    }

    public void setAdlc(String a) {this.adlc = a;}
    public void setBerkeleyInc(String bi) {this.berkeleyInc = bi;}
    public void setBerkeleyLib(String bl) {this.berkeleyLib = bl;}
    public void setAdlInc(String ai) {this.adlInc = ai;}
    public void setAdlLib(String al) {this.adlLib = al;}
    public void setImdbInc(String ii) {this.imdbInc = ii;}
    public void setImdbLib(String il) {this.imdbLib = il;}
    public void setStreamMillWidth(String aw) {this.streamMill_width = aw;}
    public void setStreamMillHeight(String ah) {this.streamMill_height = ah;}
    public void setEditorWidth(String ew) {this.editor_width = ew;}
    public void setEditorHeight(String eh) {this.editor_height = eh;}
    public void setOutput_type(int r) {this.output_type = r;}
    public void setEditor_type(int e) {this.editor_type = e;}

    public String getAdlc() {return adlc;}
    public String getBerkeleyInc() {return berkeleyInc;}
    public String getBerkeleyLib() {return berkeleyLib;}
    public String getadlInc() {return adlInc;}
    public String getadlLib() {return adlLib;}
    public String getimdbInc() {return imdbInc;}
    public String getimdbLib() {return imdbLib;}
    public String getStreamMillWidth() {return streamMill_width;}
    public String getStreamMillHeight() {return streamMill_height;}
    public String getEditorWidth() {return editor_width;}
    public String getEditorHeight() {return editor_height;}
    public int getOutput_type() {return output_type;}
    public int getEditor_type() {return editor_type;}

    public void setPaths(ClassLoader cl) throws FileNotFoundException, IOException
    {
	//open the file that contains all the paths
        //FileReader inFile = new FileReader();
        //System.out.println("ABC: " + inFile.ready());
    	
    	
        StreamTokenizer tokens = new StreamTokenizer (cl.getResource("atlas/resources/Paths.dat").openStream());
        String Prefs[] = {"", "", "", "", "", "", ""};

        //treat endofline as a token
        tokens.eolIsSignificant(false);

        try {
    	    for (int i=0; i<7 && (tokens.nextToken() != StreamTokenizer.TT_EOF); i++) {
            Prefs[i] = tokens.sval;
          }
        }
        catch (IOException e) 
        {
          e.printStackTrace();
        }

        //save the paths
        adlc = Prefs[0];
	berkeleyInc = Prefs[1];
	berkeleyLib = Prefs[2];
	adlInc = Prefs[3];
	adlLib = Prefs[4];
	imdbInc = Prefs[5];
	imdbLib = Prefs[6];
    }

    public void setSize(ClassLoader cl) throws FileNotFoundException, IOException
    {
	//open the file that contains the size of the interface
	//FileReader inFile2 = new FileReader("Size.dat");
	StreamTokenizer tokens2 = new StreamTokenizer (cl.getResource("atlas/resources/Size.dat").openStream());
	String Prefs2[] = {"", "", "", ""}; //StreamMill-frame width, height, editor-frame width, height
		
	try {
		for (int i=0; i<4 && (tokens2.nextToken() != StreamTokenizer.TT_EOF); i++)
			Prefs2[i] = tokens2.sval;
	}
  catch (IOException e2) {
    e2.printStackTrace();
  }

	//save the size
	streamMill_width = Prefs2[0];
	streamMill_height = Prefs2[1];
	editor_width = Prefs2[2];
	editor_height = Prefs2[3];

    }	

    public void setOutput_Editor(ClassLoader cl) throws FileNotFoundException, IOException
    {
	//open the file that contains the result (output) & editor setup  
	//FileReader inFile3 = new FileReader("Output_Editor.dat");
	StreamTokenizer tokens3 = new StreamTokenizer (cl.getResource("atlas/resources/Output_Editor.dat").openStream());
	int Prefs3[] = {0, 0}; //result (output) & editor

	try {
		for (int i=0; i<2 && (tokens3.nextToken() != StreamTokenizer.TT_EOF); i++)
		{
			Integer temp = new Integer (tokens3.sval);
			Prefs3[i] = temp.intValue();
		}
			
	}
  catch (IOException e3) {
    e3.printStackTrace();
  }

	//save the setting
	output_type = Prefs3[0];
	editor_type = Prefs3[1];
    }

    // For testing purpose

    /*public static void main (String args[]) throws IOException
      {
	Preference p = new Preference();
	p.setPaths();
	p.setSize();
		
	System.out.println ("the result: " + p.adlc + " " + p.streamMill_width + " " + p.streamMill_height);
	}*/
}
