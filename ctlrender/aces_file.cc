///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences 
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
// 
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted, 
// subject to acceptance of this license. Performance of any of the 
// aforementioned acts indicates acceptance to be bound by the following 
// terms and conditions:
//
//  * Copies of source code, in whole or in part, must retain the 
//    above copyright notice, this list of conditions and the 
//    Disclaimer of Warranty.
//
//  * Use in binary form must retain the above copyright notice, 
//    this list of conditions and the Disclaimer of Warranty in the
//    documentation and/or other materials provided with the distribution.
//
//  * Nothing in this license shall be deemed to grant any rights to 
//    trademarks, copyrights, patents, trade secrets or any other 
//    intellectual property of A.M.P.A.S. or any contributors, except 
//    as expressly stated herein.
//
//  * Neither the name "A.M.P.A.S." nor the name of any other 
//    contributors to this software may be used to endorse or promote 
//    products derivative of or based on this software without express 
//    prior written permission of A.M.P.A.S. or the contributors, as 
//    appropriate.
// 
// This license shall be construed pursuant to the laws of the State of 
// California, and any disputes related thereto shall be subject to the 
// jurisdiction of the courts therein.
//
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO 
// EVENT SHALL A.M.P.A.S., OR ANY CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE 
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, RESITUTIONARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY 
// SPECIFICALLY DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER 
// RELATED TO PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACADEMY 
// COLOR ENCODING SYSTEM, OR APPLICATIONS THEREOF, HELD BY PARTIES OTHER 
// THAN A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
///////////////////////////////////////////////////////////////////////////

#include "aces_file.hh"
#include <iostream>

#if defined( HAVE_ACESFILE )
#include <aces_Writer.h>
#include <stdexcept>
#include <half.h>

void aces_write(const char *name, float scale, 
               uint32_t width, uint32_t height, uint32_t channels,
               const float *pixels,
               format_t *format) {

	std::vector<halfBytes> scaled_pixels;
	
    {
        float const *in = pixels;
        if (scale == 0.0f) scale = 1.0f;
        
        scaled_pixels.resize(height*width*channels);
        halfBytes *out = &scaled_pixels[0];
        for(size_t i=0; i<scaled_pixels.size(); i++) {
			half tmpV( *(in++) / scale );
            *(out++)=tmpV.bits();
        }
    }

    halfBytes *in = &scaled_pixels[0];
	
    std::vector<std::string> filenames;
	filenames.push_back( name );
	
	aces_Writer x;
	
	MetaWriteClip writeParams;
	
	writeParams.duration				= 1;	
	writeParams.outputFilenames			= filenames;
	
	writeParams.outputRows				= height;
	writeParams.outputCols				= width;	
	
	writeParams.hi = x.getDefaultHeaderInfo();	
	writeParams.hi.originalImageFlag	= 1;	
	writeParams.hi.software				= "ctlrender";

	writeParams.hi.channels.clear();
	switch ( channels )
	{
		case 3:
			writeParams.hi.channels.resize(3);
			writeParams.hi.channels[0].name = "B";
			writeParams.hi.channels[1].name = "G";
			writeParams.hi.channels[2].name = "R";
			break;
		case 4:
			writeParams.hi.channels.resize(4);
			writeParams.hi.channels[0].name = "A";
			writeParams.hi.channels[1].name = "B";
			writeParams.hi.channels[2].name = "G";
			writeParams.hi.channels[3].name = "R";
			break;
		case 6:
			throw std::invalid_argument("Stereo RGB support not yet implemented");
//			writeParams.hi.channels.resize(6);
//			writeParams.hi.channels[0].name = "B";
//			writeParams.hi.channels[1].name = "G";
//			writeParams.hi.channels[2].name = "R";
//			writeParams.hi.channels[3].name = "left.B";
//			writeParams.hi.channels[4].name = "left.G";
//			writeParams.hi.channels[5].name = "left.R";
//			break;
		case 8:
			throw std::invalid_argument("Stereo RGB support not yet implemented");
//			writeParams.hi.channels.resize(8);
//			writeParams.hi.channels[0].name = "A";
//			writeParams.hi.channels[1].name = "B";
//			writeParams.hi.channels[2].name = "G";
//			writeParams.hi.channels[3].name = "R";
//			writeParams.hi.channels[4].name = "left.A";
//			writeParams.hi.channels[5].name = "left.B";
//			writeParams.hi.channels[6].name = "left.G";
//			writeParams.hi.channels[7].name = "left.R";
//			break;
		default:
			throw std::invalid_argument("Only RGB, RGBA or stereo RGB[A] file supported");
			break;
	}

	DynamicMetadata dynamicMeta;
	dynamicMeta.imageIndex = 0;
	dynamicMeta.imageCounter = 0;
	
	x.configure ( writeParams );
	x.newImageObject ( dynamicMeta );		

	for ( uint32 row = 0; row < height; row++) {
		halfBytes *rgbData = in + width*channels*row;
		x.storeHalfRow ( rgbData, row ); 
	}

#if 0
	std::cout << "saving aces file" << std::endl;
	std::cout << "size " << width << "x" << height << "x" << channels << std::endl;
	std::cout << "size " << writeParams.outputCols << "x" << writeParams.outputRows << std::endl;
	std::cout << "duration " << writeParams.duration << std::endl;
	std::cout << writeParams.hi;
	std::cout << "\ndynamic meta" << std::endl;
	std::cout << "imageIndex " << dynamicMeta.imageIndex << std::endl;
	std::cout << "imageCounter " << dynamicMeta.imageCounter << std::endl;
	std::cout << "timeCode " << dynamicMeta.timeCode << std::endl;
	std::cout << "keyCode " << dynamicMeta.keyCode << std::endl;
	std::cout << "capDate " << dynamicMeta.capDate << std::endl;
	std::cout << "uuid " << dynamicMeta.uuid << std::endl;
#endif

	x.saveImageObject ( );	
}

#else 

void aces_write(const char *name, float scale,
                uint32_t width, uint32_t height, uint32_t channels,
                const float *pixels,
                format_t *format)
{
	std::cerr << "AcesContainer library not found" << std::endl;
}

#endif


