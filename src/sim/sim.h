// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
/**
 @file
 @brief Global Compile Switches
*/

#ifndef SIM_H
#define SIM_H



/**
 Enables simulation of uniform fluid flow using parameter 'Simul::flow'
 Option normally OFF
 */
//#define NEW_CYTOPLASMIC_FLOW


/**
 Display the Meca-links with OpenGL 
 This only works for DIM > 1, and if compilation is done in Xcode
 Option normally OFF
 */
//#define DISPLAY_INTERACTIONS


/**
 Include a test of the attachement algorithm 
 Option normally OFF
 */
//#define TEST_BINDING 10


/**
 Enable code to be able to read old trajectory files
 Option normally ON
 */
#define BACKWARD_COMPATIBILITY


#endif
