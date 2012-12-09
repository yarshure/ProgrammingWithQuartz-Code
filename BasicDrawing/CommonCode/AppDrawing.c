/*
*  File:    AppDrawing.c
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

#include "AppDrawing.h"
#include "UIHandling.h"
#include "FrameworkUtilities.h"
#include "DataProvidersAndConsumers.h"

/* Defines */
#define kCatPDF				CFSTR("Kitty.pdf")
#define kPDFForBlendMode	CFSTR("blendmode.pdf")

#define kOurJPEG			CFSTR("Poot.jpg")
#define kQTImage			CFSTR("ptlobos.tif")
#define kOurSubstituteJPG	CFSTR("LyingOnDeckNoProfile.JPG")
#define kOurEPS				CFSTR("imageturkey.eps")

#define RAW_IMAGE_WIDTH		400
#define RAW_IMAGE_HEIGHT	300
#define kRawColorImage		CFSTR("image-400x300x24.raw")
#define kOtherColorImage	CFSTR("otherimage-400x300x24.raw")

#define MASKING_IMAGE_WIDTH	400
#define MASKING_IMAGE_HEIGHT 259
#define kMaskingImage		CFSTR("400x259x8.bw.raw")

typedef void (doPDFDrawProc)(CGContextRef context, CFURLRef url);

static void callPDFDrawProc(CGContextRef context, doPDFDrawProc proc, CFStringRef pdfFile)
{
    CFURLRef ourPDFurl = NULL;
	CFBundleRef mainBundle = getAppBundle();
	
	if(mainBundle)
		ourPDFurl = CFBundleCopyResourceURL(mainBundle, pdfFile, NULL, NULL);
	else
		fprintf(stderr, "Can't get the app bundle!\n");

    if(ourPDFurl)
	    proc(context, ourPDFurl);
    else
	    fprintf(stderr, "Couldn't create the URL for our PDF document!\n");

}

static void doDrawJPEGFile(CGContextRef context)
{
    static CFURLRef ourJPEGurl = NULL;
    if(!ourJPEGurl){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            ourJPEGurl = CFBundleCopyResourceURL(mainBundle, kOurJPEG, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(ourJPEGurl)
	    drawJPEGImage(context, ourJPEGurl);
    else
	    fprintf(stderr, "Couldn't create the URL for our JPEG file!\n");

}

static void doRawImageFileWithURL(CGContextRef context)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kRawColorImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url)
		drawImageFromURL(context, url,
					RAW_IMAGE_WIDTH, RAW_IMAGE_HEIGHT, 
					8, true);   // 8 bits per component, isColor = true
    else
		fprintf(stderr, "Couldn't create the URL for our Raw Image file!\n");

}

static void doRawImageFileWithCallbacks(CGContextRef context)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kRawColorImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url)
		doImageWithCallbacksCreatedFromURL(context, url,
					RAW_IMAGE_WIDTH, RAW_IMAGE_HEIGHT, 
					8, true);   // 8 bits per component, isColor = true
    else
		fprintf(stderr, "Couldn't create the URL for our Raw Image file!\n");

}

static void doDrawImageWithCGImageSource(CGContextRef context)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kOurJPEG, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url)
	    drawImageWithCGImageDataSource(context, url);
    else
	    fprintf(stderr, "Couldn't create the URL for our CGImageSource input file!\n");

}


static void doIncrementalImage(CGContextRef context)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kOurJPEG, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url)
	    doIncrementalImageWithURL(context, url);
    else
	    fprintf(stderr, "Couldn't create the URL for our JPEG file!\n");

}

static void doQTImage(CGContextRef context)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kQTImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url)
	    drawQTImageWithQuartz(context, url);
    else
	    fprintf(stderr, "Couldn't create the URL for our QT file!\n");

}

static void doJPEGDocumentWithMultipleProfiles(CGContextRef context)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kOurSubstituteJPG, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url)
	    drawJPEGDocumentWithMultipleProfiles(context, url);
    else
	    fprintf(stderr, "Couldn't create the URL for our JPEG file!\n");

}

