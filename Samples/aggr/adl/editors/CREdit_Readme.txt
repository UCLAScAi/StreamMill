
Here is how to customize the CR editor for ATLaS

1. (Syntax Highlighting) Open  CREdit.
In the Options menu, click on "Syntax Highlighting" 
and then Import/open the file:

C:/Atlas/editors/syntax_CREdit.ini

2. (Adding ATLaS command to the Tool menu). 
EXIT CREdit and Copy C:/Atlas/editors/CREdit.INI 
into the system directory 
	C:/WINNT for win 2000;
	c:/windows for win98/XP;
[But save the old copy of CREdit.INI into another file
  before this operation]

3. (Checking commands) Open  CREdit. In the Tools menu, 
you should now find the following commands:

 * Compile_ATLaS (generates a C:/Atlas/adlout.o) file
 
 * Exec_ATlaS: this executes the compiled program 
   and return the results in the bottom window.
 
 * Exec_Atlas>>file: this will

   (i) prompt you for a file name, 
   (ii) put the output in that file,
   (iii) open the file according to its type--e.g.,
         html files will be open in the broswer. 

4. You might want to associate CREdit with *.adl files
  
 
 
 
 
 
 
 