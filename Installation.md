# Prerequisits #
The code is compiled with `gcc` with the use of gnu `make`, so make sure you have these installed beforehand. However, the code probably does not heavily rely on gcc features nor on make, so you could use whatever you feel happy with. It may even not require anything which is too specific to linux.

You may want to have xfig in the first place.

# Download #
First, cd into a directory where you would like to have code reside in. Then, get the translator with
```
  svn checkout http://xfig2gerber.googlecode.com/svn/trunk/xfig2gerber/
```
The code will be contained in the `xfig2gerber` directory in your code directory.

Proceed in a similar way for the library:
```
  svn checkout http://xfig2gerber.googlecode.com/svn/trunk/xfiglibrary/
```
All library entries are contained in the xfiglibrary directory. You may want to make sure that tis is located or linked into whatever library path you use for xfig.

# Compile #
Issue a `make` command in the `xfig2gerber` directory. This should generate an executable with the name `xfig2gerber` you can use to translate xfig code.