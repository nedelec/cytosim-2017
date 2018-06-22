% A confined aster with cortical pulling motors
% F. Nedelec, April 2010

[[ xx=linspace(0,100)]]
[[ x=-(1.0*xx)/100.0]]
% set x [[x]]



set simul aster_pull 
{
 time_step = 0.001
    viscosity = 2
kT=0.00001
    }
    

set space cell
{
    geometry = ( sphere 1 )
    display = (visible=1	;color=0xA5A5A555,0x000F0055 ;)
}

new space cell

set fiber microtubule
{
    rigidity = 0.25
    segmentation = 0.01
    display = ( line=4, 2; )
     confine = inside, 10
 
 	    activity       = classic
    growing_speed            = 1
   shrinking_speed          = -2
    catastrophe_rate         = 0.01, 200.0
rescue_rate            = 30
	growing_force  = 1.0
	fate=rescue    
	min_length     =0.01
	colinear_force = 2,1
    	catastrophe_outside=1
 
}

set bead core
{
    display = ( style=3 )
attach=1000.0
attach_point = [[x]] 0 0

}

set aster centrosome
{
    solid = core
    fibers = microtubule
    stiffness = 400,400
}

new aster centrosome
{
    radius = 0.005
%	fiber_separation=0.001
   nb_fibers = 500
    length = 0.01
       position = [[x]] 0 0
}



run simul * 
{
    nb_steps = 5000
    nb_frames =0
}
run simul * 
{
    nb_steps = 20
    nb_frames =20
}


%change bead:display core (color=red)
%change bead:attach core (0)
%change fiber:viscosity microtubule (10)

%run simul *
%{
%    nb_steps = 9000
%    nb_frames = 300	
%}


