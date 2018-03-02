### Instructions to run Cytosim on Cygwin:

Use the cytosim version provided (CytosimV) which has a modified makefile.inc and disabled offscreen rendering. If this verison is missing 
any packages you need let me know and I can send you a more complete version that also includes some of my modifications to the base code.


Run the Cygwin installer with default settings until you get to the package selection screen. In order to properly compile and run Cytosim 
several packages which are not default must be selected, do this by searching for the keywords provided and selecting items that mention
"devel" or "lib." To select items for installation click the weird icon to the left of "Skip" or "Default" until it reads "Install" instead.

- libX11    	 	(to make X11 launcher functional)
- libLAS			(LAS file reader)
- liblapack		(Linear algebra functions for shell)
- automake		(makefile generator tools)
- cmake			(makefile generator tools)
- gcc				(devel tools for C++)
- OpenGL			(graphical display tools for 'play')
- python3-devel	(python interpreter for cygwin)

In the view drop-down menu select "Category" which will reorganize packages to show them by category. Rather than go through the Developer
tools line by line, click on the icon between "Devel" and "Default" to change it to "Install." This will install all the developer tools
for Cygwin, which is overkill and will take ~20-30 minutes, but is much easier than figuring out which tools are/aren't neccessary.
After all packages are selected install Cygwin.  

### Display

Finally, to run X11 display for simulations ('play' command) it may be necessary to install Xming and launch an X11 instance through XLaunch.
Install Xming while Cygwin installation is running. Once Xming is installed run XLaunch to initiate an instance of X11 to "display 0" 
which is the default. From the Cygwin command line run the following command:

    export DISPLAY=:0   

This will allow 'play' to port display information to display 0 which will let Windows run it on the X11 instance. This command should be 
run every time the cygwin command line is closed and opened again, otherwise the display will fail and 'play' command will do nothing. 
Alternatively this command could be appended to the initiation commands for the cygwin command line (I don't recall how to do this, but it 
should be easy to figure out). 


Author: Daniel Cortes, 16 Dec. 2017