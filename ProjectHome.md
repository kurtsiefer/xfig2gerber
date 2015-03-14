You can generate multilayer circuit board layouts with xfig and convert them into the common CNC files ([RS274 or Gerber code](http://en.wikipedia.org/wiki/Gerber_File) and [Excelllon Drill files](http://en.wikipedia.org/wiki/Excellon_file)) your favorite PCB manufacturer understands. The code was developed under linux, but should be reasonably portable to other systems.

This project provides a translation program from xfig to Gerber code, a description how to use wich layers in xfig to generate proper copper layer/solder stop/silk screen files, and a library with some common elements like vias, connectors, chip pin patterns etc.

It started as a quick hack and it was just always easier to fix things rather than going through the learning curve of a proper CAD/CAM system for PCBs because I am a hopeless xfig addict. This code page is also for all those who suffer from existing circuit designs in xfig and don't know how to convert it.

Documentation and code will improve as bandwidth permits.

So far, there are wiki pages covering the following:
  * [Installation guide](http://code.google.com/p/xfig2gerber/wiki/Installation)
  * [Basic idea behind the package](http://code.google.com/p/xfig2gerber/wiki/basics)
  * [How to use the converter](http://code.google.com/p/xfig2gerber/wiki/Usage) with some examples
  * A two and four layer board example in http://code.google.com/p/xfig2gerber/wiki/Examples

Other useful links you want to follow if you work with this are:
  * An open source project on a viewer of gerber files http://gerbv.sourceforge.net
  * The official xfig link source, http://www.xfig.org
  * A Gerber code reference, http://www.artwork.com/gerber/274x/rs274xrevd_e.pdf

Things that are in the workings but need to be integrated into this package:
  * Generation of stencil masks that can differ slightly from the solder mask apertures
  * Consolidation and documentation of pick & place list generation scripts

Wish list, ideas etc
  * Some command or method that allows to place text on the board in a less insane way than now

If you are interested in this, have suggestions or complaints, please feel free to contact me (Christian).