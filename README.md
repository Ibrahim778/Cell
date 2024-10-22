# Cell

This is my custom LiveArea proof of concept I [showcased](https://youtu.be/MjaeSctyods) back in 2021.  
Since then I have tinkered with shell a good deal and most of my what I found is available in main.cpp

The current commit is NOT the code I used for the proof of concept, unfortunately that version of source code was never committed to git and has since been lost to time. You can find parts of how it was done in the first commit however it was quite trivial, a simple scrollview with 500 buttons and an event handler.

For those wishing to continue this project I wish them luck, offests for some functions as well as other information I found is available in main.cpp.  

Probably the most efficient way to reverse engineer shell and complete this project would be to first reverse SceSQLiteVsh and then reverse SceLsdb. These 2 are used by shell to parse and store information in app.db, reversing them would likely give us a better understanding on how the db is used and maybe give us a starting point for all the unknown structs we currently have.
In addition I have found hooking things like PAF's register event, animation, create template etc functions and then printing the value in the lr register to be very useful in understanding what is going on under the hood and where certain functions are called.  
You can get a pretty good idea of the hierarchy of widgets using the PrintAllChildren function that will give you a yml-type output of widget types and hashes.
Realistically, this project could probably be completed with someone willing to devote enough time and effort to it. I have figured out how to halt shell from creating the original icons already but the remaining widgets have too many hardcoded routines in shell (applauncher_plugin & topmenu_plugin interfaces) to be deleted outright (causes crashes) so they need to be patched. Trouble is RE'ing them is hard and I frequently hit dead ends, your best shot if you want to keep your sanity is using the method I have suggested above.

The most interesting thing I have found during all this however is that shell seems to lack actual deletion / cleanup code for the livearea completelty. Calling Framework::UnloadPlugin or manually calling the shutdown / unload functions causes crashes when you'd expect it to unload and clean up the entire livearea.
