## Instructions to run Cytosim on Cygwin

Cytosim runs on Cygwin, but this is not supported. Typically this mode is only used by third parties.

### Installation

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

### Compilation

Modify the `makefile.inc` to set `MACHINE:=cygwin`, and disable offscreen rendering by setting `HAS_PNG:=0`. Recompile fresh using the toolchain (`gcc` and `make`) provided by cygwin. The procedure is the same as on other platforms (enter `make`). If you experience trouble, please let us know.

### X11 display

Finally, the 'play' command of cytosim will use X11 to open its display window, and you need to install and run a X11 window server on your local computer. Two X11 servers models can be used with Cygwin:

[Cygwin/X](https://en.wikipedia.org/wiki/Cygwin/X)

[Xming](https://en.wikipedia.org/wiki/Xming)

We got things to work with the free 2007 version of Xming. Xming can be installed while Cygwin installation is running. 
Once Xming is installed run XLaunch to start the X11 window server, and set the "display" of your application to this server. From the Cygwin command line run the following command:

    export DISPLAY=:0   

This will allow 'play' to send display information to display 0 which will address the default X11 server. This command should be run once every time a new cygwin terminal is opened, otherwise 'play' will fail with a message "failed to open display". 


Author: Daniel Cortes and Francois Nedelec, 3 March 2018