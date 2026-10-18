This is a simple parallel implementation of the classic reversi game.
# reversi

Interestingly I added parallelism to the top ply for obvious benefit.
Then I added code to parallelism lower ply's thinking that this might
be useful as the top ply iterates towards the end releasing unused
cores.  Oddly the result was a 8x slow down, which I attribute to the
inability to utilise the beta cut test during parallelism.  On large
trees the alpha/beta cut offs are huge and adding parallelism lower
down can block the cut offs (as it is dependant upon prior results).
Assuming my code wasn't buggy of course.

I'm still intrigued at the possibility of introducing parallelism to
lower ply's especially during the end game.  But for now the code only
runs the top ply in parallel using the number of cores available.