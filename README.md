Road Trip Disaster: PetEscape
=========
You and your explorer friends are out on a road trip with your new pets when disaster strikes!


Technical Stuff
=========
Makefile Generator: qmake

How to install: 
1) Go to http://www.qt-project.org/downloads
2) Click on the link for Qt 5.x for your platform
3) Install

Open QtCreator after installing, it may ask you to tell
it about the compiler to use. 

Windows: 	Use MSVC2010 (aka, v10.0)
Unix:		Any version of GCC should work.

Networking Library: boost (v1.53.0)

How to install/build:
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