static void doMaskImageWithMask(CGContextRef context)
{
    static CFURLRef theImageToMaskURL = NULL, theMaskingImageURL = NULL;
    if(!theImageToMaskURL){
        CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            theImageToMaskURL = CFBundleCopyResourceURL(mainBundle, kOtherColorImage, NULL, NULL);
            theMaskingImageURL = CFBundleCopyResourceURL(mainBundle, kMaskingImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(theImageToMaskURL && theMaskingImageURL){
		size_t imagewidth = RAW_IMAGE_WIDTH, imageheight = RAW_IMAGE_HEIGHT, bitsPerComponent = 8;
		size_t maskwidth = MASKING_IMAGE_WIDTH, maskheight = MASKING_IMAGE_HEIGHT;
		doMaskImageWithMaskFromURL(context, theImageToMaskURL, imagewidth,
				    imageheight, bitsPerComponent, theMaskingImageURL, maskwidth,
				    maskheight);
    }else
		fprintf(stderr, "Couldn't create the URL for our masking image!\n");
}

static void doMaskImageWithGrayImage(CGContextRef context)
{
    static CFURLRef theImageToMaskURL = NULL, theMaskingImageURL = NULL;
    if(!theImageToMaskURL){
        CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            theImageToMaskURL = CFBundleCopyResourceURL(mainBundle, kOtherColorImage, NULL, NULL);
            theMaskingImageURL = CFBundleCopyResourceURL(mainBundle, kMaskingImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(theImageToMaskURL && theMaskingImageURL){
		size_t imagewidth = RAW_IMAGE_WIDTH, imageheight = RAW_IMAGE_HEIGHT, bitsPerComponent = 8;
		size_t maskwidth = MASKING_IMAGE_WIDTH, maskheight = MASKING_IMAGE_HEIGHT;
		doMaskImageWithGrayImageFromURL(context, theImageToMaskURL, imagewidth,
				    imageheight, bitsPerComponent, theMaskingImageURL, maskwidth,
				    maskheight);
    }else
		fprintf(stderr, "Couldn't create the URL for our masking image document!\n");
}

static void doImageMaskedWithColor(CGContextRef context)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kOtherColorImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url)
		doMaskImageWithColorFromURL(context, url,
 					RAW_IMAGE_WIDTH, RAW_IMAGE_HEIGHT,
					true);
    else
	    fprintf(stderr, "Couldn't create the URL for our JPEG file!\n");

}

static void exportImageMaskedWithImage(CGContextRef context)
{
    static CFURLRef theImageToMaskURL = NULL, theMaskingImageURL = NULL;
    if(!theImageToMaskURL){
        CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            theImageToMaskURL = CFBundleCopyResourceURL(mainBundle, kOtherColorImage, NULL, NULL);
            theMaskingImageURL = CFBundleCopyResourceURL(mainBundle, kMaskingImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(theImageToMaskURL && theMaskingImageURL){
		size_t imagewidth = RAW_IMAGE_WIDTH, imageheight = RAW_IMAGE_HEIGHT, bitsPerComponent = 8;
		size_t maskwidth = MASKING_IMAGE_WIDTH, maskheight = MASKING_IMAGE_HEIGHT;
		exportImageWithMaskFromURLWithDestination(context, theImageToMaskURL, imagewidth,
				    imageheight, bitsPerComponent, theMaskingImageURL, maskwidth,
				    maskheight);
    }else
		fprintf(stderr, "Couldn't create the URL for our masking image!\n");
}

static void doClipMask(CGContextRef context)
{
    static CFURLRef theMaskingImageURL = NULL;
    if(!theMaskingImageURL){
        CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            theMaskingImageURL = CFBundleCopyResourceURL(mainBundle, kMaskingImage, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(theMaskingImageURL){
		size_t maskwidth = MASKING_IMAGE_WIDTH, maskheight = MASKING_IMAGE_HEIGHT;
		drawWithClippingMask(context, theMaskingImageURL, maskwidth, maskheight);
    }else
		fprintf(stderr, "Couldn't create the URL for our clipping image!\n");
}

typedef enum OffScreenType{
    noOffScreen,
    bitmapOffScreen,
    layerOffScreen
}OffScreenType;

static void tilePDFDocument(CGContextRef context, OffScreenType offscreenType)
{
    static CFURLRef url = NULL;
    if(!url){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            url = CFBundleCopyResourceURL(mainBundle, kCatPDF, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(url){
		if(offscreenType == noOffScreen)
			TilePDFNoBuffer(context, url);
		else if(offscreenType == bitmapOffScreen){
			TilePDFWithOffscreenBitmap(context, url);
		}else
			TilePDFWithCGLayer(context, url);
    }else
		fprintf(stderr, "Couldn't create the URL for our PDF document!\n");

}

static void doCompatibleEPSDrawing(CGContextRef context)
{
    static CFURLRef ourEPSurl = NULL;
    if(!ourEPSurl){
		CFBundleRef mainBundle = getAppBundle();
        if(mainBundle){
            ourEPSurl = CFBundleCopyResourceURL(mainBundle, kOurEPS, NULL, NULL);
        }else
            fprintf(stderr, "Can't get the app bundle!\n");
    }

    if(ourEPSurl)
	    drawEPSDataImage(context, ourEPSurl);
    else
	    fprintf(stderr, "Couldn't create the URL for our EPS document!\n");

}

/* Dispatch Drawing */

void DispatchDrawing(CGContextRef context, OSType drawingType)
{
	switch (drawingType){
	    case kHICommandSimpleRect:
		    doSimpleRect(context);
		    break;

	    case kHICommandStrokedRect:
		    doStrokedRect(context);
		    break;
	    
	    case kHICommandStrokedAndFilledRect:
		    doStrokedAndFilledRect(context);
		    break;

	    case kHICommandPathRects:
		    doPathRects(context);
		    break;
	    
	    case kHICommandAlphaRects:
		    doAlphaRects(context);
		    break;
	    
	    case kHICommandDashed:
		    doDashedLines(context);
		    break;
	    
	    case kHICommandSimpleClip:
		    doClippedCircle(context);
		    break;
	    
	    case kHICommandPDFDoc:
		    callPDFDrawProc(context, doPDFDocument, kCatPDF);
		    break;
		
	    case kHICommandRotatedEllipses:
		    doRotatedEllipses(context);
		    break;
	
	    case kHICommandDrawSkewCoordinates:
		    drawSkewedCoordinateSystem(context);
		    break;

	    case kHICommandBezierEgg:
		    doEgg(context);
		    break;

	    case kHICommandRoundedRects:
		    doRoundedRects(context);
		    break;

	    case kHICommandStrokeWithCTM:
		    doStrokeWithCTM(context);
		    break;

	    case kHICommandRotatedEllipsesWithCGPath:
		    doRotatedEllipsesWithCGPath(context);
		    break;

	    case kHICommandPixelAligned:
		    doPixelAlignedFillAndStroke(context);
		    break;

	    case kHICommandDeviceFillAndStrokeColor:
		    doColorSpaceFillAndStroke(context);
		    break;

	    case kHICommandCLUTDrawGraphics:
		    doIndexedColorDrawGraphics(context);
		    break;
		    
	    case kHICommandDrawWithGlobalAlpha:
		    drawWithGlobalAlpha(context);
		    break;

	    case kHICommandDrawWithBlendMode:
		    callPDFDrawProc(context, drawWithColorBlendMode, kPDFForBlendMode);
		    break;

	    case kHICommandDrawWithColorRefs:
		    drawWithColorRefs(context);
		    break;

	    case kHICommandFunctionsHaveOwnGSave:
		    doClippedEllipse(context);
		    break;

	    case kHICommandDrawJPEGImage:
		    doDrawJPEGFile(context);
		    break;

	    case kHICommandColorImageFromFile:
		    doRawImageFileWithURL(context);
		    break;

	    case kHICommandColorImageFromData:
		    doColorRampImage(context);
		    break;

	    case kHICommandColorImageFromCallbacks:
		    doRawImageFileWithCallbacks(context);
		    break;

	    case kHICommandGrayRamp:
		    doGrayRamp(context);
		    break;

	    case kHICommandDrawWithCGImageSource:
		    doDrawImageWithCGImageSource(context);
		    break;

	    case kHICommandDrawWithCGImageSourceIncremental:
		    doIncrementalImage(context);
		    break;

	    case kHICommandDrawWithQuickTime:
		    doQTImage(context);
		    break;

	    case kHICommandSubstituteImageProfile:
		    doJPEGDocumentWithMultipleProfiles(context);
		    break;

	    case kHICommandDoSubImage:
		    doColorRampSubImage(context);
		    break;

	    case kHICommandExportWithQuickTime:
		    exportColorRampImageWithQT(context);
		    break;

	    case kHICommandMaskTurkeyImage:
		    doOneBitMaskImages(context);
		    break;

	    case kHICommandImageMaskedWithMask:
		    doMaskImageWithMask(context);
		    break;

	    case kHICommandImageMaskedWithGrayImage:
		    doMaskImageWithGrayImage(context);
		    break;

	    case kHICommandMaskImageWithColor:
		    doImageMaskedWithColor(context);
		    break;

	    case kHICommandClipToMask:
		    doClipMask(context);
		    break;

	    case kHICommandExportWithCGImageDestination:
		    exportImageMaskedWithImage(context);
		    break;

	    case kHICommandSimpleCGLayer:
		    doSimpleCGLayer(context);
		    break;

	    case kHICommandAlphaOnlyContext:
		    doAlphaOnlyContext(context);
		    break;

	    case kHICommandDrawNoOffScreenImage:
		    tilePDFDocument(context, noOffScreen);
		    break;

	    case kHICommandDrawOffScreenImage:
		    tilePDFDocument(context, bitmapOffScreen);
		    break;

	    case kHICommandDrawWithLayer:
		    tilePDFDocument(context, layerOffScreen);
		    break;

	    case kHICommandQuartzRomanText:
		    drawQuartzRomanText(context);
		    break;

	    case kHICommandQuartzTextModes:
		    drawQuartzTextWithTextModes(context);
		    break;

	    case kHICommandQuartzTextMatrix:
		    drawQuartzTextWithTextMatrix(context);
		    break;

#if CARBON_SAMPLE	// Defined in the Carbon project file.
	    case kHICommandPaste:
		    drawPasteBoardPDF(context);
		    break;

	    case kHICommandThemeText:
		    drawTextWithThemeText(context);
		    break;

	    case kHICommandATSUText:
		    drawTextWithATSUI(context);
		    break;
#endif

	    case kHICommandSimplePattern:
		    doRedBlackCheckerboard(context);
		    break;

	    case kHICommandPatternPhase:
		    doPatternPhase(context);
		    break;

	    case kHICommandPatternMatrix:
		    doPatternMatrix(context);
		    break;

	    case kHICommandUncoloredPattern:
		    doStencilPattern(context);
		    break;
		    
	    case kHICommandDrawWithPDFPattern:
		    callPDFDrawProc(context, drawWithPDFPattern, kCatPDF);
		    break;

	    case kHICommandSimpleShadow:
		    drawSimpleShadow(context);
		    break;
		
	    case kHICommandShadowScaling:
		    doShadowScaling(context);
		    break;
	    
	    case kHICommandShadowProblems:
		    showComplexShadowIssues(context);
		    break;

	    case kHICommandComplexShadow:
		    showComplexShadow(context);
		    break;
	    
	    case kHICommandMultipleShapeComposite:
		    doLayerCompositing(context);
		    break;

	    case kHICommandFillAndStrokeWithShadow:
		    drawFillAndStrokeWithShadow(context);
		    break;

	    case kHICommandPDFDocumentShadow:
		    callPDFDrawProc(context, shadowPDFDocument, kCatPDF);
		    break;

	    case kHICommandSimpleAxialShading:
		    doSimpleAxialShading(context);
		    break;
		    
	    case kHICommandExampleAxialShadings:
		    doExampleAxialShading(context);
		    break;

	    case kHICommandSimpleRadialShading:
		    doSimpleRadialShading(context);
		    break;
    
	    case kHICommandExampleRadialShadings:
		    doExampleRadialShadings(context);
		    break;

	    case kHICommandEllipseShading:
		    doEllipseShading(context);
		    break;

	    case kHICommandDoCompatibleEPS:
		    doCompatibleEPSDrawing(context);
		    break;

	    default:
		    break;
	}
}


