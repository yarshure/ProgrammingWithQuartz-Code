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

enum {
    kCantCreateSaveURL = 2000
};

// The command IDs for the different app specific menu commands.
enum {
    /***    Drawing Basics ***/
	kHICommandSimpleRect			= 'sirt',
	kHICommandStrokedRect			= 'strt',
	kHICommandStrokedAndFilledRect	= 'sfrt',
	kHICommandPathRects				= 'scrt',
	kHICommandAlphaRects			= 'alrt',
	kHICommandDashed				= 'dart',
	kHICommandSimpleClip			= 'sicl',
	kHICommandPDFDoc				= 'pddo',

    /***    Coordinate System ***/
	kHICommandRotatedEllipses		= 'sccl',
	kHICommandDrawSkewCoordinates	= 'skew',

    /***    Path Drawing ***/
	kHICommandBezierEgg				= 'beze',
	kHICommandRoundedRects			= 'rrct',
	kHICommandStrokeWithCTM			= 'sctm',
	kHICommandRotatedEllipsesWithCGPath	= 'sccp',
	kHICommandPixelAligned			= 'paln',

    /***    Color and GState ***/
	kHICommandDeviceFillAndStrokeColor	= 'deco',
	kHICommandCLUTDrawGraphics		= 'clut',
	kHICommandDrawWithGlobalAlpha	= 'galf',
	kHICommandDrawWithBlendMode		= 'blen',
	kHICommandDrawWithColorRefs		= 'cref',
	kHICommandFunctionsHaveOwnGSave	= 'rous',

    /***    Images ***/
	kHICommandDrawJPEGImage			= 'simi',
	kHICommandColorImageFromFile	= 'cimf',
	kHICommandColorImageFromData	= 'cidt',
	kHICommandColorImageFromCallbacks	= 'cicb',
	kHICommandGrayRamp				= 'grmp',
	kHICommandDrawWithCGImageSource	= 'imsr',
	kHICommandDrawWithCGImageSourceIncremental = 'imic',
	kHICommandDrawWithQuickTime		= 'qtim',
	kHICommandSubstituteImageProfile	= 'spro',
	kHICommandDoSubImage			= 'subi',
	kHICommandExportWithQuickTime	= 'etim',

    /***    Image Masking ***/
	kHICommandMaskTurkeyImage		= 'mtky',
	kHICommandImageMaskedWithMask	= 'mimm',
	kHICommandImageMaskedWithGrayImage	= 'mgmm',
	kHICommandMaskImageWithColor	= 'mcim',
	kHICommandClipToMask			= 'cmsk',
	kHICommandExportWithCGImageDestination	= 'imdt',

    /*** Bitmap Graphics Context and CGLayerRef ***/
	kHICommandSimpleCGLayer			= 'cgly',
	kHICommandAlphaOnlyContext		= 'aoly',
	kHICommandDrawNoOffScreenImage	= 'noff',
	kHICommandDrawOffScreenImage	= 'ioff',
	kHICommandDrawWithLayer			= 'loff',

    /***    PDF Export ***/
	kHICommandExportPDF				= 'epdf',


    /***    TIFF Export ***/
	kHICommandExportTIFF			= 'etif',
	kHICommandExportPNG				= 'epng',
	kHICommandExportJPEG			= 'ejpg',

    /***    Image Export Menu ***/
	kHICommandExportImagesWithCG	= 'exio',
	kHICommandExportImagesWithQT	= 'exqt',

	kHICommandExportImagesWith72dpi	= '072d',
	kHICommandExportImagesWith144dpi = '144d',
	kHICommandExportImagesWith300dpi = '300d',

    /***    Text ***/
	kHICommandQuartzRomanText		= 'btxt',
	kHICommandQuartzTextModes		= 'rtxt',
	kHICommandQuartzTextMatrix		= 'qtxt',
	
	kHICommandThemeText				= 'ttex',
	kHICommandATSUText				= 'atex',

    /***    Advanced Drawing ***/
	kHICommandSimplePattern			= 'spat',
	kHICommandPatternPhase			= 'ppha',
	kHICommandPatternMatrix			= 'pmtx',
	kHICommandUncoloredPattern		= 'upat',
	kHICommandDrawWithPDFPattern	= 'pdfp',

	kHICommandSimpleShadow			= 'shad',
	kHICommandShadowScaling			= 'shds',
	kHICommandShadowProblems		= 'cshd',
	kHICommandComplexShadow			= 'cmsd',

	kHICommandMultipleShapeComposite	= 'cshp',
	kHICommandFillAndStrokeWithShadow	= 'fshd',
	kHICommandPDFDocumentShadow		= 'cpdf',
	
	kHICommandSimpleAxialShading	= 'sshd',
	kHICommandExampleAxialShadings	= 'eshd',
	kHICommandSimpleRadialShading	= 'srad',
	kHICommandExampleRadialShadings	= 'rshd',
	kHICommandEllipseShading		= 'elsh',
	
	/* EPS */
	kHICommandDoCompatibleEPS		= 'deps',
};

// These key the localizable strings.
#define kMyDeleteErrorFormatStrKey 	CFSTR("NoDelete error format")
#define	kAboutBoxStringKey			CFSTR("About Box Info")
#define	kAboutBoxTitleKey			CFSTR("About Box")
#define kMyWriteErrorFormatStrKey 	CFSTR("Write error format")

void DoErrorAlert(OSStatus status, CFStringRef errorFormatString);

#endif	// __UIHandling__