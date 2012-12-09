#!/usr/bin/python
#
#  File:    Stamp PDF with "Confidential"
#  
#  Copyright:  Copyright (c) 2005 Apple Computer, Inc., All Rights Reserved
# 
#  Disclaimer:  IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
#        consideration of your agreement to the following terms, and your use, installation, modification 
#        or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
#        not agree with these terms, please do not use, install, modify or redistribute this Apple 
#        software.
#
#        In consideration of your agreement to abide by the following terms, and subject to these terms, 
#        Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
#        original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
#        Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
#        redistribute the Apple Software in its entirety and without modifications, you must retain this 
#        notice and the following text and disclaimers in all such redistributions of the Apple Software. 
#        Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
#        endorse or promote products derived from the Apple Software without specific prior written 
#        permission from Apple.  Except as expressly stated in this notice, no other rights or 
#        licenses, express or implied, are granted by Apple herein, including but not limited to any 
#        patent rights that may be infringed by your derivative works or by other works in which the 
#        Apple Software may be incorporated.
#
#        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
#        IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
#        AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
#        OR IN COMBINATION WITH YOUR PRODUCTS.
#
#        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
#        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
#        OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
#        REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
#        UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
#        IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from CoreGraphics import *
import sys, os
from math import atan

# The arguments to a printing workflow script 
# are "title" "options" "path-to-pdf-file"

if len (sys.argv) != 4:
  print >>sys.stderr, "usage: %s title options PDF-FILE" % sys.argv[0]
  sys.exit (1)

in_file = sys.argv[3]

if not os.path.isfile(in_file) :
    print "Input file \"%s\" does not exist!" % in_file
    sys.exit (2)

# Create a temporary file name for the output file.
out_prefix = sys.argv[1] + "."
out_file = os.tempnam(None, out_prefix)

pdf = CGPDFDocumentCreateWithProvider(
        CGDataProviderCreateWithFilename (in_file))

numPages = pdf.getNumberOfPages()

mediaRect = pdf.getMediaBox(1)
mediaRect.origin.x = mediaRect.origin.y = 0

# Create a PDF context for the output file using
# the mediaRect.
pdfContext = CGPDFContextCreateWithFilename(out_file, mediaRect)

stampString = "Confidential"
font = "Gadget"
fontSize = 60

# This creates an HTML string that draws the word 'stampString'
# in the typeface 'font', at the size 'fontSize'.
htmlString = '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" \
		"http://www.w3.org/TR/html4/strict.dtd"> \
	<html> <head> <meta http-equiv=\"Content-Type\" \
	content="text/html; charset=UTF-8\"> \
  <meta http-equiv=\"Content-Style-Type\" content="text/css\"> \
  <style type=\"text/css\"> \
	p.p1 {margin: 0.0px 0.0px 0.0px 0.0px; font: %dpx %s} \
  </style> </head> <body> <p class=\"p1\">%s</p> </body> </html>' \
  		% (fontSize, font, stampString)

stringProvider = CGDataProviderCreateWithString (htmlString)

textBoxWidth = -1
textBoxHeight = -1

for i in range(1, numPages + 1):

    # Get the media box of the content for this page.
    mediaRect = pdf.getMediaBox(i)
    mediaRect.origin.x = mediaRect.origin.y = 0
    
    pdfContext.beginPage(mediaRect)
    
    # Calculate the box width and height the first time
    # through the loop.
    if(i is 1) :
	# Draw the text to determine the width. Set the drawing mode
	# to invisible so that it doesn't appear.
	pdfContext.setTextDrawingMode(kCGTextInvisible)
	outRect = pdfContext.drawHTMLTextInRect(stringProvider, 
					    mediaRect, fontSize)
	# Get the text position.
	newTextPosition = pdfContext.getTextPosition()
	# The width of the text is the ending x pen position minus
	# the starting pen position (which is 0 in this case).
	textBoxWidth = newTextPosition.x 
	# The text height is the height of the rect returned from
	#  drawHTMLTextInRect. Because of a bug in drawHTMLTextInRect, 
	# the width of this rect is not the correct width.
	textBoxHeight = outRect.size.height
	# Reset the text mode to fill for painting the text.
	pdfContext.setTextDrawingMode(kCGTextFill)
	
    # Draw the page of the source PDF document to the PDF context.
    pdfContext.drawPDFDocument(mediaRect, pdf, i)
        
    pdfContext.saveGState()

    # Locate the text. First translate to the center of the page.
    pdfContext.translateCTM(mediaRect.size.width/2 , 
				mediaRect.size.height/2)
    
    # Compute the angle along the diagonal and rotate by that angle.
    theta = atan ( mediaRect.size.height / mediaRect.size.width)
    pdfContext.rotateCTM(theta)
    
    # Position so the text will be centered about the center of the page.
    pdfContext.translateCTM(-textBoxWidth/2, -textBoxHeight/2)
    
    # Set the global alpha to 0.3 so the text partially transparent.
    pdfContext.setAlpha(0.3)
    
    # Draw the text on top of the PDF page.
    pdfContext.drawHTMLTextInRect(stringProvider, 
		    CGRectMake(0,0,textBoxWidth,textBoxHeight), fontSize)
    
    pdfContext.restoreGState()

    pdfContext.endPage()

pdfContext.finish()

# Delete the input file since it is no longer needed.
os.unlink(in_file)

# Open the resulting file in Preview.
os.system("/usr/bin/open -a Preview \"%s\"" % out_file)
