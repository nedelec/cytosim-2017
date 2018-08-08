## Instructions to run Cytosim on Cygwin

Cytosim runs on Cygwin, but this is not supported. Typically this mode is only used by third parties.

### Installation

Run the Cygwin installer with default settings until you get to the package selection screen. In order to properly compile and run Cytosim 
several packages which are not default must be installed:

- gcc				(GNU compiler suite including C++)
- make				(GNU make)
- libBLAS			(Basic Linear Algebra Subprograms)
- libLAPACK		(Linear Algebra PACKage)
- X11    	    	(X Window System)
- automake		(makefile generator tool)
- cmake			(makefile generator tool)
- OpenGL			(Open Graphics Library)
- GLEW				(OpenGL Extension Wrangler Library)
- python3     	(python interpreter for cygwin)

Searching for the keywords provided and selecting items that mention
"devel" or "lib". To select items for installation click the weird icon to the left of "Skip" or "Default" until it reads "Install" instead.
After all packages are selected install Cygwin.  

**Tip**: In the view drop-down menu select "Category" which will reorganize packages to show them by category. Rather than go through the Developer
tools line by line, click on the icon between "Devel" and "Default" to change it to "Install". This will install all the developer tools
for Cygwin, which is overkill and will take ~20-30 minutes, but is much easier than figuring out which tools are/aren't neccessary.

### Compilation

Modify the `makefile.inc` to set `MACHINE:=cygwin`, and disable offscreen rendering by setting `HAS_PNG:=0`. Recompile fresh using the toolchain (`gcc` and `make`) provided by cygwin. The procedure is the same as on other platforms (enter `make`). If you experience trouble, please let us know.

### The X Window System (X11)

Finally, all graphical tools included in cytosim (in particular 'play') will use the [X Window System](https://en.wikipedia.org/wiki/X_Window_System) also known as X11 to open a window, and this will work only if a X11 server is running on your local computer. Two X11 implementations can be installed with Cygwin:

[Cygwin/X](https://en.wikipedia.org/wiki/Cygwin/X)

[Xming](https://en.wikipedia.org/wiki/Xming)

We got things to work with the free 2007 version of Xming. Xming can be installed while Cygwin installation is running. 
Once Xming is installed run XLaunch to start the X11 window server, and set the "display" of your application to this server. From the Cygwin command line enter the following command:

    export DISPLAY=:0

This will instruct 'play' to send X11 queries to display 0 which will address the default X11 server on the local machine. This command should be run once every time a new cygwin terminal is opened, otherwise 'play' will fail with a message "failed to open display". 

**Tip**: To debug X11, it may be useful to run standard program such as `xclok`. This way you can test if the X11 server is operational.

### Starting

Cytosim will only work within Cygwin, and not directly under Windows.

This means that Cytosim should be started from the Cygwin terminal window.
It cannot be started from the Windows command line, and you can close the Windows/MS-DOS command prompt. 

    cd cytosim
    bin/play live   

Author: Daniel Cortes and Francois Nedelec, 23 March 2018