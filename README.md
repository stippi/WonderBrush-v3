# WonderBrush-v3
WonderBrush is a graphics application for Haiku.

## How to build

This code currently builds on Haiku for the "x86" architecture. If this is not already your primary architecture, you need to

  *setarch x86*

... in a Haiku Terminal. Then you can build with

  *jam -q -j2*

"-q" is for stopping the build on the first error, "-j" is to build using two concurrent jobs. If you have more CPU cores
available, adjust the parameter as needed.

If you get any missing header or missing libraries errors, you need to solve them by installing the respective development packages in HaikuDepot. I am sure at least libfreetype2 is needed. No fiddling or any cludges should be necessary aport from simply installing development packages (those that come with headers).

The build artifacts are placed in a folder "generated". By default, you do a release build, so the WonderBrush executable 
is placed in sub-folder "distro-haiku". You can launch WonderBrush by executing

  *generated/distro-haiku/WonderBrush --fonts data/fonts*

The "--fonts" parameters is to tell WonderBrush were to look for some additional fonts. At the moment, WonderBrush directly 
creates a "demo" document which use some of these fonts. The document will look wrong when you forget this parameter.

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
 
 * Use of a layer-tree instead of a flat layer list. So layers can now be nested.
 
 * 16-bit linear RGB internal color space. sRGB is designed to preserve details in dark and bright colors at the same 
 time. This is achieved by using a non-uniform distribution of brightness. Non-uniform color values mean you cannot 
 really do any calculations, or if you do, you will get weird artifacts.
 
 * Global resources. Objects in WonderBrush 3 can share esources, such as colors or vector paths. Manipulating
 the shared resources will affect all objects which use them at the same time.

Some of the more recent work has been to provide the capability to load WonderBrush v2 documents. This is incomplete in that WonderBrush v3 does not include all the features of v2, and it will also reveal some current shortcommings of the new code. The most important one is that no caching exists and some operations are not yet optimized enough. The effect is that loading some documents will appear to lock up the new WonderBrush, and once it finally renders something, actually manipulating the document will prove hopeless.

## What's included

This code also contains an incomplete (and currently broken) port to Qt. The Qt port could be fixed by providing platform delegate code to the new tool UI which has been written since the Qt port was last compiled.

Also included is some code which is currently not used and not compiled. For example the ALM layout code. The plan was to allow optional layouts to be used in documents, which will place objects within a layout (instead of at absolute global positions).

There might be more code included which is currently not used.
