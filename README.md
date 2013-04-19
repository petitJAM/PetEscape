Road Trip Disaster: PetEscape
=========
You and your explorer friends are out on a road trip with your new pets when disaster strikes!


Technical Stuff
=========

How to install qmake
---------

1) Go to http://www.qt-project.org/downloads

2) Click on the link for Qt 5.x for your platform

3) Install

Open QtCreator after installing, it may ask you to tell
it about the compiler to use. 

Windows: 	Use MSVC2010 (aka, v10.0)
Unix:		Any version of GCC should work.

How to install/build boost 1.53
---------

1) Go to http://sourceforge.net/projects/boost/files/boost/1.53.0/

2) Download one of them.

3) Extract somewhere.

3a) Windows: Open a MSVC2010 command prompt: 

	Start -> Microsoft Visual Studios 2010 -> Visual Studio Tools 
	Click on "Visual Studio Command Promt"
	move to the director that you extracted boost to.
	run the following:
		bootstrap
		.\b2 install --prefix=OUTPUT_DIRECTORY
	where OUTPUT_DIRECTORY is the path to the output directory (your choice)
	
3b) Linux: Open a shell

	Move to the extraction directory
	Run the following:
		./booststrap.sh --prefix=OUTPUT_DIRECTORY
		./b2 install 
	where OUTPUT_DIRECTORY is the path to the output directory (your choice)
	
Using Git
---------
http://stackoverflow.com/questions/1628563/move-recent-commit-to-a-new-branch

Setting up INCLUDEPATH and LIBS
---------

The libraries that PetEscape require (at the time of this writing) are as follows: boost_system, allegro, and allegro_image.

There may be additional information prefixing or suffixing the library name, that's okay, it's part of their naming convention.

In the file "PetEscape.mine.pro", you should have something such as the following:

    INCLUDEPATH += C:/devel/boost_1_53_0 \
                   C:/devel/allegro_5_0_8/include

    LIBS += -LC:/devel/boost_1_53_0/stage/lib \
            -LC:/devel/allegro_5_0_8/lib

NOTE: YOUR PATHS WILL BE DIFFERENT. That is why they are set in a file that is NOT version control here.

Now that you have told your project where to find the libraries and header files for boost and Allegro, you should be able to build.

TODO:
=========
There are currently no *nix libraries being loaded in the PetEscape.pro file. There is a section that looks like this:

    win32 {
        LIBS += libboost_system-vc100-mt-gd-1_53.lib \
                -lallegro-5.0.8-mt \
                -lallegro_image-5.0.8-mt
    }

    unix {
        LIBS += # Unix LIbs.
    }

In the unix block, we still need to add the libraries. THESE NAMES SHOULD BE IDENTICAL ACROSS ALL UNIX PLATFORMS.

I don't have a linux environment set up for Pet Escape at the moment, so this hasn't been a priority. If you are using a unix
variant, copy the unix{} block from above, and put it in PetEscape.mine.pro for now. Add the required libraries in that section
(use -l infront of the library name to specify that it is a library, see win32 section for example).

REMEMBER:
=========
If the project isn't building, make sure there are no instances of the execuatble running, then right click on the project name
in QtCreator, "clean", "qmake", "build". That will remove all associated binaries, object files, and it will recreate the Makefile
with the latest information from the .pro files.
