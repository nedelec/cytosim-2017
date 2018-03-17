// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


/**
 return the maximum segmentation of all existing FiberProp,
 multiplied by 0.5
 */
real Simul::estimateFiberGridStep() const
{
    real res = -1;
    
    PropertyList plist = properties.find_all("fiber");
    
    for ( unsigned int n = 0; n < plist.size(); ++n )
    {
        FiberProp * fp = static_cast<FiberProp*>(plist[n]);
        if ( n == 0  ||  res < fp->segmentation )
            res = fp->segmentation;
    }

    if ( res <= 0 )
        return 1;
    else
        return res * 0.5;
}


/**
 Procedure:
 1. if binding_grid_step is not set, attempt to find a suitable value for it,
 2. if the number of cells is superior to 1e5, double the step size,
 2. initialize the grid with this calculated step size.
 */
void Simul::setFiberGrid(Space * spc) const
{
    assert_true(spc);
    real& step = prop->binding_grid_step;
    
    // automatic adjustment of cell size:
    if ( step <= 0 )
    {
        step = estimateFiberGridStep();
        //Cytosim::MSG("auto setting simul:binding_grid_step=%.3f\n", step);
    }

    assert_true( step > 0 );

    int res;
    do {
        res = fiberGrid.setGrid(spc, modulo, step, 1e5);
        if ( res )
        {
            step *= 2;
            //Cytosim::MSG("auto adjust simul:binding_grid_step=%.3f\n", step);
        }
    } while ( res != 0 );
    
    assert_true(fiberGrid.hasGrid());
}


//------------------------------------------------------------------------------

void Simul::step()
{
    assert_true(sReady);
    
    sTime += prop->time_step;
        
    /* 
     Lists of objects are mixed, to ensure that objects are
     considered in a different and random order at each step,
     which leads to better monte-carlo simulation results
     */
    organizers.mix();
    beads.mix();
    solids.mix();
    fibers.mix();
    spheres.mix();
    couples.mix();
    singles.mix();
    // Spaces are not mixed
    
    /*
     Step for every Objects
     */
    spaces.step();
    fields.step();
    organizers.step();
    spheres.step();
    beads.step();
    solids.step();
    fibers.step();
    
    /*
     Prepare the Grid for Hand binding interactions.
     Note: if Fibers never move, we could do this only once
     */
    if ( !fiberGrid.hasGrid() )
        setFiberGrid(space());
    
    //Cytosim::MSG(9, "grid range = %.2f nm\n", 1000 * HandProp::binding_range_max);
    fiberGrid.paintGrid(fibers.first(), 0, HandProp::binding_range_max);
    
    
#ifdef TEST_BINDING
    
    if ( HandProp::binding_range_max > 0 ) 
    {
        HandProp hp("test_binding");
        hp.binding_rate  = 10;
        hp.binding_range = HandProp::binding_range_max;
        hp.bind_also_ends = true;
        hp.complete(prop, 0);
        
        for ( unsigned int cnt = 0; cnt < TEST_BINDING; ++cnt )
        {
            Vector pos = space()->randomPlace();
            fiberGrid.testAttach(stdout, pos, fibers.first(), &hp);
        }
    }
    
#endif
       
    couples.step(fibers, fiberGrid);
    singles.step(fibers, fiberGrid);
}

