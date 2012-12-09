/*
*  File:    NavServicesHandling.c
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

#include "NavServicesHandling.h"
#include "AppDrawing.h"
#include "UIHandling.h"

#define kOpenPrefKey	1


/****  Typedefs ********/

// the structure we're going to give to the open dialog to hang our data off of
typedef struct OurDialogData{	
    NavDialogRef	dialogRef;
}OurDialogData;

/**** Prototypes ****/

static pascal void NavEventProc( NavEventCallbackMessage callBackSelector, 
                                NavCBRecPtr callBackParms, void* callBackUD );

static OSStatus GetFSRefFromAEDesc( AEDesc* fileObject, FSRef* returnRef )
{
    OSStatus 	theErr = noErr;
    AEDesc		theDesc;
    
    if ((theErr = AECoerceDesc( fileObject, typeFSRef, &theDesc )) == noErr)
    {
	theErr = AEGetDescData( &theDesc, returnRef, sizeof ( FSRef ) );
	AEDisposeDesc( &theDesc );
    }
    return theErr;
}

static pascal void NavEventProc( NavEventCallbackMessage callBackSelector, 
                                NavCBRecPtr callBackParms, void* callBackUD )
{
    OurDialogData *dialogDataP = (OurDialogData*)callBackUD;
    OSStatus 	err = noErr;		        
	
    switch( callBackSelector )
	{
		case kNavCBUserAction:
		{
			NavReplyRecord 	reply;
			NavUserAction 	userAction = 0;
			
			if ((err = NavDialogGetReply( callBackParms->context, &reply )) == noErr )
			{
			OSStatus tempErr;
			userAction = NavDialogGetUserAction( callBackParms->context );
					
			switch( userAction )
			{
				case kNavUserActionOpen:
				{	
					long count = 0;
					short index;
					err = AECountItems( &reply.selection, &count );
					for ( index = 1; index <= count; index++ )
					{
						AEKeyword 	keyWord;
						AEDesc		theDesc;
						if (( err = AEGetNthDesc( &reply.selection, index, typeWildCard, &keyWord, &theDesc )) == noErr )
						{
							FSRef 	fileRef;
							GetFSRefFromAEDesc(&theDesc, &fileRef);
							(void)doTheFile( fileRef );
							AEDisposeDesc( &theDesc );
						}
					}
					EnableMenuCommand(NULL, kHICommandOpen);
				}
				break;
											
				case kNavUserActionCancel:
					//..
					break;
							
				case kNavUserActionNewFolder:
					//..
					break;
			}
				  
			tempErr = NavDisposeReply( &reply );
			if(!err)
				err = tempErr;
				}
				break;
		}
				
		case kNavCBTerminate:
			if( dialogDataP != NULL )
			{
				if(dialogDataP->dialogRef)
					NavDialogDispose(dialogDataP->dialogRef );
				
				dialogDataP->dialogRef = NULL;
				free(dialogDataP);
			}
			
		break;
	}
}

OSStatus DoOpenDocument(void)
{
    OSStatus 	err = noErr;
    NavDialogCreationOptions	dialogOptions;
    OurDialogData *dialogDataP = NULL;
    static NavEventUPP	gNavEventProc = NULL;		// event proc for our Nav Dialogs 
    if(!gNavEventProc){
        gNavEventProc = NewNavEventUPP(NavEventProc);
        if(!gNavEventProc)
            err = memFullErr;
    }

    // while our open dialog is up we'll disable our Open command, else
    // we might end up with more than one open dialog. Yuk
    DisableMenuCommand(NULL, kHICommandOpen);

    dialogDataP = (OurDialogData *)calloc(1, sizeof(OurDialogData));
    if(!dialogDataP)
	err = memFullErr;
	
    if (!err && ( err = NavGetDefaultDialogCreationOptions( &dialogOptions )) == noErr )
	{
		dialogOptions.preferenceKey = kOpenPrefKey;
		dialogOptions.modality = kWindowModalityAppModal;	// make it modal
		
		if ((err = NavCreateGetFileDialog( 	&dialogOptions,
							NULL,		   //openListH,
							gNavEventProc,
							NULL,		// no custom previews
							NULL,		// filter proc is NULL
							dialogDataP,
							&dialogDataP->dialogRef )) == noErr)
		{
			if (( err = NavDialogRun( dialogDataP->dialogRef )) != noErr){
				if ( dialogDataP->dialogRef != NULL ){
					NavDialogDispose( dialogDataP->dialogRef );
					dialogDataP->dialogRef = NULL;
					free(dialogDataP);
				}
			}
		}
		
		if(err == userCanceledErr){
			err = noErr;
		}
		EnableMenuCommand(NULL, kHICommandOpen);
	}

    return err;
}
