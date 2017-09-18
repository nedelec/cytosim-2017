# Tutorial 5: Varying Parameters

Authors: Francois Nedelec (8.09.2017)

## Objective

Learn how to vary some parameter in the configuration file, perform simulations automatically and examine the result by making a plot. For this exercise, we will use the Unix command-line, which allow us to automate everything.

# 1. Preamble

First create a new directory for this tutorial. You need the cytosim executables `sim` and `play` and a configuration file `config.cym`. Copy these files into your new working directory (assuming here that it is called `tuto`).

    cd tuto
    cp ../cytosim/cym/aster.cym config.cym
    cp ../cytosim/bin/sim sim
    cp ../cytosim/bin/play play

#### Start a simple run:
    
    ./sim

This will read 'config.cym' and should produce the usual cytosim output files

#### Check the result visually:

    ./play

#### Save the output into new directory

Save a copy of the config file, and all the output files to a new directory `run0`

    mkdir run0
    mv *.cmo run0
    cp config.cym run0

#### You can verify the results again:

Play accepts a directory as argument, and in this case will display the simulation stored in this directory.

    ./play run0

# 2. Manually changing parameters

The configuration file is a plain text file and can be edited by many editors. You will get better results with editors made for editing source code, rather than word processor.

Manually edit the configuratino files, and run the simulation again, by repeating the same steps:

    ./sim
    ./play

Save this run into a new directory `run1`

    mkdir run1
    mv *.cmo run1
    cp config.cym run1

### Change the length of microtubules

In the configuration file, all the microtubule have the same length, but it is possible to let Cytosim chose length at random, following a decaying exponential distribution:

    new aster centrosome
    {
        ...
        fiber_length = 10, exponential
    }

Do you know why the exponential distribution is a good choice for microtubules?


# 3. Automatically varying parameters 

You will now use a script to automate the process of varying parameters. Specifically, you will create a 'template' configuration file, to define which parameters should be varied, and how they should be varied, and use `Preconfig` to generate multiple files.

### Prerequisites:

Get a number of useful tools: `preconfig`, `collect.py` and `scan.py` from GitHub:

    https://github.com/nedelec/preconfig
    https://github.com/nedelec/collect.py
    https://github.com/nedelec/scan.py
    https://github.com/nedelec/make_page.py

### Create a templated config file:

You should start from a regular config file, and edit it to add variations.
Duplicate the file and add '.tpl' to the extension:
    
    cp config.cym config.cym.tpl

Open the new file in your favorite editor, and add text on first line to define a variable `L` that will set the length of the microtubules:

    [[ L = random.uniform(5, 15) ]]% [[L]]

This will define `L` to be a random value between 5 and 15, and will also print the value of `L` in the first line.
In addition, replace the hard-coded definition of the microtubule length by the new variable.

    new aster centrosome
    {
        ...
        fiber_length = 10
    }

by

    new aster centrosome
    {
        ...
        fiber_length = [[L]]
    }

### Generate files using the templated configuration

This is automatically done by `Preconfig`:

    preconfig config.cym.tpl 32

This should create 32 files called `config0000.cym`, `config0001.cym`, etc.
Please, check the documentation of `Preconfig' if you want to know more.

### Prepare directories

We create here one directory for each of the configuration file, and move the files within these directories. This is done automatically by `collect.py`, another Python script:

    collect.py run%04i/config.cym config????.cym

You should know have directories called `run0000`, `run0001`, each containing one file `config.cym`.

### Compute the simulations

You can now run the executable `sim` in each of the `run` directory. This is done using the script `scan.py`:

    scan.py ../sim run????

It can take some time to compute all the simulations. If your computer has multiple cores, you can also run a number of processes in parallel, as below for 4 cores:

    scan.py ../sim nproc=4 run????

### Examine some results

At this stage, you can manually examine some of these runs, for example:

    ./play run0003
    ./play run0004

### Generate pictures for all simulations 

Because each run in stored in its own directory, you can use `scan.py` to perform various things. Here one genrates one image of the last time point:

    scan.py "../play image frame=1000" run????

To display all these pictures, we will use a browswer, and generate the HTML page with `make_page.py`:

    make_page.py tile=5 run????
    
This should generate a file `page.html`, which can be openned in your favorite browser.


## Alternative sampling strategies

Instead of using a random value for `L`, you could also sample the segment regularly using this command:

    [[ L=range(5, 15) ]]% [[L]]

This will define `L` to take all integral values between 5 and 15. You can refine the sampling for example with 100 values, with:

    [[ X = range(0, 100) ]] [[ L = 5 + 0.1*X ]]% [[L]]

Take a minute to consider the advantages and diadvantages of this method over the random sampling method.

### Sorted random sampling

A good compromise is to use random values, but to sort these values such as to scan through the parameter values in a natural order. This can be done with Preconfig using this code:

    [[ L = sorted([random.uniform(0.10, 0.25) for i in range(256)]) ]]

## Another scan

To make sure that you are confortable with the whole process, you should now perform another similar scan, but with the aster placed initially at the center of the box:

    new aster centrosome
    {
        ...
        position = center
    }

By default, if `position` is not specified, an object is placed in a random position of the box. 
Before you start, make sure that you backup all the previous simulation into a new subdirectory.
Chose the sampling strategy that you think is most adapted to this problem.

With this new setup, how is the question that the simulation is addressing changed? 

## Create a Python script to plot the results

Our usual approach is to write a custom Python script to analyse data from multiple runs. In this case, the objective would be to eventually make a plot with on the X-axis the length of the microtubule `L`, and on the Y-axis the distance between the centrosome to the center of the box. Such plot will show us under which condition the aster is centered.

This is the topic of the next tutorial.


## The end

Congratulation, you have completed the tutorial.

