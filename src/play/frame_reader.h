// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "iowrapper.h"
#include <vector>
class Simul;


/// Class to read a particular frame in a trajectory file
/** 
 FrameReader is used to find a particular frame (eg. frame 10) in a trajectory file,
 and it will handle basic IO failures.
 FrameReader will remember the starting points of all frames that were found,
 and will use this information to speed up future access to these and other frames.

 FrameReader makes minimal assuptions on what constitutes a 'frame':
 - It looks for a string-tag present at the start of a frame (FRAME_TAG).
 - It calls Simul::reloadObjects() to actually read the simulation state.
 .
*/
class FrameReader
{    
private:
    
    /// accessory class to store a position in a file
    class file_pos 
    {
    public:
        int    status;   ///< indicates that \a value is not valid
        fpos_t value;
        file_pos() { status=0; }
    };
    
    /// type for list of positions
    typedef  std::vector<file_pos> PosList;
    
    /// the stream from which input is made
    InputWrapper inw;
    
    /// starting position for each frame
    PosList  framePos;
    
    /// index of frame stored currently in Simul
    int      curFrame;

    /// remember position \pos as the place where frame \a frm should start
    void     savePos(int frm, const fpos_t& pos, int status);
   
    /// go to a position where a frame close to \a frm is known to start
    int      seekPos(int frm);
    
    /// check file validity
    void     checkFile();
    
    /// return 0 if file is good for input
    int      badFile();
    
public:
    
    /// constructor, after which openFile() should be called
    FrameReader();
    
    /// destructor
    virtual ~FrameReader() {}
    
    /// open file for input
    void     openFile(std::string& file);
    
    /// clear the buffer
    void     clearPositions();
    
    /// last frame seen in the file
    int      lastFrame() const;
    
    /// true at end of file
    bool     eof()      { return inw.eof();  }

    /// true if everything looks correct for input
    bool     good()     { return inw.good(); }

    /// rewind
    void     rewind();
    
    /// return index of current frame 
    int      frame() const { return curFrame; }

    /// find the starting point of frame by brute force !
    int      seekFrame(int frm);
    
    /// read the frame specified by index, storing it in global variable 'sim'
    int      readFrame(Simul&, int frm, bool reload = false);
    
    /// read the frame specified by index, storing it in global variable 'sim'
    int      readFrameCatch(Simul&, int frm, bool reload = false);
    
    /// read the next frame in the file, return 1 for EOF
    int      readNextFrame(Simul&);
        
};


