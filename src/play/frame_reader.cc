// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "frame_reader.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"

//#define VERBOSE_READER

//------------------------------------------------------------------------------

FrameReader::FrameReader() : inw(0)
{
    clearPositions();
}

void FrameReader::rewind()
{
    inw.rewind();
    clearPositions();
}

void FrameReader::openFile(std::string& file)
{
    clearPositions();
    
    int error = inw.open(file.c_str(), "rb");
    
    if ( error )
    {
        //file was not found, we try 'gunzip'
        std::string tmp = file + ".gz";
        FILE* fp = fopen(tmp.c_str(), "r");
        if ( fp )
        {
            fclose( fp );
            tmp = "gunzip " + tmp;
            std::cerr << tmp << std::endl;
            
            if ( 0 == system(tmp.c_str()) )
                inw.open(file.c_str(), "rb");
        }
    }
    
    if ( !inw.file() )
        throw InvalidIO("file `"+file+"' not found");

    if ( inw.error() )
        throw InvalidIO("file `"+file+"' is invalid");
    
    //std::cerr << "FrameReader: has openned " << obj_file << std::endl;
}


int FrameReader::badFile()
{
    if ( 0 == inw.file() )
        return 1;
    
    if ( inw.eof() )
        inw.clearerr();
    
    if ( ! inw.good() )
        return 2;
    
    return 0;
}


void FrameReader::checkFile()
{
    if ( 0 == inw.file() )
        throw InvalidIO("No open file");
    
    if ( inw.eof() )
        inw.clearerr();
    
    if ( ! inw.good() )
        throw InvalidIO("File has errors");
}

//------------------------------------------------------------------------------
#pragma mark -

void FrameReader::clearPositions()
{
#ifdef VERBOSE_READER
    std::cerr << "FrameReader: clear" << std::endl;
#endif
    
    curFrame = -1;
    framePos.clear();
    framePos.reserve(1024);
}


void FrameReader::savePos(int frm, const fpos_t& pos, int s)
{
    if ( frm < 0 )
        return;
    
    unsigned inx = frm;
    
    if ( inx >= framePos.capacity() )
    {
        const unsigned chunk = 1024;
        unsigned sz = ( inx + chunk - 1 ) & -chunk;
        framePos.reserve(sz);
    }
    
    if ( inx >= framePos.size() )
    {
        size_t i = framePos.size();
        framePos.resize(inx+1);
        while ( i <= inx )
            framePos[i++].status = 0;
    }
    
    if ( framePos[inx].status < s )
    {
        framePos[inx].status = s;
        framePos[inx].value = pos;
    
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: learned position of frame " << frm << std::endl;
#endif
    }
}


/**
 This uses the current knowledge to move to a position
 in the file where we should find frame \a frm.
*/
int FrameReader::seekPos(int frm)
{
    if ( inw.eof() )
        inw.clearerr();
    
    if ( frm < 1 || framePos.size() < 1 )
    {
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: rewind 0" << std::endl;
#endif
        inw.rewind();
        return 0;
    }
    
    unsigned int inx = frm;
    
    if ( inx >= framePos.size() ) 
        inx = framePos.size() - 1;

    while ( inx > 0  &&  framePos[inx].status == 0 )
        --inx;
    
    //check if we know already were the frame starts:
    if ( 0 < inx )
    {
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: using known position of frame " << inx << std::endl;
#endif
        inw.set_pos(framePos[inx].value);
        return inx;
    }
    else {
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: rewind " << std::endl;
#endif
        inw.rewind();
        return 0;
    }
}


int FrameReader::lastFrame() const
{
    int res = framePos.size() - 1;
    while ( 0 < res  &&  framePos[res].status < 2 )
        --res;
    return res;
}

//------------------------------------------------------------------------------
#pragma mark -

