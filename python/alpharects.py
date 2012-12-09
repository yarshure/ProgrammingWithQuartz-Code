#!/usr/bin/python
#
#  File:    alpharects.py
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

# alpharects.py -- Simple python example to draw a series of alpha composited rectangles.

# Need to import all the attributes of the CoreGraphics module.
from CoreGraphics import *
import math   # for pi

#  Create a portrait US Letter size media rectangle.
mediaRect = CGRectMake(0, 0, 612, 792)

# Create a PDF context for the file "alpharects.pdf" using
# the mediaRect.
c = CGPDFContextCreateWithFilename("alpharects.pdf", mediaRect)

# Begin the first page of the PDF document being created.
c.beginPage(mediaRect)

# Compute the drawing to perform. This is the alpha rects
# drawing from Chapter 3.
numRects = 6
tintAdjust = 1./numRects
rotateAngle = 2*math.pi/numRects
ourRect = CGRectMake(0., 0., 130., 100.)

# Move the origin so that the drawing fits on the page.
c.translateCTM(2*ourRect.size.width, 2*ourRect.size.height)

# Start at tint value 1 and decrement each time 
# through the loop.
tint = 1.
# Loop from 0 to (numRects-1).
for i in range(0, numRects):
    # Set the fill color to the tint of red with
    # and alpha value the same as the tint.
    c.setRGBFillColor(tint, 0., 0., tint)
    c.fillRect(ourRect)
    c.rotateCTM(rotateAngle)
    tint -= tintAdjust

# End the first (and only) page of the PDF document.
c.endPage()
# Finalize the context, emitting any pending output.
# Any calls on the context object after calling
# finish produce undefined results.
c.finish()

