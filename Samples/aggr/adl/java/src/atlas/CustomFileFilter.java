package atlas;
import java.io.File;

import javax.swing.filechooser.FileFilter;

/*
 */

public class CustomFileFilter extends FileFilter {
  private String extension;
  private String description;

  public CustomFileFilter(String extension, String description)
  {
    this.extension = extension;
    this.description = description;
  }

	public boolean accept(File f) {
    if (f.isDirectory()) {
      return true;
    }
    if(f.getName().endsWith(extension))
    {
      return true;
    }
    return false;
	}

	public String getDescription() {
		return description;
	}

}
