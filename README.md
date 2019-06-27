# Cytosim

**This is an older version of Cytosim.** Please go to:

	https://gitlab.com/f.nedelec/cytosim

## Description
Cytosim is a cytoskeleton simulation engine written in C++ working on Mac OS, GNU/Linux and Windows (with Cygwin).

The objects in the simulation and their parameters are defined within a single text file `config.cym`:

~~~
set simul example 
{
    time_step = 0.005
    viscosity = 0.02
}

set space cell
{
    shape = sphere
    dimensions = 5
}

new space cell

set fiber microtubule
{
    rigidity = 20
    segmentation = 0.5
    confine = inside, 200, cell
}

new fiber microtubule
{
    length = 11
}

run 5000 simul example
{
    nb_frames = 10
}
~~~


## Documentation
The documentation can be generated by [doxygen](http://www.stack.nl/~dimitri/doxygen/):

~~~bash
doxygen src/doc/Doxyfile
~~~

This will generate an html folder in the root directory.

You can now open `html/index.html` in a browser.

## Installation and compilation

In the documentation, you will find a module describing how to compile Cytosim. On Mac OS X and Linux this should be straightforward if you are familiar with compilation in general. On Windows, we suggest that you compile within Cygwin. For more information, please check src/doc/compile.dox.

After installing Apple's Xcode, enter these commands to compile:

	git clone https://github.com/nedelec/cytosim.git
	cd cytosim
	make

## Usage

Once cytosim is running on your machine, check the tutorials, the page on "Running Simulations", and the examples contained in the folder "cym". We recommend starting with the shortest configuration files (e.g. fiber.cym). Finally, do not hesitate to contact us if you have difficulties: cytosim@cytosim.org

We hope cytosim can be useful for your research. 

Sincerely yours, The Developers of Cytosim
