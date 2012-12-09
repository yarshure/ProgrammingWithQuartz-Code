/*
*  File:    UIHandling.h
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

#ifndef __UIHandling__
#define __UIHandling__

typedef enum {
    /***    Drawing Basics ***/
	kHICommandSimpleRect		    = 1000,
	kHICommandStrokedRect		    = 1001,
	kHICommandStrokedAndFilledRect	= 1002,   
	kHICommandPathRects				= 1003,
	kHICommandAlphaRects		    = 1004,
	kHICommandDashed				= 1005,
	kHICommandSimpleClip		    = 1006,
	kHICommandPDFDoc				= 1007,

    /***    Coordinate System ***/
	kHICommandRotatedEllipses	    = 1008,
	kHICommandDrawSkewCoordinates	= 1082,

    /***    Path Drawing ***/
	kHICommandBezierEgg				= 1009,
	kHICommandRoundedRects		    = 1010,
	kHICommandStrokeWithCTM		    = 1011,
	kHICommandRotatedEllipsesWithCGPath = 1012,
	kHICommandPixelAligned		    = 1099,

    /***    Color and GState ***/
	kHICommandDeviceFillAndStrokeColor  = 1013,
	kHICommandCLUTDrawGraphics	    = 1014,
	kHICommandDrawWithGlobalAlpha	= 1015,
	kHICommandDrawWithBlendMode	    = 1068,
	kHICommandDrawWithColorRefs	    = 1016,
	kHICommandFunctionsHaveOwnGSave	= 1017,

    /***    Images ***/
	kHICommandDrawJPEGImage		    = 1018,
	kHICommandColorImageFromFile	= 1019,
	kHICommandColorImageFromData	= 1020,
	kHICommandColorImageFromCallbacks	= 1021,
	kHICommandGrayRamp				= 1022,
	kHICommandDrawWithCGImageSource	= 1023,
	kHICommandDrawWithCGImageSourceIncremental = 1024,
	kHICommandDrawWithQuickTime		= 1025,
	kHICommandSubstituteImageProfile	= 1026,
	kHICommandDoSubImage			= 1027,
	kHICommandExportWithQuickTime	=  1028,

    /***    Image Masking ***/
	kHICommandMaskTurkeyImage		= 1029,
	kHICommandImageMaskedWithMask	= 1030,
	kHICommandImageMaskedWithGrayImage	= 1031,
	kHICommandMaskImageWithColor	= 1033,
	kHICommandClipToMask			= 1080,
	kHICommandExportWithCGImageDestination	= 1034,

    /*** Bitmap Graphics Context and CGLayerRef ***/
	kHICommandSimpleCGLayer			= 1090,
	kHICommandAlphaOnlyContext		= 1091,
	kHICommandDrawNoOffScreenImage	= 1035,
	kHICommandDrawOffScreenImage	= 1036,
	kHICommandDrawWithLayer			= 1037,

    /***    Text ***/
	kHICommandQuartzRomanText		= 1038,
	kHICommandQuartzTextModes		= 1039,
	kHICommandQuartzTextMatrix		= 1040,
	
	kHICommandDrawNSString			= 1041,
	kHICommandDrawNSLayoutMgr		= 1042,
	kHICommandDrawCustomNSLayoutMgr	= 1043,

    /***    Advanced Drawing ***/
	kHICommandSimplePattern			= 1050,
	kHICommandPatternMatrix			= 1051,
	kHICommandPatternPhase			= 1052,
	kHICommandUncoloredPattern		= 1053,
	kHICommandDrawWithPDFPattern	= 1054,

	kHICommandSimpleShadow			= 1055,
	kHICommandShadowScaling			= 1056,
	kHICommandShadowProblems		= 1057,
	kHICommandComplexShadow			= 1058,

	kHICommandMultipleShapeComposite	= 1059,
	kHICommandFillAndStrokeWithShadow = 1085,
	kHICommandPDFDocumentShadow		= 1060,
	
	kHICommandSimpleAxialShading	= 1061,
	kHICommandExampleAxialShadings	= 1062,
	kHICommandSimpleRadialShading	= 1063,
	kHICommandExampleRadialShadings	= 1064,
	kHICommandEllipseShading		= 1065,
	
	/*** EPS drawing ***/
	kHICommandDoCompatibleEPS		= 1066,

} DrawingCommand;

#endif // __UIHandling__