// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

/**
 Calculate the grid size automatically for dynamic Fiber,
 Bead, Sphere and Solid.
 
 This function can be used to set SimulProp::steric_max_range.\n
 
 We assume that Fiber::adjustSegmentation() is used, ensuring that
 ( actual segmentation ) < ( 4/3 * FiberProp::segmentation ).
 */
real Simul::estimateStericRange() const
{
    real ran = 0;
    real len = 0;
    
    ///check all FiberProp with enabled steric:
    PropertyList plist = properties.find_all("fiber");
    for ( PropertyList::iterator ip = plist.begin(); ip < plist.end(); ++ip )
    {
        FiberProp const* fp = static_cast<FiberProp*>(*ip);
        if ( fp->steric )
        {
            // The maximum length of a segment is 4/3 * segmentation
            real x = 4.1/3 * fp->segmentation;
            if ( len < x )
                len = x;
            
            // check extended range of interaction
            x = fp->steric_radius + fp->steric_range;
            if ( ran < x )
                ran = x;
        }
    }
    
    ///verify against the actual segmentations of the Fibers:
    for ( Fiber const* fib=fibers.first(); fib; fib=fib->next() )
    {
        if ( fib->prop->steric )
        {            
            // also check the actual segmentation
            real x = fib->segmentation();
            if ( len < x )
                len = x;
        }
    }

    /*
     The interaction can be aligned with the fiber, and we must add the distances:
     2 * range if two fibers of radius 'range' interact.
     + 2 * ( len / 2 ) since len/2 is the distance between the center of the segment
     and its most distal point.
     */
    ran =  len + 2*ran;
    
    
    for ( Sphere const* sp=spheres.first(); sp; sp=sp->next() )
    {
        real d = 2 * sp->radius();
        if ( sp->prop->steric && ran < d )
            ran = d;
    }
    
    for ( Bead const* bd=beads.first(); bd; bd=bd->next() )
    {
        real d = 2 * bd->radius();
        if ( bd->prop->steric && ran < d )
            ran = d;
    }
    
    for ( Solid const* so=solids.first(); so; so=so->next() )
    {
        if ( so->prop->steric )
        {
            for ( unsigned p = 0; p < so->nbPoints(); ++p )
            {
                real d = 2 * so->radius(p);
                if ( ran < d )
                    ran = d;
            }
        }
    }
    
    if ( ran < REAL_EPSILON )
        MSG_ONCE("Warning: could not estimate simul:steric_max_range automatically!\n");
    
    return ran;
}




void Simul::setStericGrid(Space const* spc) const
{
    assert_true(spc);
    real& range = prop->steric_max_range;
    
    if ( range <= 0 ) 
    {
        range = estimateStericRange();
        //Cytosim::MSG("auto setting simul:steric_max_range=%.3f\n", range);
    }
    
    if ( range > 0 ) 
        stericGrid.setGrid(spc, modulo, range);
    else
        throw InvalidParameter("simul:steric is enabled, but simul:steric_max_range was not set");
}


/**
 The prop->steric of each object is a bit-field that
 specify one or more 'pane' where the object is present.
 The different panes are then treated consecutively and independently, 
 and only objects in the same pane may interact.
 
 @code
 for ( int pane=1; pane<=2 && pane<=prop->steric; ++pane )
 {
     if ( obj->prop->steric & pane )
     ...
 }
 @endcode
 
 With this mechanism, the user can flexibly configure which objects
 may see each other and thus control the steric interactions.
 
 At present, we only support 1 pane (Simul property steric).
 This can be extended if necessary, but the steric_stiffness[]
 properties should be extended as well.
 */
