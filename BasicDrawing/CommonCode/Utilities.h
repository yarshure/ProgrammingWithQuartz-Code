/*
*  File:    Utilities.h
*  
*  Copyright:  Copyright © 2005 Apple Computer, Inc., All Rights Reserved
* 
*  Disclaimer:  IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*        consideration of your agreement to the following terms, and your use, installation, modification 
*        or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*        not agree with these terms, please do not use, install, modify or redistribute this Apple 
*        software.
*
*        In consideration of your agreement to abide by the following terms, and subject to these terms, 
*        Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*        original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*        Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*        redistribute the Apple Software in its entirety and without modifications, you must retain this 
*        notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*        Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*        endorse or promote products derived from the Apple Software without specific prior written 
*        permission from Apple.  Except as expressly stated in this notice, no other rights or 
*        licenses, express or implied, are granted by Apple herein, including but not limited to any 
*        patent rights that may be infringed by your derivative works or by other works in which the 
*        Apple Software may be incorporated.
*
*        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*        IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*        AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*        OR IN COMBINATION WITH YOUR PRODUCTS.
*
*        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*        OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*        REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*        UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*        IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#ifndef __Utilities__
#define __Utilities__

#include <ApplicationServices/ApplicationServices.h>
#include <math.h>		// for M_PI

typedef struct ExportInfo{
    OSType		command;
    OSType		fileType;
    Boolean		useQTForExport;
    int			dpi;
}ExportInfo;


static inline float DEGREES_TO_RADIANS(float degrees){
	return degrees * M_PI/180;
}

CFBundleRef getAppBundle(void);

void setScalingFactor(float dpi);
float getScalingFactor(void);

CGColorSpaceRef getTheCalibratedRGBColorSpace(void);
CGColorSpaceRef getTheCalibratedGrayColorSpace(void);
CGColorSpaceRef getTheSRGBColorSpace(void);
CGColorSpaceRef getTheDisplayColorSpace(void);

CGColorRef getRGBOpaqueWhiteColor(void);
CGColorRef getRGBOpaqueBlackColor(void);
CGColorRef getRGBOpaqueRedColor(void);
CGColorRef getRGBOpaqueBlueColor(void);
CGColorRef getRGBOpaquePurpleColor(void);
CGColorRef getRGBOpaqueDarkBlueColor(void);
CGColorRef getRGBOpaqueBrownColor(void);
CGColorRef getRGBOpaqueOrangeColor(void);
CGColorRef getRGBOpaqueYellowColor(void);
CGColorRef getRGBOpaqueGreenColor(void);
CGColorRef getOpaqueGrayColor(void);
CGColorRef getRGBOpaqueDarkGreenColor(void);

void myCGContextAddEllipseInRect(CGContextRef context, CGRect r);

// Debugging aids.
void drawPoint(CGContextRef context, CGPoint p);
void printCTM(CGContextRef context);
void drawCoordinateAxes(CGContextRef context);
void drawDebuggingRect(CGContextRef context, CGRect rect);

#endif	// __Utilities__