/**
 scan file forward from current position to find the next occurence of FRAME_TAG
 @return 0 if the frame was found
*/
int FrameReader::seekFrame(const int frm)
{        
#ifdef VERBOSE_READER
    std::cerr << "FrameReader: seekFrame("<< frm <<")" << std::endl;
#endif
    
    int inx = seekPos(frm);
    
    if ( inx == frm )
        return 0;
    
    fpos_t pos;
    std::string line;

    while ( ! inw.eof() )
    {
        do {

            inw.get_pos(pos);
            inw.get_line(line);
            
            if ( inw.eof() )
                return 1;
            
#ifdef BACKWARD_COMPATIBILITY
            if ( 0 == line.compare(0, 7, "#frame ") )
                break;
#endif
            
        } while ( line.compare(0, sizeof(FRAME_TAG)-1, FRAME_TAG) );

        if ( ! inw.eof() )
        {
            savePos(inx, pos, 2);
            if ( inx == frm )
            {
                inw.set_pos(pos);
                return 0;
            }
            ++inx;
        }
    }
    
#ifdef VERBOSE_READER
    std::cerr << "FrameReader: seekFrame("<< frm <<") encountered EOF" << std::endl;
#endif
    return 1;
}

//------------------------------------------------------------------------------
/** 
 returns 0 for success, an error code, or throws an exception
 */
int FrameReader::readFrame(Simul& sim, int frm, const bool reload)
{
    if ( badFile() )
        return 1;

#ifdef VERBOSE_READER
    std::cerr << "FrameReader: readFrame("<<frm<<", "<<reload <<")" << std::endl;
#endif
    
    // a negative index is counted from the end
    if ( frm < 0 )
    {
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: counting down from frame "<< lastFrame() << std::endl;
#endif
        frm += 1 + lastFrame();
        if ( frm < 0 )
            frm = 0;
    }
    
    // what we are looking for might already be in the buffer:
    if ( frm == curFrame && ! reload )
        return 0;
    
    // it might be the next one in the buffer:
    if ( frm == 1+curFrame )
        return readNextFrame(sim);

    //---------------------try to find the start tag from there:
    
    if ( 0 != seekFrame(frm) )
        return 1;
    
    //--------------------read the sim-state from there:
    
#ifdef VERBOSE_READER
    std::cerr << "FrameReader: reading frame "<< frm << std::endl;
#endif
    
    fpos_t pos;
    inw.get_pos(pos);

    if ( 0 == sim.reloadObjects(inw) )
    {
    
#ifdef VERBOSE_READER
        std::cerr << "FrameReader:readFrame("<< frm <<") successful" << std::endl;
#endif
        curFrame = frm;
        savePos(curFrame, pos, 3);

        // the next frame should start at the current position:
        inw.get_pos(pos);
        savePos(curFrame+1, pos, 1);
        return 0;
    }
    else
    {
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: readFrame("<< frm <<") EOF at frame " << frm << std::endl;
#endif
        return 1;
    }
}

//------------------------------------------------------------------------------
int FrameReader::readFrameCatch(Simul& sim, const int frm, const bool reload)
{
    try {
        
        return readFrame(sim, frm, reload);
    
    }
    catch( Exception & e ) {
        
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: Error in frame " << frm <<" : " << e.what() << std::endl;
#endif
        // the next frame should start after this position,
        // but there was an error, and the position might not be reliable

        return 6;
        
    }
}


//------------------------------------------------------------------------------
/** 
 returns 0 for success, an error code, or throws an exception
 */
int FrameReader::readNextFrame(Simul& sim)
{
    if ( badFile() )
        return 1;
    
    fpos_t pos;
    inw.get_pos(pos);

    if ( 0 == sim.reloadObjects(inw) )
    {
        ++curFrame;
        
        // the position we used was good, to read this frame
        savePos(curFrame, pos, 3);

#ifdef VERBOSE_READER
        std::cerr << "FrameReader: readNextFrame() read frame " << frame() << std::endl;
#endif
        
        // the next frame should start from the current position:
        inw.get_pos(pos);
        savePos(curFrame+1, pos, 1);
        return 0;
    } 
    else
    {
#ifdef VERBOSE_READER
        std::cerr << "FrameReader: readNextFrame() EOF after frame " << frame() << std::endl;
#endif
        return 1;
    }
}