void Simul::setStericInteractions(Meca& meca) const
{
    if ( !stericGrid.hasGrid() )
        setStericGrid(sSpace);

    // clear grid
    stericGrid.clear();
        
    // distribute Fiber-points on the grid
    for ( Fiber* fib=fibers.first(); fib; fib=fib->next() )
    {
        if ( fib->prop->steric )
        {
            const real rad = fib->prop->steric_radius;        // equilibrium radius
            const real ran = rad + fib->prop->steric_range;   // extended range of interaction
        
            // include segments, in the cell associated with their center
            for ( unsigned int r = 0; r < fib->nbSegments(); ++r )
                stericGrid.add(FiberLocus(fib, r), rad, ran);
        }
    }
    
    // include Spheres
    for ( Sphere* sp=spheres.first(); sp; sp=sp->next() )
    {
        if ( sp->prop->steric )
            stericGrid.add(PointExact(sp, 0), sp->radius());
    }
    
    // include Beads
    for ( Bead* bd=beads.first(); bd; bd=bd->next() )
    {
        if ( bd->prop->steric )
            stericGrid.add(PointExact(bd, 0), bd->radius());
    }
        
    // include Points that have a radius from Solids
    for ( Solid* so=solids.first(); so; so=so->next() )
    {
        if ( so->prop->steric )
        {
            for ( unsigned int pp = 0; pp < so->nbPoints(); ++pp )
            {
                if ( so->radius(pp) > REAL_EPSILON )
                    stericGrid.add(PointExact(so, pp), so->radius(pp));
            }
        }
    }
    
    
    /// create parameters
    PointGridParam pam(prop->steric_stiffness_push[0], prop->steric_stiffness_pull[0]);

    // add steric interactions
    stericGrid.setInteractions(meca, pam);

}


//------------------------------------------------------------------------------
/**
 This will:
 - Register all Mecables in the Meca: Fiber Solid Bead and Sphere
 - call setInteractions() for all objects in the system,
 - call setStericInteractions() if prop->steric is true.
 .
 */
void Simul::setInteractions(Meca & meca) const
{
    // prepare the meca, and register Mecables
    meca.clear();
    
    for ( Fiber  * mt= fibers.first(); mt ; mt=mt->next() )
        meca.add(mt);
    for ( Solid  * so= solids.first(); so ; so=so->next() )
        meca.add(so);
    for ( Bead   * bd=  beads.first(); bd ; bd=bd->next() )
        meca.add(bd);
    for ( Sphere * se=spheres.first(); se ; se=se->next() )
        meca.add(se);
    
    meca.prepare(prop);
    
    // add interactions
    
    for ( Space * sp=spaces.first(); sp; sp=sp->next() )
        sp->setInteractions(meca);
    
    for ( Fiber * mt=fibers.first(); mt ; mt=mt->next() )
        mt->setInteractions(meca);
    
    for ( Solid * so=solids.first(); so ; so=so->next() )
        so->setInteractions(meca);
    
    for ( Bead * bd=beads.first(); bd ; bd=bd->next() )
        bd->setInteractions(meca);
    
    for ( Sphere * sp=spheres.first(); sp ; sp=sp->next() )
        sp->setInteractions(meca);
    
    for ( Single * si=singles.firstA(); si ; si=si->next() )
        si->setInteractions(meca);

    for ( Couple * cx=couples.firstAA(); cx ; cx=cx->next() )
        cx->setInteractions(meca);
    
    for ( Organizer * as = organizers.first(); as; as=as->next() )
        as->setInteractions(meca);
    
    // add steric interactions
    if ( prop->steric && sSpace )
        setStericInteractions(meca);
    
}

//------------------------------------------------------------------------------
void Simul::solve()
{
    setInteractions(sMeca);

    sMeca.solve(prop, prop->precondition);
}


/**
 calculateForces() uses a dummy Meca to calculate the forces and
 set the Lagrange multipliers in the system
 */
void Simul::computeForces() const
{
    // make sure properties are ready for simulations:
    prop->strict = 1;
    prop->complete(prop, const_cast<PropertyList*>(&properties));
    Meca mec;
    setInteractions(mec);
    mec.computeForces();
    prop->strict = 0;
}
