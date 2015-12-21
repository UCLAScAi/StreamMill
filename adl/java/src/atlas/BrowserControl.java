/*
 */

package atlas;
import java.io.IOException;
public class BrowserControl
{
  public static void displayURL(String url)
  {
    boolean windows = isWindowsPlatform();
    String cmd = null;
    try {
        if (windows)
            {
                if(url.startsWith("/"))
                  cmd = WIN_PATH + " " + WIN_FLAG + " " + url.substring(1);  
                else if(url.startsWith("file:/"))
                  cmd = WIN_PATH + " " + WIN_FLAG + " " + url.substring(6);
                else
                  cmd = WIN_PATH + " " + WIN_FLAG + " " + url;
                //cmd = cmd.replace('!', '/');
                //System.out.println(cmd);
                Process p = Runtime.getRuntime().exec(cmd);
            }
            else
            {
                cmd = UNIX_PATH + " " + UNIX_FLAG + "(" + url + ")";
                Process p = Runtime.getRuntime().exec(cmd);
                try
                {
                    int exitCode = p.waitFor();
                    if (exitCode != 0)
                    {
                        cmd = UNIX_PATH + " "  + url;
                        p = Runtime.getRuntime().exec(cmd);
                    }
                }
                catch(InterruptedException x)
                {
                    System.err.println("Error bringing up browser, cmd='" +
                                       cmd + "'");
                    System.err.println("Caught: " + x);
                }
            }
        }
        catch(IOException x)
        {
            // couldn't exec browser
            System.err.println("Could not invoke browser, command=" + cmd);
            System.err.println("Caught: " + x);
        }
    }
    /**
     * Try to determine whether this application is running under Windows
     * or some other platform by examing the "os.name" property.
     *
     * @return true if this application is running under a Windows OS
     */
    public static boolean isWindowsPlatform()
    {
        String os = System.getProperty("os.name");
        if ( os != null && os.startsWith(WIN_ID))
            return true;
        else
            return false;
  
    }
    /**
     * Simple example.
     */
    public static void main(String[] args)
    {
        displayURL("http://www.javaworld.com");
    }
    // Used to identify the windows platform.
    private static final String WIN_ID = "Windows";
    // The default system browser under windows.
    private static final String WIN_PATH = "rundll32";
    // The flag to display a url.
    private static final String WIN_FLAG = "url.dll,FileProtocolHandler";
    // The default browser under unix.
    private static final String UNIX_PATH = "netscape";
    // The flag to display a url.
    private static final String UNIX_FLAG = "-remote openURL";
}
