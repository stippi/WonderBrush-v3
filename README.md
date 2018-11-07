# WonderBrush-v3
WonderBrush is a graphics application for Haiku.

## How to build

This code currently builds on Haiku for the "x86" architecture. If this is not already your primary architecture, you need to

  *setarch x86*

... in a Haiku Terminal. Then you can build with

  *jam -q -j2*

"-q" is for stopping the build on the first error, "-j" is to build using two concurrent jobs. If you have more CPU cores
available, adjust the parameter as needed.

If you get any missing header or missing libraries errors, you need to solve them by installing the respective development packages in HaikuDepot. I am sure at least libfreetype2 is needed. No fiddling or any cludges should be necessary apart from simply installing development packages (those that come with headers).

The build artifacts are placed in a folder "generated". By default, you do a release build, so the WonderBrush executable 
is placed in a sub-folder "distro-haiku". You can launch WonderBrush by executing

  *generated/distro-haiku/WonderBrush --fonts data/fonts*

The "--fonts" parameters is to tell WonderBrush were to look for some additional fonts. At the moment, WonderBrush directly 
creates a "demo" document which uses some of these fonts. The document will look wrong when you forget this parameter.

You can also do a debug-build. For this, simply define a "DEBUG" environment variable while doing the build. Such as this:

  *DEBUG=1 jam -q -j2*

This will automatically place the object files into a different sub-folder in "generated", so you launch WonderBrush like this:

  *generated/distro-debug-haiku/WonderBrush --fonts data/fonts*

or like this:

  *Debugger generated/distro-debug-haiku/WonderBrush --fonts data/fonts*

... to launch it directly in the Haiku source level debugger.

## What this is

This is completely new code. There are some painful shortcommings in the "old" WonderBrush, not only in its overall 
capabilities and its tools, but also in the way the code was written. The new code is supposed to 
address these problems by being properly designed. In terms of features, here are the most important highlights:

 * Fully multi-threaded and asynchronous rendering pipeline. The main objective was to keep the UI responsive at all 
 times, also and especially while manipulating objects on the canvas.
 
 * Use of a layer-tree instead of a flat layer list. So layers can now be nested and for example inherit their transformation.
 
 * 16-bit linear and pre-multiplied RGB internal color space. 8 bit sRGB is designed to preserve details in dark and 
 bright colors at the same time so humans can perceive about the same change in brightness going from one value to the next.
 This is achieved by using a non-uniform distribution of brightness. Non-uniform color values mean you cannot really do any
 calculations, or if you do, you will get weird artifacts. This is why a liniar RGB color space is used internally, which has
 a linear change in brightness. To preserve detail, 16 bits are used instead of 8. The colors are also pre-multiplied with the
 alpha channel. When alpha is zero at a given pixel, the color channels are meaningless when combining with other pixels, and that is what pre-multiplying achieves. This is important in all sorts of calculations, especially filters.
 
 * Global resources. Objects in WonderBrush 3 can share resources, such as colors or vector paths. Manipulating
 the shared resources will affect all objects which use them at the same time.

Some of the more recent work has been to provide the capability to load WonderBrush v2 documents. This is incomplete in that WonderBrush v3 does not include all the features of v2, and it will also reveal some current shortcommings of the new code. The most important one is that almost no caching exists and some operations are not yet optimized enough. The effect is that loading some documents will appear to lock up the new WonderBrush, and once it finally renders something, actually manipulating the document will prove hopeless.

Here is an old blog post about WonderBrush v3: https://www.haiku-os.org/blog/stippi/2012-10-25_new_wonderbrush/

And here is the original website of WonderBrush v2: http://yellowbites.com/wonderbrush.html

(Please don't try to purchase WonderBrush, it is no longer available on Kagi or Mensys. Instead, it comes for free with every Haiku installation.)

## What's included

This code also contains an incomplete (and currently broken) port to Qt. The Qt port could be fixed by providing platform delegate code to the new tool UI which has been written since the Qt port was last maintained.

Also included is some code which is currently not used and not compiled. For example the ALM layout code. The plan was to allow optional layouts to be used in documents, which would place objects within a layout (instead of at absolute positions on a given layer).

There might be more code included which is currently not used.
