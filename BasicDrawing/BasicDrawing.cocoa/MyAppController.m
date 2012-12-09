/*
*  File:    MyAppController.m
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

#import "MyAppController.h"
#include "Utilities.h"

@implementation MyAppController

// Initial defaults
float	_dpi = 144;
BOOL	_useQT = NO;

CFURLRef getURLToExport(char* suffix)
{
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    
     NSString *intialFileName = [NSString stringWithFormat:@"BasicDrawing.%s", suffix];

    if([savePanel runModalForDirectory: nil file: intialFileName] == NSFileHandlingPanelOKButton){
    
		return (CFURLRef)[savePanel URL];
    
    }
    return NULL;
}

- (IBAction)print:(id)sender
{
   [((MyView *)theView) print:sender];
}

-(void) updateDPIMenu:(id) sender
{
    if( currentDPIMenuItem != sender){
		// Uncheck the previous item.
		[currentDPIMenuItem setState:NSOffState];
		// Update to the current item.
		currentDPIMenuItem = sender;
		// Check new menu item.
		[currentDPIMenuItem setState:NSOnState];
    }
}

-(void) updateExportStyleMenu:(id) sender
{
    if(currentExportStyleMenuItem != sender){
		// Uncheck the previous item.
		[currentExportStyleMenuItem setState:NSOffState];
		// Update to the current item.
		currentExportStyleMenuItem = sender;
		// Check new menu item.
		[currentExportStyleMenuItem setState:NSOnState];
    }
}

- (IBAction)setExportResolution:(id)sender
{
    _dpi = [ sender tag ];
    [self updateDPIMenu:sender];
}

- (IBAction)setUseQT:(id)sender
{
    _useQT = YES;
    [self updateExportStyleMenu:sender];
}

- (IBAction)setUseCGImageSource:(id)sender
{
    _useQT = NO;
    [self updateExportStyleMenu:sender];
}

- (void)setupExportInfo:(ExportInfo *)exportInfoP
{
    // Use the printable version of the current command. This produces
    // the best results for exporting.
    exportInfoP->command = [((MyView *)theView) currentPrintableCommand];
    exportInfoP->fileType = '    ';	// unused
    exportInfoP->useQTForExport = _useQT;
    exportInfoP->dpi = _dpi;
}

- (IBAction)exportAsPDF:(id)sender
{
    CFURLRef url = getURLToExport("pdf");
    if(url){
		ExportInfo exportInfo;
		[self setupExportInfo:&exportInfo];
		MakePDFDocument(url, &exportInfo);
    }
}

- (IBAction)exportAsPNG:(id)sender
{
    CFURLRef url = getURLToExport("png");
    if(url){
		ExportInfo exportInfo;
		[self setupExportInfo:&exportInfo];
		MakePNGDocument(url, &exportInfo);
    }
}


- (IBAction)exportAsTIFF:(id)sender
{
    CFURLRef url = getURLToExport("tif");
    if(url){
		ExportInfo exportInfo;
		[self setupExportInfo:&exportInfo];
		MakeTIFFDocument(url, &exportInfo);
    }
}

- (IBAction)exportAsJPEG:(id)sender
{
    CFURLRef url = getURLToExport("jpg");
    if(url){
		ExportInfo exportInfo;
		[self setupExportInfo:&exportInfo];
		MakeJPEGDocument(url, &exportInfo);
    }
}

- (BOOL)validateMenuItem: (id <NSMenuItem>)menuItem
{
    if ([menuItem tag] == _dpi)
    {
		currentDPIMenuItem = menuItem;
		[menuItem setState: YES];
	} else if ([menuItem action] == @selector(setUseQT:)){
		if(_useQT == YES){
			currentDPIMenuItem = menuItem;
			[menuItem setState: YES];
		}else
			[menuItem setState: NO];
        
    } else if ([menuItem action] == @selector(setUseCGImageSource:)){
		if(_useQT == NO){
			currentDPIMenuItem = menuItem;
			[menuItem setState: YES];
		}else
			[menuItem setState: NO];
        
	}
	
	return YES;
}


@end
